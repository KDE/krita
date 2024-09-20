/*
 *  SPDX-FileCopyrightText: 2019 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISFOREST_H
#define KISFOREST_H

#include <utility>

#include <boost/iterator/iterator_facade.hpp>
#include <boost/iterator/iterator_adaptor.hpp>

#include "KisCppQuirks.h"

#include "kis_assert.h"
#include "kis_debug.h"

namespace KisForestDetail {

template <typename Base>
struct RootNodeImpl
{
    RootNodeImpl<Base> *parent = nullptr;
    Base *firstChild = nullptr;
    Base *lastChild = nullptr;

    inline bool isRoot() const {
        return !parent;
    }
};


template <typename Base>
struct BaseNode : RootNodeImpl<Base>
{
    Base *nextSibling = nullptr;
    Base *prevSibling = nullptr;
};

template <typename T>
struct Node : public BaseNode<Node<T>>
{
    template <typename X>
    explicit Node(X &&newValue)
        : value(std::forward<X>(newValue))
    {
    }

    T value;
};

template <typename T>
using RootNode = RootNodeImpl<Node<T>>;


/*************************************************************************/
/*               BaseIterator                                            */
/*************************************************************************/

template <typename BaseClass, typename T, typename Tag, bool is_const>
class BaseIterator :
        public boost::iterator_facade <BaseClass,
                                       T,
                                       Tag,
                                       std::add_lvalue_reference_t<
                                                       std::add_const_if_t<is_const, T>>>
{
public:
    using value_type = T;
    using NodeType = std::add_const_if_t<is_const, Node<T>>;

    BaseIterator(NodeType *node)
        : m_node(node)
    {
    }

    NodeType* node() const {
        return m_node;
    }

private:
    friend class boost::iterator_core_access;

    typename std::iterator_traits<BaseIterator>::reference dereference() const {
        KIS_ASSERT(m_node);
        return m_node->value;
    }

    bool equal(const BaseClass &other) const {
        return m_node == other.m_node;
    }

protected:
    NodeType *m_node;
};


/*************************************************************************/
/*               ChildIterator                                           */
/*************************************************************************/

/**
 * Child iterator is used to traverse all the children of the current node.
 * It models \c BidirectionalIterator concept, so you can traverse with it in both
 * directions.
 *
 * \code{.cpp}
 *
 * // Forest:
 * //
 * // 0 1
 * //   2
 * //   3 5 6
 * //       7
 * //   4
 * // 8 9
 * //   10
 *
 * KisForest<int>::iterator it0 = findValue(forest, 0);
 *
 * // iterate through all the children of '0'
 * for (auto it = childBegin(it0); it != childEnd(it0); ++it) {
 *     qDebug() << *it;
 * }
 * // prints: 1,2,3,4
 *
 * \endcode
 *
 * It is also possible to convert any iterator type into child iterator
 * via siblingCurrent() function.
 *
 * \code{.cpp}
 *
 * // Forest:
 * //
 * // 0 1
 * //   2
 * //   3 5 6
 * //       7
 * //   4
 * // 8 9
 * //   10
 *
 * KisForest<int>::iterator it2 = findValue(forest, 2);
 *
 * // iterate the children of '0' from '2' to '4'
 * for (auto it = siblingCurrent(it2); it != siblingEnd(it2); ++it) {
 *     qDebug() << *it;
 * }
 * // prints: 2,3,4
 *
 * \endcode
 *
 * WARNING: converting end() iterator to other iterator types currently leads
 *          to undefined behavior.
 */

