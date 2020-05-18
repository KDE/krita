/*
 *  Copyright (c) 2019 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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

template <typename BaseClass, typename T, typename Tag>
class BaseIterator :
        public boost::iterator_facade <BaseClass,
                                       T,
                                       Tag>
{
public:
    using value_type = T;

    BaseIterator(Node<T> *node)
        : m_node(node)
    {
    }

    Node<T>* node() const {
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
    Node<T> *m_node;
};


/*************************************************************************/
/*               ChildIterator                                           */
/*************************************************************************/

/**
 * Child iterator is used to traverse all the the children of the current node.
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

template <typename T>
class ChildIterator :
        public BaseIterator <ChildIterator<T>,
                              T,
                              boost::bidirectional_traversal_tag>
{
public:
    ChildIterator(Node<T> *node, RootNode<T> *parent)
        : BaseIterator<ChildIterator<T>, T, boost::bidirectional_traversal_tag>(node),
          m_parent(parent)
    {
    }

private:
    friend class boost::iterator_core_access;
    template <typename X>
    friend ChildIterator<X> parent(const ChildIterator<X> &it);
    template <typename X> friend class Forest;

    void increment() {
        this->m_node = this->m_node->nextSibling;
    }

    void decrement() {
        this->m_node =
            this->m_node ?
            this->m_node->prevSibling :
            m_parent ? m_parent->lastChild : nullptr;
    }

    bool equal(const ChildIterator<T> &other) const {
        return this->m_node == other.m_node &&
             (this->m_node || this->m_parent == other.m_parent);
    }

private:
    RootNode<T> *m_parent;
};

template <typename Iterator,
          typename ResultIterator = ChildIterator<typename Iterator::value_type>>
ResultIterator siblingBegin(Iterator it)
{
    using RootNodeType = RootNode<typename Iterator::value_type>;

    RootNodeType *parent = it.node() ? it.node()->parent : nullptr;
    return ResultIterator(parent ? parent->firstChild : nullptr, parent);
}


template <typename Iterator,
          typename ResultIterator = ChildIterator<typename Iterator::value_type>>
ResultIterator siblingCurrent(Iterator it)
{
    return ResultIterator(it.node(), it.node() ? it.node()->parent : nullptr);
}

template <typename Iterator,
          typename ResultIterator = ChildIterator<typename Iterator::value_type>>
ResultIterator siblingEnd(Iterator it)
{
    return ResultIterator(nullptr, it.node() ? it.node()->parent : nullptr);
}

template <typename Iterator,
          typename ResultIterator = ChildIterator<typename Iterator::value_type>>
ResultIterator childBegin(Iterator it)
{
    return ResultIterator(it.node() ? it.node()->firstChild : nullptr, it.node());
}

template <typename Iterator,
          typename ResultIterator = ChildIterator<typename Iterator::value_type>>
ResultIterator childEnd(Iterator it)
{
    return ResultIterator(nullptr, it.node());
}

template <typename T>
ChildIterator<T> parent(const ChildIterator<T> &it)
{
    Node<T> *parent = static_cast<Node<T>*>(it.m_parent && !it.m_parent->isRoot() ? it.m_parent : nullptr);
    return ChildIterator<T>(parent, parent ? parent->parent : 0);
}

template <typename T>
inline bool isEnd(const ChildIterator<T> &it) {
    return !it.node();
}


/*************************************************************************/
/*               HierarchyIterator                                       */
/*************************************************************************/

