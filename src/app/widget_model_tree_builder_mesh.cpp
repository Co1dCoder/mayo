/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#include "widget_model_tree_builder_mesh.h"
#include "../base/caf_utils.h"
#include "theme.h"
#include "widget_model_tree.h"

#include <QtWidgets/QTreeWidgetItem>
#include <TDataXtd_Triangulation.hxx>

namespace Mayo {

bool WidgetModelTreeBuilder_Mesh::supportsEntity(const DocumentTreeNode& node) const
{
    return CafUtils::hasAttribute<TDataXtd_Triangulation>(node.label());
}

QTreeWidgetItem* WidgetModelTreeBuilder_Mesh::createTreeItem(const DocumentTreeNode& node)
{
    Expects(this->supportsEntity(node));

    auto treeItem = WidgetModelTreeBuilder::createTreeItem(node);
    treeItem->setIcon(0, mayoTheme()->icon(Theme::Icon::ItemMesh));
    return treeItem;
}

std::unique_ptr<WidgetModelTreeBuilder> WidgetModelTreeBuilder_Mesh::clone() const
{
    return std::make_unique<WidgetModelTreeBuilder_Mesh>();
}

} // namespace Mayo