template <typename T, bool is_const>
class ChildIterator :
        public BaseIterator <ChildIterator<T, is_const>,
                             T,
                             boost::bidirectional_traversal_tag,
                             is_const>
{
    using BaseClass =
        BaseIterator <ChildIterator<T, is_const>,
                     T,
                     boost::bidirectional_traversal_tag,
                     is_const>;

public:
    using RootNodeType = std::add_const_if_t<is_const, RootNode<T>>;
    using NodeType = typename BaseClass::NodeType;

    ChildIterator(NodeType *node, RootNodeType *parent, int offsetToParent)
        : BaseClass(node),
          m_parent(parent),
          m_offsetToParent(offsetToParent)
    {
    }

    operator ChildIterator<T, true>() const {
        return ChildIterator<T, true>(this->m_node, m_parent, m_offsetToParent);
    }

private:
    friend class boost::iterator_core_access;
    template <typename X, bool c>
    friend ChildIterator<X, c> parent(const ChildIterator<X, c> &it);
    template <typename X, bool c>
    friend ChildIterator<X, c> siblingBegin(const ChildIterator<X, c> &it);
    template <typename X, bool c>
    friend ChildIterator<X, c> siblingEnd(const ChildIterator<X, c> &it);
    template <typename X, bool c>
    friend ChildIterator<X, c> childBegin(const ChildIterator<X, c> &it);
    template <typename X, bool c>
    friend ChildIterator<X, c> childEnd(const ChildIterator<X, c> &it);

    template <typename X, bool c>
    friend QDebug operator<<(QDebug dbg, const ChildIterator<X, c> &it);
    template <typename X> friend class Forest;

    void increment() {
        this->m_node = this->m_node->nextSibling;
    }

    void decrement() {
        this->m_node =
            this->m_node ?
            this->m_node->prevSibling :
            (m_parent && m_offsetToParent == 0) ? m_parent->lastChild : nullptr;
    }

    bool equal(const ChildIterator<T, is_const> &other) const {
        return this->m_node == other.m_node &&
            (this->m_node ||
             (this->m_parent == other.m_parent &&
              this->m_offsetToParent == other.m_offsetToParent));
    }

private:
    RootNodeType *m_parent;
    int m_offsetToParent;
};

template <typename value_type, bool is_const>
QDebug operator<<(QDebug dbg, const ChildIterator<value_type, is_const> &it)
{
    if (it.node()) {
        dbg.nospace() << "ChildIterator(" << it.node() << "(" <<  it.node()->value << ")" << ", parent:" << it.m_parent << ")";
    } else {
        dbg.nospace() << "ChildIterator(" << "nullptr" << ", parent:" << it.m_parent << ", offset:" << it.m_offsetToParent << ")";
    }
    return dbg;
}


template <typename value_type, bool is_const>
ChildIterator<value_type, is_const> siblingCurrent(ChildIterator<value_type, is_const> it)
{
    return it;
}

template <typename value_type, bool is_const>
ChildIterator<value_type, is_const> siblingBegin(const ChildIterator<value_type, is_const> &it)
{
    using RootNodeType = typename ChildIterator<value_type, is_const>::RootNodeType;
    if (!it.node() && it.m_offsetToParent != 0) return it;
    RootNodeType *parent = it.m_parent;
    return ChildIterator<value_type, is_const>(parent ? parent->firstChild : nullptr, parent, 0);
}

template <typename value_type, bool is_const>
ChildIterator<value_type, is_const> siblingEnd(const ChildIterator<value_type, is_const> &it)
{
    if (!it.node() && it.m_offsetToParent != 0) return it;
    return ChildIterator<value_type, is_const>(nullptr, it.m_parent, 0);
}

template <typename Iterator,
          bool is_const = std::is_const_v<typename Iterator::NodeType>,
          typename ResultIterator = ChildIterator<typename Iterator::value_type, is_const>>
ResultIterator siblingCurrent(Iterator it)
{
    // TODO: conversion of an end-iterator into a child iterator is still not implemented
    //       and considered as UB. We need to implement all special-cases for that.
    KIS_SAFE_ASSERT_RECOVER_NOOP(it.node());

    return ResultIterator(it.node(), it.node() ? it.node()->parent : nullptr, 0);
}

template <typename Iterator,
          bool is_const = std::is_const_v<typename Iterator::NodeType>,
          typename ResultIterator = ChildIterator<typename Iterator::value_type, is_const>>
ResultIterator siblingBegin(Iterator it)
{
    return siblingBegin(siblingCurrent(it));
}

template <typename Iterator,
          bool is_const = std::is_const_v<typename Iterator::NodeType>,
          typename ResultIterator = ChildIterator<typename Iterator::value_type, is_const>>
ResultIterator siblingEnd(Iterator it)
{
    return siblingEnd(siblingCurrent(it));
}

 template <typename value_type, bool is_const>
 ChildIterator<value_type, is_const> childBegin(const ChildIterator<value_type, is_const> &it)
{
    if (it.node()) {
        return ChildIterator<value_type, is_const>(it.node()->firstChild, it.node(), 0);
    } else {
        return ChildIterator<value_type, is_const>(nullptr, it.m_parent, it.m_offsetToParent + 1);
    }
}

template <typename value_type, bool is_const>
ChildIterator<value_type, is_const> childEnd(const ChildIterator<value_type, is_const> &it)
{
    if (it.node()) {
        return ChildIterator<value_type, is_const>(nullptr, it.node(), 0);
    } else {
        return ChildIterator<value_type, is_const>(nullptr, it.m_parent, it.m_offsetToParent + 1);
    }
}

