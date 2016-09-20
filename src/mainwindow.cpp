#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "application.h"
#include "dialog_about.h"
#include "dialog_export_options.h"
#include "dialog_options.h"
#include "dialog_save_image_view.h"
#include "dialog_task_manager.h"
#include "document.h"
#include "gui_application.h"
#include "gui_document.h"
#include "widget_gui_document_view3d.h"
#include "widget_message_indicator.h"
#include "fougtools/qttools/gui/qwidget_utils.h"
#include "fougtools/qttools/task/manager.h"
#include "fougtools/qttools/task/runner_qthreadpool.h"

#include <QtCore/QTime>
#include <QtCore/QSettings>
#include <QtGui/QDesktopServices>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFileDialog>

namespace Mayo {

namespace Internal {

static const QLatin1String keyLastOpenDir("GUI/MainWindow_lastOpenDir");
static const QLatin1String keyLastSelectedFilter("GUI/MainWindow_lastSelectedFilter");

static Application::PartFormat partFormatFromFilter(const QString& filter)
{
    const auto itFormatFound =
            std::find_if(Application::partFormats().cbegin(),
                         Application::partFormats().cend(),
                         [=](Application::PartFormat format) {
        return filter == Application::partFormatFilter(format);
    } );
    return itFormatFound != Application::partFormats().cend() ?
                *itFormatFound :
                Application::PartFormat::Unknown;
}

struct ImportExportSettings
{
    QString openDir;
    QString selectedFilter;

    static ImportExportSettings load()
    {
        QSettings settings;
        return { settings.value(keyLastOpenDir, QString()).toString(),
                 settings.value(keyLastSelectedFilter, QString()).toString() };
    }

