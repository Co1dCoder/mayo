/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#if 0
#pragma once

#include "gpx_document.h"
#include "../base/xde_document_item.h"
#include <XCAFPrs_AISObject.hxx>
#include <QtGui/QColor>
#include <unordered_set>

namespace Mayo {

class GpxXdeDocumentItem : public GpxDocumentItem {
    Q_DECLARE_TR_FUNCTIONS(Mayo::GpxXdeDocumentItem)
public:
    enum SelectionMode {
        SelectVertex,
        SelectEdge,
        SelectWire,
        SelectFace,
        SelectShell,
        SelectSolid
    };

    GpxXdeDocumentItem(XdeDocumentItem* item);
    ~GpxXdeDocumentItem();

    XdeDocumentItem* documentItem() const override;

    void setVisible(bool on) override;
    void activateSelection(int mode) override;
    std::vector<Handle_SelectMgr_EntityOwner> entityOwners(int mode) const override;
    Bnd_Box boundingBox() const override;

    PropertyInt propertyTransparency;
    PropertyEnumeration propertyDisplayMode;

    enum DisplayMode {
        DisplayMode_Wireframe,
        DisplayMode_Shaded,
        DisplayMode_ShadedWithFaceBoundary
    };
    static const Enumeration& enumDisplayMode();

    struct DefaultValues {
        Graphic3d_NameOfMaterial material = Graphic3d_NOM_PLASTIC;
        QColor color = Qt::gray;
    };

    static const DefaultValues& defaultValues();
    static void setDefaultValues(const DefaultValues& values);

protected:
    void onPropertyChanged(Property* prop) override;

private:
    XdeDocumentItem* m_xdeDocItem = nullptr;
    std::vector<Handle_XCAFPrs_AISObject> m_vecXdeGpx;
    bool m_selectionActivated = false;
    std::unordered_set<SelectionMode> m_setActivatedSelectionMode;
};

} // namespace Mayo
#endif