template <typename Iterator,
          bool is_const = std::is_const_v<typename Iterator::NodeType>,
          typename ResultIterator = ChildIterator<typename Iterator::value_type, is_const>>
ResultIterator childBegin(Iterator it)
{
    return childBegin(siblingCurrent(it));
}

template <typename Iterator,
          bool is_const = std::is_const_v<typename Iterator::NodeType>,
          typename ResultIterator = ChildIterator<typename Iterator::value_type, is_const>>
ResultIterator childEnd(Iterator it)
{
    return childEnd(siblingCurrent(it));
}


template <typename value_type, bool is_const>
ChildIterator<value_type, is_const> parent(const ChildIterator<value_type, is_const> &it)
{
    if (it.m_parent->isRoot()) {
        return ChildIterator<value_type, is_const>(nullptr, it.m_parent, qMax(-1, it.m_offsetToParent - 1));
    } else if (it.m_offsetToParent == 0) {
        using NodeType = typename ChildIterator<value_type, is_const>::NodeType;
        NodeType *parentNode = static_cast<NodeType*>(it.m_parent);
        return ChildIterator<value_type, is_const>(parentNode, parentNode->parent, 0);
    } else {
        return ChildIterator<value_type, is_const>(nullptr, it.m_parent, it.m_offsetToParent - 1);
    }
}

template <typename T, bool is_const>
inline bool isEnd(const ChildIterator<T, is_const> &it) {
    return !it.node();
}


/*************************************************************************/
/*               HierarchyIterator                                       */
/*************************************************************************/

/**
 * Hierarchy iterator is used to traverse from the current node to the root
 * of the current subtree of the forest. It models \c ForwardIterator concept.
 *
 * \code{.cpp}
 *
 * // Forest:
 * //
 * // 0 1
 * //   2
 * //   3 5 6
 * //       7
 * //   4
 * // 8 9
 * //   10
 *
 * KisForest<int>::iterator nodeIt = findValue(forest, 5);
 *
 * // print all the parent nodes for nodeIt, including nodeIt itself
 * for (auto it = hierarchyBegin(nodeIt); it != hierarchyEnd(nodeIt); ++it) {
 *     qDebug() << *it;
 * }
 * // prints: 5,3,0
 *
 * \endcode
 *
 * WARNING: converting end() iterator to other iterator types currently leads
 *          to undefined behavior.
 */

template <typename T, bool is_const>
class HierarchyIterator :
        public BaseIterator <HierarchyIterator<T, is_const>,
                             T,
                             boost::forward_traversal_tag,
                             is_const>
{
    using BaseClass = BaseIterator <HierarchyIterator<T, is_const>,
                                   T,
                                   boost::forward_traversal_tag,
                                   is_const>;
public:
    using RootNodeType = std::add_const_if_t<is_const, RootNode<T>>;
    using NodeType = typename BaseClass::NodeType;

    HierarchyIterator(NodeType *node)
        : BaseClass(node)
    {
    }

    operator HierarchyIterator<T, true>() const {
        return HierarchyIterator<T, true>(this->m_node);
    }

private:
    friend class boost::iterator_core_access;

    void increment() {
        RootNodeType *parent = this->m_node->parent;
        this->m_node =
            static_cast<NodeType*>(
                parent && !parent->isRoot() ?
                parent : nullptr);
    }
};

template <typename Iterator,
          bool is_const = std::is_const_v<typename Iterator::NodeType>,
          typename ResultIterator = HierarchyIterator<typename Iterator::value_type, is_const>>
ResultIterator hierarchyBegin(Iterator it)
{
    return ResultIterator(it.node());
}

template <typename Iterator,
          bool is_const = std::is_const_v<typename Iterator::NodeType>,
          typename ResultIterator = HierarchyIterator<typename Iterator::value_type, is_const>>
ResultIterator hierarchyEnd(Iterator it)
{
    Q_UNUSED(it);
    return ResultIterator(nullptr);
}


/*************************************************************************/
/*               CompositionIterator                                     */
/*************************************************************************/