    static void save(const ImportExportSettings& sets)
    {
        QSettings settings;
        settings.setValue(keyLastOpenDir, sets.openDir);
        settings.setValue(keyLastSelectedFilter, sets.selectedFilter);
    }
};

} // namespace Internal


MainWindow::MainWindow(GuiApplication *guiApp, QWidget *parent)
    : QMainWindow(parent),
      m_guiApp(guiApp),
      m_ui(new Ui_MainWindow)
{
    m_ui->setupUi(this);
    m_ui->widget_DocumentProps->setGuiApplication(guiApp);
    m_ui->widget_DocumentProps->editDocumentItems(std::vector<DocumentItem*>());

    new DialogTaskManager(this);

    QObject::connect(
                m_ui->actionNewDoc, &QAction::triggered,
                this, &MainWindow::newDoc);
    QObject::connect(
                m_ui->actionOpen, &QAction::triggered,
                this, &MainWindow::openPartInNewDoc);
    QObject::connect(
                m_ui->actionImportPart, &QAction::triggered,
                this, &MainWindow::importPartInCurrentDoc);
    QObject::connect(
                m_ui->actionExportSelectedItems, &QAction::triggered,
                this, &MainWindow::exportSelectedItems);
    QObject::connect(
                m_ui->actionQuit, &QAction::triggered,
                this, &MainWindow::quitApp);
    QObject::connect(
                m_ui->actionSaveImageView, &QAction::triggered,
                this, &MainWindow::saveImageView);
    QObject::connect(
                m_ui->actionOptions, &QAction::triggered,
                this, &MainWindow::editOptions);
    QObject::connect(
                m_ui->actionReportBug, &QAction::triggered,
                this, &MainWindow::reportbug);
    QObject::connect(
                m_ui->actionAboutMayo, &QAction::triggered,
                this, &MainWindow::aboutMayo);
    QObject::connect(
                m_ui->tab_GuiDocuments, &QTabWidget::tabCloseRequested,
                this, &MainWindow::onTabCloseRequested);
    QObject::connect(
                this, &MainWindow::operationFinished,
                this, &MainWindow::onOperationFinished);

    QObject::connect(
                guiApp, &GuiApplication::guiDocumentAdded,
                this, &MainWindow::onGuiDocumentAdded);
    QObject::connect(
                m_ui->widget_ApplicationTree,
                &WidgetApplicationTree::selectionChanged,
                this,
                &MainWindow::onApplicationTreeWidgetSelectionChanged);

    this->updateControlsActivation();
}

MainWindow::~MainWindow()
{
    delete m_ui;
}

void MainWindow::newDoc()
{
    Application::instance()->addDocument();
}

void MainWindow::openPartInNewDoc()
{
    qtgui::QWidgetUtils::asyncMsgBoxCritical(
                this, tr("Error"), tr("Not yet implemented"));
}

void MainWindow::importPartInCurrentDoc()
{
    auto docView3d =
            qobject_cast<WidgetGuiDocumentView3d*>(
                m_ui->tab_GuiDocuments->currentWidget());
    if (docView3d != nullptr) {
        auto lastSettings = Internal::ImportExportSettings::load();
        const QString filepath =
                QFileDialog::getOpenFileName(
                    this,
                    tr("Select Part File"),
                    lastSettings.openDir,
                    Application::partFormatFilters().join(QLatin1String(";;")),
                    &lastSettings.selectedFilter);
        if (!filepath.isEmpty()) {
            lastSettings.openDir = QFileInfo(filepath).canonicalPath();
            Document* doc = docView3d->guiDocument()->document();
            const Application::PartFormat format =
                    Internal::partFormatFromFilter(lastSettings.selectedFilter);
            qttask::BaseRunner* task =
                    qttask::Manager::globalInstance()->newTask<QThreadPool>();
            task->run([=]{
                QTime chrono;
                chrono.start();
                const bool ok =
                        Application::instance()->importInDocument(
                            doc, format, filepath, &task->progress());
                const QString msg =
                        ok ?
                            tr("Import time: %1ms").arg(chrono.elapsed()) :
                            tr("Failed to import part:\n    '%1'").arg(filepath);
                emit operationFinished(ok, msg);
            });
            Internal::ImportExportSettings::save(lastSettings);
        }
    }
}

void MainWindow::exportSelectedItems()
{
    auto lastSettings = Internal::ImportExportSettings::load();
    const QString filepath =
            QFileDialog::getSaveFileName(
                this,
                tr("Select Output File"),
                lastSettings.openDir,
                Application::partFormatFilters().join(QLatin1String(";;")),
                &lastSettings.selectedFilter);
    if (!filepath.isEmpty()) {
        lastSettings.openDir = QFileInfo(filepath).canonicalPath();
        const Application::PartFormat format =
                Internal::partFormatFromFilter(lastSettings.selectedFilter);
        if (Application::hasExportOptionsForFormat(format)) {
            auto dlg = new DialogExportOptions(this);
            dlg->setPartFormat(format);
            QObject::connect(
                        dlg, &QDialog::accepted,
                        [=]{
                this->doExportSelectedItems(
                            format, dlg->currentExportOptions(), filepath);
                Internal::ImportExportSettings::save(lastSettings);
            });
            qtgui::QWidgetUtils::asyncDialogExec(dlg);
        }
        else {
            this->doExportSelectedItems(
                        format, Application::ExportOptions(), filepath);
            Internal::ImportExportSettings::save(lastSettings);
        }
    }
}

void MainWindow::quitApp()
{
    QApplication::quit();
}

void MainWindow::editOptions()
{
    auto dlg = new DialogOptions(this);
    qtgui::QWidgetUtils::asyncDialogExec(dlg);
}

void MainWindow::saveImageView()
{
    auto docView3d =
            qobject_cast<const WidgetGuiDocumentView3d*>(
                m_ui->tab_GuiDocuments->currentWidget());
    auto dlg = new DialogSaveImageView(docView3d->widgetOccView());
    qtgui::QWidgetUtils::asyncDialogExec(dlg);
}

void MainWindow::aboutMayo()
{
    auto dlg = new DialogAbout(this);
    qtgui::QWidgetUtils::asyncDialogExec(dlg);
}

void MainWindow::reportbug()
{
    QDesktopServices::openUrl(
                QUrl(QStringLiteral("https://github.com/fougue/mayo/issues")));
}

void MainWindow::onGuiDocumentAdded(GuiDocument *guiDoc)
{
    m_ui->tab_GuiDocuments->addTab(
                guiDoc->widgetView3d(), guiDoc->document()->label());
    m_ui->tab_GuiDocuments->setCurrentWidget(guiDoc->widgetView3d());
    this->updateControlsActivation();
}

void MainWindow::onApplicationTreeWidgetSelectionChanged()
{
    m_ui->widget_DocumentProps->editDocumentItems(
                m_ui->widget_ApplicationTree->selectedDocumentItems());
}

void MainWindow::onOperationFinished(bool ok, const QString &msg)
{
    if (ok)
        WidgetMessageIndicator::showMessage(msg, this);
    else
        qtgui::QWidgetUtils::asyncMsgBoxCritical(this, tr("Error"), msg);
}

void MainWindow::onTabCloseRequested(int tabIndex)
{
    QWidget* tab = m_ui->tab_GuiDocuments->widget(tabIndex);
    auto docView3d = qobject_cast<WidgetGuiDocumentView3d*>(tab);
    Application::instance()->eraseDocument(docView3d->guiDocument()->document());
    m_ui->tab_GuiDocuments->removeTab(tabIndex);
    this->updateControlsActivation();
}

void MainWindow::doExportSelectedItems(
        Application::PartFormat format,
        const Application::ExportOptions &opts,
        const QString& filepath)
{
    const std::vector<DocumentItem*> vecDocItem =
            m_ui->widget_ApplicationTree->selectedDocumentItems();
    qttask::BaseRunner* task =
            qttask::Manager::globalInstance()->newTask<QThreadPool>();
    task->run([=]{
        QTime chrono;
        chrono.start();
        const bool ok =
                Application::instance()->exportDocumentItems(
                    vecDocItem, format, opts, filepath, &task->progress());
        const QString msg =
                ok ?
                    tr("Export time: %1ms").arg(chrono.elapsed()) :
                    tr("Failed to export part:\n    '%1'").arg(filepath);
        emit operationFinished(ok, msg);
    });
}

void MainWindow::updateControlsActivation()
{
    const bool appDocumentsEmpty = Application::instance()->documents().empty();
    m_ui->actionImportPart->setEnabled(!appDocumentsEmpty);
    m_ui->actionSaveImageView->setEnabled(!appDocumentsEmpty);
}

} // namespace Mayo