/**
 * Hierarchy iterator is used to traverse from the current node to the root
 * of the the current subtree of the forest. It models \c ForwardIterator concept.
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

template <typename T>
class HierarchyIterator :
        public BaseIterator <HierarchyIterator<T>,
                              T,
                              boost::forward_traversal_tag>
{
public:
    HierarchyIterator(Node<T> *node)
        : BaseIterator<HierarchyIterator<T>, T, boost::forward_traversal_tag>(node)
    {
    }


private:
    friend class boost::iterator_core_access;

    void increment() {
        RootNode<T> *parent = this->m_node->parent;
        this->m_node =
            static_cast<Node<T>*>(
                parent && !parent->isRoot() ?
                parent : nullptr);
    }
};

template <typename Iterator,
          typename ResultIterator = HierarchyIterator<typename Iterator::value_type>>
ResultIterator hierarchyBegin(Iterator it)
{
    return ResultIterator(it.node());
}

template <typename Iterator,
          typename ResultIterator = HierarchyIterator<typename Iterator::value_type>>
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

template <typename T>
class CompositionIterator :
        public boost::iterator_adaptor<
            CompositionIterator<T>,
            ChildIterator<T>,
            boost::use_default,
            boost::forward_traversal_tag>
{
    using BaseClass = boost::iterator_adaptor<
        CompositionIterator<T>,
        ChildIterator<T>,
        boost::use_default,
        boost::forward_traversal_tag>;

public:
    using traversal_state = TraversalState;

    Node<T>* node() const {
        return this->base().node();
    }

public:
    CompositionIterator(Node<T> *node, traversal_state state = Enter)
        : BaseClass(ChildIterator<T>(node, node ? node->parent : nullptr)),
          m_state(state)
    {
    }

    traversal_state state() const {
        return m_state;
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

        this->base_reference() = ChildIterator<T>(nullptr, nullptr);
    }

private:
    traversal_state m_state = Enter;
};

template <typename Iterator>
CompositionIterator<typename Iterator::value_type> compositionBegin(Iterator it)
{
    using CompositionIterator = CompositionIterator<typename Iterator::value_type>;
    return CompositionIterator(it.node(), Enter);
}

template <typename Iterator>
CompositionIterator<typename Iterator::value_type> compositionEnd(Iterator it)
{
    using CompositionIterator = CompositionIterator<typename Iterator::value_type>;
    return it.node() ?
        std::next(CompositionIterator(it.node(), Leave)) :
        CompositionIterator(nullptr, Leave);
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

template <typename T, TraversalState visit_on>
class DepthFirstIterator :
        public boost::iterator_adaptor<
            DepthFirstIterator<T, visit_on>,
            CompositionIterator<T>,
            boost::use_default,
            boost::forward_traversal_tag>
{
    using BaseClass = boost::iterator_adaptor<
        DepthFirstIterator<T, visit_on>,
        CompositionIterator<T>,
        boost::use_default,
        boost::forward_traversal_tag>;

public:
    using traversal_state = TraversalState;

    DepthFirstIterator(Node<T> *node,
                       traversal_state state = traversal_state::Enter,
                       bool shouldSkipToBegin = false)
        : BaseClass(CompositionIterator<T>(node, state))
    {
        if (shouldSkipToBegin) {
            skipToState(visit_on);
        }
    }

    Node<T>* node() const {
        return this->base().node();
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
          typename ResultIterator = DepthFirstIterator<typename Iterator::value_type, Enter>>
ResultIterator subtreeBegin(Iterator it)
{
    return ResultIterator(it.node(), Enter);
}

template <typename Iterator,
          typename ResultIterator = DepthFirstIterator<typename Iterator::value_type, Enter>>
ResultIterator subtreeEnd(Iterator it)
{
    return it.node() ?
        std::next(ResultIterator(it.node(), Leave)) :
        ResultIterator(nullptr, Leave);
}

template <typename Iterator,
          typename ResultIterator = DepthFirstIterator<typename Iterator::value_type, Leave>>
ResultIterator tailSubtreeBegin(Iterator it)
{
    return ResultIterator(it.node(), Enter, true);
}

template <typename Iterator,
          typename ResultIterator = DepthFirstIterator<typename Iterator::value_type, Leave>>
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

    using child_iterator = ChildIterator<T>;
    using composition_iterator = CompositionIterator<T>;
    using hierarchy_iterator = HierarchyIterator<T>;
    using iterator = DepthFirstIterator<T, Enter>;
    using depth_first_tail_iterator = DepthFirstIterator<T, Leave>;

    iterator begin() {
        return iterator(m_root.firstChild);
    }

    iterator end() {
        return iterator(nullptr);
    }

    depth_first_tail_iterator depthFirstTailBegin() {
        return tailSubtreeBegin(begin());
    }

    depth_first_tail_iterator depthFirstTailEnd() {
        return tailSubtreeEnd(end());
    }

    child_iterator childBegin() {
        return child_iterator(m_root.firstChild, &m_root);
    }

    child_iterator childEnd() {
        return child_iterator(nullptr, &m_root);
    }

    composition_iterator compositionBegin() {
        return composition_iterator(m_root.firstChild);
    }

    composition_iterator compositionEnd() {
        return composition_iterator(nullptr);
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

        return child_iterator(node, node->parent);
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
        return child_iterator(node, node->parent);
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
private:
    RootNode<T> m_root;
};

template <typename T>
typename Forest<T>::child_iterator childBegin(Forest<T> &forest)
{
    return forest.childBegin();
}

template <typename T>
typename Forest<T>::child_iterator childEnd(Forest<T> &forest)
{
    return forest.childEnd();
}

template <typename T>
typename Forest<T>::composition_iterator compositionBegin(Forest<T> &forest)
{
    return forest.compositionBegin();
}

template <typename T>
typename Forest<T>::composition_iterator compositionEnd(Forest<T> &forest)
{
    return forest.compositionEnd();
}

template <typename T>
typename Forest<T>::depth_first_tail_iterator tailSubtreeBegin(Forest<T> &forest)
{
    return forest.depthFirstTailBegin();
}

template <typename T>
typename Forest<T>::depth_first_tail_iterator tailSubtreeEnd(Forest<T> &forest)
{
    return forest.depthFirstTailEnd();
}

template <typename T>
int depth(typename Forest<T>::child_iterator beginIt,
          typename Forest<T>::child_iterator endIt)
{

    int currentDepth = 0;

    for (auto it = beginIt; it != endIt; ++it) {
        currentDepth = std::max(currentDepth, 1 + depth<T>(childBegin(it), childEnd(it)));
    }

    return currentDepth;
}

template <typename T>
int depth(Forest<T> &forest) {
    return depth<T>(childBegin(forest), childEnd(forest));
}

template <typename T>
int size(Forest<T> &forest) {
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