/**
 * Composition iterator is used to traverse entire child-subtree of the node
 * recursively in depth-first order. Every node it entered twice: first time, when
 * subtree is entered; second time, when subtree is left. To check the current
 * state of the iterator (Enter or Leave) use \c it.state() call.
 *
 * Iterator models \c ForwardIterator concept.
 *
 * \code{.cpp}
 *
 * // Forest:
 * //
 * // 0 1
 * //   2
 * //   3 5 6
 * //       7
 * //   4
 * // 8 9
 * //   10
 *
 * KisForest<int>::iterator it3 = findValue(forest, 3);
 *
 * // iterate through all the children of '3' recursively
 * for (auto it = compositionBegin(it3); it != compositionEnd(it3); ++it) {
 *     qDebug() << *it << it.state();
 * }
 * // prints: (3, Enter)
 * //           (5, Enter)
 * //             (6, Enter)
 * //             (6, Leave)
 * //             (7, Enter)
 * //             (7, Leave)
 * //           (5, Leave)
 * //         (3, Leave)
 *
 * \endcode
 *
 * WARNING: converting end() iterator to other iterator types currently leads
 *          to undefined behavior.
 */

enum TraversalState {
    Enter,
    Leave
};

template <typename T, bool is_const>
class CompositionIterator :
        public boost::iterator_adaptor<
            CompositionIterator<T, is_const>,
            ChildIterator<T, is_const>,
            boost::use_default,
            boost::forward_traversal_tag>
{
    using BaseClass = boost::iterator_adaptor<
        CompositionIterator<T, is_const>,
        ChildIterator<T, is_const>,
        boost::use_default,
        boost::forward_traversal_tag>;

    using BaseIteratorType = typename CompositionIterator::base_type;

public:
    using traversal_state = TraversalState;
    using RootNodeType = typename BaseIteratorType::RootNodeType;
    using NodeType = typename BaseIteratorType::NodeType;

    NodeType* node() const {
        return this->base().node();
    }

public:
    CompositionIterator(NodeType *node, traversal_state state = Enter)
        : BaseClass(BaseIteratorType(node, node ? node->parent : nullptr, 0)),
          m_state(state)
    {
    }

    traversal_state state() const {
        return m_state;
    }

    operator CompositionIterator<T, true>() const {
        return CompositionIterator<T, true>(this->m_node, this->m_state);
    }

private:
    friend class boost::iterator_core_access;

    bool tryJumpToPos(typename CompositionIterator::base_type it,
                      TraversalState newState) {
        bool result = false;

        if (!isEnd(it)) {
            this->base_reference() = it;
            result = true;
            m_state = newState;
        }

        return result;
    }

    void increment() {
        switch (m_state) {
        case Enter:
            if (tryJumpToPos(childBegin(this->base()), Enter)) return;
            if (tryJumpToPos(this->base(), Leave)) return;
            break;
        case Leave:
            if (tryJumpToPos(std::next(this->base()), Enter)) return;
            if (tryJumpToPos(parent(this->base()), Leave)) return;
            break;
        }

        this->base_reference() = BaseIteratorType(nullptr, nullptr, 0);
    }

private:
    traversal_state m_state = Enter;
};

template <typename Iterator,
         bool is_const = std::is_const_v<typename Iterator::NodeType>,
         typename ResultIterator = CompositionIterator<typename Iterator::value_type, is_const>>
ResultIterator compositionBegin(Iterator it)
{
    return ResultIterator(it.node(), Enter);
}

template <typename Iterator,
         bool is_const = std::is_const_v<typename Iterator::NodeType>,
         typename ResultIterator = CompositionIterator<typename Iterator::value_type, is_const>>
ResultIterator compositionEnd(Iterator it)
{
    return it.node() ?
        std::next(ResultIterator(it.node(), Leave)) :
        ResultIterator(nullptr, Leave);
}


/*************************************************************************/
/*               DepthFirstIterator                                      */
/*************************************************************************/

/**
 * Depth-first iterator is used to traverse entire child-subtree of the node
 * recursively in depth-first order. Every node is entered only once. It
 * implements standard recursion iteration:
 *
 *   * \c DepthFirstIterator<T, Enter> implements head-recursion, that is,
 *     the node is visited *before* its children
 *
 *   * \c DepthFirstIterator<T, Leave> implements tail-recursion, that is,
 *     the node is visited *after* its children
 *
 * Use \c subtreeBegin() and \c subtreeEnd() for head-recursion and \c tailSubtreeBegin() and
 * \c tailSubtreeEnd() for tail-recursion.
 *
 * Iterator models \c ForwardIterator concept.
 *
 * \code{.cpp}
 *
 * // Forest:
 * //
 * // 0 1
 * //   2
 * //   3 5 6
 * //       7
 * //   4
 * // 8 9
 * //   10
 *
 * KisForest<int>::iterator it3 = findValue(forest, 3);
 *
 * // iterate through all the children of '3' in head-recursive way
 * for (auto it = subtreeBegin(it3); it != subtreeEnd(it3); ++it) {
 *     qDebug() << *it << it.state();
 * }
 * // prints: 3, 5, 6, 7
 *
 * // iterate through all the children of '3' in tail-recursive way
 * for (auto it = tailSubtreeBegin(it3); it != tailSubtreeEnd(it3); ++it) {
 *     qDebug() << *it << it.state();
 * }
 * // prints: 6, 7, 5, 3
 *
 * \endcode
 *
 * WARNING: converting end() iterator to other iterator types currently leads
 *          to undefined behavior.
 */

