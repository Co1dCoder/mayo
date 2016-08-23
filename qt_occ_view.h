#pragma once

#include <functional>
#include <QtWidgets/QWidget>
#include <V3d_View.hxx>
#include <V3d_Viewer.hxx>

namespace Mayo {

class QtOccView : public QWidget
{
    Q_OBJECT

public:
    QtOccView(QWidget* parent = nullptr);
    QtOccView(const Handle_V3d_Viewer& viewer, QWidget* parent = nullptr);

    const Handle_V3d_Viewer& v3dViewer() const;
    void setV3dViewer(const Handle_V3d_Viewer& viewer);

    const Handle_V3d_View& v3dView() const;

    QPaintEngine* paintEngine() const override;

    void redraw();
    void fitAll();

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    void initialize();

    Handle_V3d_Viewer m_viewer;
    Handle_V3d_View m_view;
    bool m_isInitialized = false;
    bool m_needsResize = false;
};

} // namespace Mayo