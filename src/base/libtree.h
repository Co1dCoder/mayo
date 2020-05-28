/****************************************************************************
** Copyright (c) 2020, Fougue Ltd. <http://www.fougue.pro>
** All rights reserved.
** See license at https://github.com/fougue/mayo/blob/master/LICENSE.txt
****************************************************************************/

#pragma once

#include "span.h"
#include <vector>

namespace Mayo {

using TreeNodeId = uint32_t;

template<typename T> class Tree {
public:
    Tree();

    TreeNodeId nodeSiblingPrevious(TreeNodeId id) const;
    TreeNodeId nodeSiblingNext(TreeNodeId id) const;
    TreeNodeId nodeChildFirst(TreeNodeId id) const;
    TreeNodeId nodeChildLast(TreeNodeId id) const;
    TreeNodeId nodeParent(TreeNodeId id) const;
    const T& nodeData(TreeNodeId id) const;
    bool nodeIsRoot(TreeNodeId id) const;
    Span<const TreeNodeId> roots() const;

    void clear();
    TreeNodeId appendChild(TreeNodeId parentId, const T& data);
    void remove(TreeNodeId id);

private:
    struct TreeNode {
        TreeNodeId siblingPrevious;
        TreeNodeId siblingNext;
        TreeNodeId childFirst;
        TreeNodeId childLast;
        TreeNodeId parent;
        T data;
    };

    TreeNodeId lastNodeId() const;
    TreeNode* ptrNode(TreeNodeId id);
    const TreeNode* ptrNode(TreeNodeId id) const;

    std::vector<TreeNode> m_vecNode;
    std::vector<TreeNodeId> m_vecRoot;
};

template<typename T, typename FUNC>
void deepForeachTreeNode(const Tree<T>& tree, const FUNC& func);

template<typename T, typename FUNC>
void deepForeachTreeNode(TreeNodeId node, const Tree<T>& tree, const FUNC& func);



// --
// -- Implementation
// --

template<typename T> Tree<T>::Tree() {}

template<typename T> TreeNodeId Tree<T>::nodeSiblingPrevious(TreeNodeId id) const {
    const TreeNode* node = this->ptrNode(id);
    return node ? node->siblingPrevious : 0;
}

template<typename T> TreeNodeId Tree<T>::nodeSiblingNext(TreeNodeId id) const {
    const TreeNode* node = this->ptrNode(id);
    return node ? node->siblingNext : 0;
}

template<typename T> TreeNodeId Tree<T>::nodeChildFirst(TreeNodeId id) const {
    const TreeNode* node = this->ptrNode(id);
    return node ? node->childFirst : 0;
}

template<typename T> TreeNodeId Tree<T>::nodeChildLast(TreeNodeId id) const {
    const TreeNode* node = this->ptrNode(id);
    return node ? node->childLast : 0;
}

template<typename T> TreeNodeId Tree<T>::nodeParent(TreeNodeId id) const {
    const TreeNode* node = this->ptrNode(id);
    return node ? node->parent : 0;
}

template<typename T> const T& Tree<T>::nodeData(TreeNodeId id) const {
    static const T nullObject = {};
    const TreeNode* node = this->ptrNode(id);
    return node ? node->data : nullObject;
}

template<typename T> bool Tree<T>::nodeIsRoot(TreeNodeId id) const
{
    const TreeNode* node = this->ptrNode(id);
    return node ? node->parent == 0 : false;
}

template<typename T> void Tree<T>::clear()
{
    m_vecNode.clear();
    m_vecRoot.clear();
}

template<typename T>
TreeNodeId Tree<T>::appendChild(TreeNodeId parentId, const T& data)
{
    m_vecNode.push_back({});
    const TreeNodeId nodeId = this->lastNodeId();
    TreeNode* node = &m_vecNode.back();
    node->data = data;
    node->parent = parentId;
    node->siblingPrevious = this->nodeChildLast(parentId);
    if (parentId != 0) {
        TreeNode* parentNode = this->ptrNode(parentId);
        if (parentNode->childFirst == 0)
            parentNode->childFirst = nodeId;

        if (parentNode->childLast != 0)
            this->ptrNode(parentNode->childLast)->siblingNext = nodeId;

        parentNode->childLast = nodeId;
    }
    else {
        m_vecRoot.push_back(nodeId);
    }

    return nodeId;
}

template<typename T> void Tree<T>::remove(TreeNodeId id)
{
    Expects(false && "not implemented yet");
}

template<typename T>
Span<const TreeNodeId> Tree<T>::roots() const {
    return m_vecRoot;
}

template<typename T>
TreeNodeId Tree<T>::lastNodeId() const {
    return static_cast<TreeNodeId>(m_vecNode.size());
}

template<typename T>
typename Tree<T>::TreeNode* Tree<T>::ptrNode(TreeNodeId id)
{
    return id != 0 && id <= m_vecNode.size() ? &m_vecNode.at(id - 1) : nullptr;
}

template<typename T>
const typename Tree<T>::TreeNode* Tree<T>::ptrNode(TreeNodeId id) const
{
    return id != 0 && id <= m_vecNode.size() ? &m_vecNode.at(id - 1) : nullptr;
}

template<typename T, typename FUNC>
void deepForeachTreeNode(TreeNodeId node, const Tree<T>& tree, const FUNC& func)
{
    func(node);
    const TreeNodeId childFirst = tree.nodeChildFirst(node);
    for (auto it = childFirst; it != 0; it = tree.nodeSiblingNext(it))
        deepForeachTreeNode(it, tree, func);
}

template<typename T, typename FUNC>
void deepForeachTreeNode(const Tree<T>& tree, const FUNC& func)
{
    for (TreeNodeId node : tree.roots())
        deepForeachTreeNode(node, tree, func);
}

} // namespace Mayo