template <typename T, TraversalState visit_on, bool is_const>
class DepthFirstIterator :
        public boost::iterator_adaptor<
            DepthFirstIterator<T, visit_on, is_const>,
            CompositionIterator<T, is_const>,
            boost::use_default,
            boost::forward_traversal_tag>
{
    using BaseClass = boost::iterator_adaptor<
        DepthFirstIterator<T, visit_on, is_const>,
        CompositionIterator<T, is_const>,
        boost::use_default,
        boost::forward_traversal_tag>;

    using BaseIteratorType = typename DepthFirstIterator::base_type;

public:
    using traversal_state = TraversalState;
    using RootNodeType = typename BaseIteratorType::RootNodeType;
    using NodeType = typename BaseIteratorType::NodeType;

    DepthFirstIterator(NodeType *node,
                       traversal_state state = traversal_state::Enter,
                       bool shouldSkipToBegin = false)
        : BaseClass(BaseIteratorType(node, state))
    {
        if (shouldSkipToBegin) {
            skipToState(visit_on);
        }
    }

    NodeType* node() const {
        return this->base().node();
    }

    operator DepthFirstIterator<T, visit_on, true>() const {
        return DepthFirstIterator<T, visit_on, true>(this->m_node, this->m_state);
    }

private:
    friend class boost::iterator_core_access;

    void increment() {
        this->base_reference()++;
        skipToState(visit_on);
    }

    void skipToState(TraversalState state) {
        while (this->base().node() && this->base().state() != state) {
            this->base_reference()++;
        }
    }

};

template <typename Iterator,
          bool is_const = std::is_const_v<typename Iterator::NodeType>,
          typename ResultIterator = DepthFirstIterator<typename Iterator::value_type, Enter, is_const>>
ResultIterator subtreeBegin(Iterator it)
{
    return ResultIterator(it.node(), Enter);
}

template <typename Iterator,
          bool is_const = std::is_const_v<typename Iterator::NodeType>,
          typename ResultIterator = DepthFirstIterator<typename Iterator::value_type, Enter, is_const>>
ResultIterator subtreeEnd(Iterator it)
{
    return it.node() ?
        std::next(ResultIterator(it.node(), Leave)) :
        ResultIterator(nullptr, Leave);
}

template <typename Iterator,
          bool is_const = std::is_const_v<typename Iterator::NodeType>,
          typename ResultIterator = DepthFirstIterator<typename Iterator::value_type, Leave, is_const>>
ResultIterator tailSubtreeBegin(Iterator it)
{
    return ResultIterator(it.node(), Enter, true);
}

template <typename Iterator,
          bool is_const = std::is_const_v<typename Iterator::NodeType>,
          typename ResultIterator = DepthFirstIterator<typename Iterator::value_type, Leave, is_const>>
ResultIterator tailSubtreeEnd(Iterator it)
{
    return it.node() ?
        std::next(ResultIterator(it.node(), Leave)) :
        ResultIterator(nullptr, Leave);
}


/*************************************************************************/
/*               Forest                                                  */
/*************************************************************************/

/**
 * Forest implements a container for composing tree-like structures from
 * arbitrary objects.
 *
 * All add/remove operations are implemented via child_iterator. You can
 * convert any iterator type into a child iterator via \c siblingCurrent()
 * function.
 *
 * * \code{.cpp}
 *
 * // Forest:
 * //
 * // 0 1
 * //   2
 * //   3 5 6
 * //       7
 * //   4
 * // 8 9
 * //   10
 *
 * KisForest<int> forest;
 *
 * auto it0 = forest.insert(childBegin(forest), 0);
 * auto it8 = forest.insert(childEnd(forest), 8);
 *
 * auto it1 = forest.insert(childEnd(it0), 1);
 * auto it2 = forest.insert(childEnd(it0), 2);
 * auto it3 = forest.insert(childEnd(it0), 3);
 * auto it4 = forest.insert(childEnd(it0), 4);
 *
 * auto it5 = forest.insert(childEnd(it3), 5);
 *
 * auto it6 = forest.insert(childEnd(it5), 6);
 * auto it7 = forest.insert(childEnd(it5), 7);
 *
 * auto it9 = forest.insert(childEnd(it8), 9);
 * auto it10 = forest.insert(childEnd(it8), 10);
 *
 * // iterate through all elements of the forest
 * for (auto it = subtreeBegin(forest); it != subtreeEnd(forest); ++it) {
 *     qDebug() << *it << it.state();
 * }
 * // prints: 0,1,2,3,5,6,7,4,8,9,10
 *
 * \endcode
 *
 *
 */

