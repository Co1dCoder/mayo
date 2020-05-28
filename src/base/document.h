/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "document_ptr.h"
#include "libtree.h"
#include "xcaf.h"
#include <QtCore/QObject>
#include <functional>

namespace Mayo {

class Application;
class DocumentTreeNode;

class Document : public QObject, public TDocStd_Document {
    Q_OBJECT
    Q_PROPERTY(int identifier READ identifier)
    Q_PROPERTY(QString name READ name WRITE setName)
    Q_PROPERTY(bool isXCafDocument READ isXCafDocument)
public:
    using Identifier = int;
    enum class Format { Binary, Xml };

    Identifier identifier() const { return m_identifier; }

    QString name() const;
    void setName(const QString& v);

    QString filePath() const;
    void setFilePath(const QString& filepath);

    static const char NameFormatBinary[];
    static const char NameFormatXml[];
    static const char* toNameFormat(Format format);

    static const char TypeName[];
    virtual const char* dynTypeName() const { return Document::TypeName; }

    bool isXCafDocument() const;
    XCaf& xcaf() { return m_xcaf; }
    const XCaf& xcaf() const { return m_xcaf; }

    TDF_Label rootLabel() const;
    bool isEntity(TreeNodeId nodeId);

    const Tree<TDF_Label>& modelTree() const { return m_modelTree; }
    void rebuildModelTree();

    static DocumentPtr findFrom(const TDF_Label& label);

    void xcafImport(const std::function<bool()>& fnImport);
    void singleImport(const std::function<bool(TDF_Label)>& fnImport);

    void destroyEntity(TreeNodeId entityTreeNodeId);

public: // -- from TDocStd_Document
    void BeforeClose() override;
    void ChangeStorageFormat(const TCollection_ExtendedString& newStorageFormat) override;

    DEFINE_STANDARD_RTTI_INLINE(Document, TDocStd_Document)

signals:
    //void entityAdded(const TDF_Label& labelEntity);
    void entityAdded(TreeNodeId entityTreeNodeId);
    void entityAboutToBeDestroyed(TreeNodeId entityTreeNodeId);
    //void itemErased(const DocumentItem* docItem);
    //void itemPropertyChanged(DocumentItem* docItem, Property* prop);

private:
    friend class Application;
    class FormatBinaryRetrievalDriver;
    class FormatXmlRetrievalDriver;

    Document();
    void initXCaf();
    void setIdentifier(Identifier ident) { m_identifier = ident; }

    Identifier m_identifier = -1;
    QString m_name;
    QString m_filePath;
    XCaf m_xcaf;
    Tree<TDF_Label> m_modelTree;
};

} // namespace Mayo