template <typename T>
class Forest
{
public:
    Forest()
    {
    }

    ~Forest() {
        erase(childBegin(), childEnd());
    }

    Forest(const Forest<T> &rhs) {
        for (auto it = rhs.childBegin(); it != rhs.childEnd(); ++it) {
            auto cloneIt = this->insert(this->childEnd(), *it);
            cloneChildren(it, cloneIt);
        }
    }

    Forest<T>& operator=(const Forest<T> &rhs) {
        erase(childBegin(), childEnd());
        for (auto it = rhs.childBegin(); it != rhs.childEnd(); ++it) {
            auto cloneIt = this->insert(this->childEnd(), *it);
            cloneChildren(it, cloneIt);
        }
        return *this;
    }

    using value_type = T;

    using child_iterator = ChildIterator<T, false>;
    using const_child_iterator = ChildIterator<T, true>;

    using composition_iterator = CompositionIterator<T, false>;
    using const_composition_iterator = CompositionIterator<T, true>;

    using hierarchy_iterator = HierarchyIterator<T, false>;
    using const_hierarchy_iterator = HierarchyIterator<T, true>;

    using iterator = DepthFirstIterator<T, Enter, false>;
    using const_iterator = DepthFirstIterator<T, Enter, true>;

    using depth_first_tail_iterator = DepthFirstIterator<T, Leave, false>;
    using const_depth_first_tail_iterator = DepthFirstIterator<T, Leave, true>;

    bool empty() const {
        return !m_root.firstChild;
    }

    void swap(Forest &other) {
        std::swap(m_root, other.m_root);
    }

    iterator begin() {
        return beginImpl(*this);
    }

    iterator end() {
        return endImpl(*this);
    }

    const_iterator begin() const {
        return beginImpl(*this);
    }

    const_iterator end() const {
        return endImpl(*this);
    }

    const_iterator constBegin() const {
        return beginImpl(*this);
    }

    const_iterator constEnd() const {
        return endImpl(*this);
    }

    depth_first_tail_iterator depthFirstTailBegin() {
        return tailSubtreeBegin(begin());
    }

    depth_first_tail_iterator depthFirstTailEnd() {
        return tailSubtreeEnd(end());
    }

    const_depth_first_tail_iterator depthFirstTailBegin() const {
        return tailSubtreeBegin(constBegin());
    }

    const_depth_first_tail_iterator depthFirstTailEnd() const {
        return tailSubtreeEnd(constEnd());
    }

    const_depth_first_tail_iterator constDepthFirstTailBegin() const {
        return tailSubtreeBegin(constBegin());
    }

    const_depth_first_tail_iterator constDepthFirstTailEnd() const {
        return tailSubtreeEnd(constEnd());
    }

    child_iterator childBegin() {
        return childBeginImpl(*this);
    }

    child_iterator childEnd() {
        return childEndImpl(*this);
    }

    child_iterator parentEnd() {
        return parentEndImpl(*this);
    }

    const_child_iterator childBegin() const {
        return childBeginImpl(*this);
    }

    const_child_iterator childEnd() const {
        return childEndImpl(*this);
    }

    const_child_iterator parentEnd() const {
        return parentEndImpl(*this);
    }

    const_child_iterator constChildBegin() const {
        return childBeginImpl(*this);
    }

    const_child_iterator constChildEnd() const {
        return childEndImpl(*this);
    }

    const_child_iterator constParentEnd() const {
        return parentEndImpl(*this);
    }

    composition_iterator compositionBegin() {
        return compositionBeginImpl(*this);
    }

    composition_iterator compositionEnd() {
        return compositionEndImpl(*this);
    }

    const_composition_iterator compositionBegin() const {
        return compositionBeginImpl(*this);
    }

    const_composition_iterator compositionEnd() const {
        return compositionEndImpl(*this);
    }

    const_composition_iterator constCompositionBegin() const {
        return compositionBeginImpl(*this);
    }

    const_composition_iterator constCompositionEnd() const {
        return compositionEndImpl(*this);
    }

    /**
     * @brief Inserts element \p value into position \p pos.
     * \p value becomes the child of the same parent as \p pos
     * and is placed right before \p pos.
     * @return iterator pointing to the inserted element
     */
    template <typename X>
    child_iterator insert(child_iterator pos, X &&value) {
        Node<T> *node = new Node<T>(std::forward<X>(value));

        linkNode(pos, node);

        return child_iterator(node, node->parent, 0);
    }

    /**
     * @brief Removes element at position \p pos.
     * If \p pos is 'end', then result is undefined.
     * @return the element following \p pos
     */
    child_iterator erase(child_iterator pos) {
        child_iterator nextNode = std::next(pos);
        Node<T> *node = pos.node();

        Node<T> *lastNode = node;
        for (auto it = tailSubtreeBegin(pos); it != tailSubtreeEnd(pos); ++it) {

            if (lastNode != node) {
                delete lastNode;
            }
            lastNode = it.node();
        }

        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(lastNode == node, pos);

        unlinkNode(node);
        delete node;

        return nextNode;
    }

    /**
     * @brief erases all elements from \p pos to \p end (excluding \p end)
     * @return \p end
     */
    child_iterator erase(child_iterator pos, child_iterator end) {
        while (pos != end) {
            pos = erase(pos);
        }
        return pos;
    }

    /**
     * @brief move a subtree to new position
     * Moves \p subtree into a new position pointer by \p newPos. \p newPos
     * must not be inside the subtree, otherwise cycling links may appear.
     * @return iterator to a new position of \p subtree
     */
    child_iterator move(child_iterator subtree, child_iterator newPos) {
        // sanity check for cycling move
        for (auto it = hierarchyBegin(newPos); it != hierarchyEnd(newPos); ++it) {
            KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(it.node() != subtree.node(), subtree);
        }

        Node<T> *node = subtree.node();
        unlinkNode(node);

        node->prevSibling = nullptr;
        node->nextSibling = nullptr;
        node->parent = nullptr;

        linkNode(newPos, node);
        return child_iterator(node, node->parent, 0);
    }

private:

    inline void linkBefore(Node<T> *node, Node<T> *beforeMe) {
        if (beforeMe) {
            node->nextSibling = beforeMe;
            beforeMe->prevSibling = node;
        }
    }

    inline void linkAfter(Node<T> *node, Node<T> *afterMe) {
        if (afterMe) {
            node->prevSibling = afterMe;
            afterMe->nextSibling = node;
        }
    }

    inline void linkNode(child_iterator pos, Node<T> *node) {
        Node<T> *nextNode = pos.node();
        RootNode<T> *parentNode = pos.m_parent;

        Node<T> *prevNode = nextNode ?
                            nextNode->prevSibling :
                            parentNode ? parentNode->lastChild : m_root.lastChild;

        linkAfter(node, prevNode);
        linkBefore(node, nextNode);

        KIS_SAFE_ASSERT_RECOVER_RETURN(parentNode);
        if (!nextNode) {
            parentNode->lastChild = node;
        }

        if (nextNode == parentNode->firstChild) {
            parentNode->firstChild = node;
        }
        node->parent = parentNode;
    }

    inline void unlinkNode(Node<T> *node) {
        RootNode<T> *parentNode = node->parent;

        if (node->nextSibling) {
            node->nextSibling->prevSibling = node->prevSibling;
        }

        if (node->prevSibling) {
            node->prevSibling->nextSibling = node->nextSibling;
        }

        KIS_SAFE_ASSERT_RECOVER_RETURN(parentNode);
        if (parentNode->firstChild == node) {
            parentNode->firstChild = node->nextSibling;
        }

        if (parentNode->lastChild == node) {
            parentNode->lastChild = node->prevSibling;
        }
    }

    void cloneChildren(const_child_iterator oldParent, child_iterator parent) {
        auto it = KisForestDetail::childBegin(oldParent);
        auto end = KisForestDetail::childEnd(oldParent);
        for (;it != end; ++it) {
            auto itClone = this->insert(KisForestDetail::childEnd(parent), *it);
            cloneChildren(it, itClone);
        }
    }

private:
    template <class ForestType,
              class IteratorType = ChildIterator<typename ForestType::value_type,
                                                 std::is_const_v<ForestType>>>
    static IteratorType childBeginImpl(ForestType &forest) {
        return IteratorType(forest.m_root.firstChild, &forest.m_root, 0);
    }

    template <class ForestType,
              class IteratorType = ChildIterator<typename ForestType::value_type,
                                                 std::is_const_v<ForestType>>>
    static IteratorType childEndImpl(ForestType &forest) {
        return IteratorType(nullptr, &forest.m_root, 0);
    }

    template <class ForestType,
              class IteratorType = ChildIterator<typename ForestType::value_type,
                                                 std::is_const_v<ForestType>>>
    static IteratorType parentEndImpl(ForestType &forest) {
        return IteratorType(nullptr, &forest.m_root, -1);
    }

    template <class ForestType,
              class IteratorType = DepthFirstIterator<typename ForestType::value_type,
                                                      Enter,
                                                      std::is_const_v<ForestType>>>
    static IteratorType beginImpl(ForestType &forest) {
        return IteratorType(forest.m_root.firstChild);
    }

    template <class ForestType,
             class IteratorType = DepthFirstIterator<typename ForestType::value_type,
                                                     Enter,
                                                     std::is_const_v<ForestType>>>
    static IteratorType endImpl(ForestType &forest) {
        Q_UNUSED(forest);
        return IteratorType(nullptr);
    }

    template <class ForestType,
              class IteratorType = CompositionIterator<typename ForestType::value_type,
                                                      std::is_const_v<ForestType>>>
    static IteratorType compositionBeginImpl(ForestType &forest) {
        return IteratorType(forest.m_root.firstChild);
    }

    template <class ForestType,
              class IteratorType = CompositionIterator<typename ForestType::value_type,
                                                       std::is_const_v<ForestType>>>
    static IteratorType compositionEndImpl(ForestType &forest) {
        Q_UNUSED(forest);
        return IteratorType(nullptr);
    }

private:
    RootNode<T> m_root;
};

template <typename T>
typename Forest<T>::child_iterator childBegin(Forest<T> &forest)
{
    return forest.childBegin();
}

template <typename T>
typename Forest<T>::const_child_iterator childBegin(const Forest<T> &forest)
{
    return forest.childBegin();
}


template <typename T>
typename Forest<T>::child_iterator childEnd(Forest<T> &forest)
{
    return forest.childEnd();
}

template <typename T>
typename Forest<T>::const_child_iterator childEnd(const Forest<T> &forest)
{
    return forest.childEnd();
}

template <typename T>
typename Forest<T>::composition_iterator compositionBegin(Forest<T> &forest)
{
    return forest.compositionBegin();
}

template <typename T>
typename Forest<T>::const_composition_iterator compositionBegin(const Forest<T> &forest)
{
    return forest.compositionBegin();
}


template <typename T>
typename Forest<T>::composition_iterator compositionEnd(Forest<T> &forest)
{
    return forest.compositionEnd();
}

template <typename T>
typename Forest<T>::const_composition_iterator compositionEnd(const Forest<T> &forest)
{
    return forest.compositionEnd();
}


template <typename T>
typename Forest<T>::depth_first_tail_iterator tailSubtreeBegin(Forest<T> &forest)
{
    return forest.depthFirstTailBegin();
}

template <typename T>
typename Forest<T>::const_depth_first_tail_iterator tailSubtreeBegin(const Forest<T> &forest)
{
    return forest.depthFirstTailBegin();
}

template <typename T>
typename Forest<T>::depth_first_tail_iterator tailSubtreeEnd(Forest<T> &forest)
{
    return forest.depthFirstTailEnd();
}

template <typename T>
typename Forest<T>::const_depth_first_tail_iterator tailSubtreeEnd(const Forest<T> &forest)
{
    return forest.depthFirstTailEnd();
}

template <typename T>
int depth(typename Forest<T>::const_child_iterator beginIt,
          typename Forest<T>::const_child_iterator endIt)
{

    int currentDepth = 0;

    for (auto it = beginIt; it != endIt; ++it) {
        currentDepth = std::max(currentDepth, 1 + depth<T>(childBegin(it), childEnd(it)));
    }

    return currentDepth;
}

template <typename T>
int depth(const Forest<T> &forest) {
    return depth<T>(childBegin(forest), childEnd(forest));
}

template <typename T>
int size(const Forest<T> &forest) {
    return std::distance(begin(forest), end(forest));
}

using std::begin;
using std::end;
using std::make_reverse_iterator;

using std::find;
using std::find_if;
using std::find_if_not;
}

template<typename T>
using KisForest = KisForestDetail::Forest<T>;


#endif // KISFOREST_H
