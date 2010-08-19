/* This file is part of the KDE project
   Copyright (c) 2006 Thorsten Zachmann <zachmann@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.

   Based on code from Wolfgang Baer - WBaer@gmx.de
*/

#ifndef KORTREE_H
#define KORTREE_H

#include <KDebug>

#include <QtCore/QPair>
#include <QtCore/QMap>
#include <QtCore/QList>
#include <QtCore/QVector>
#include <QtCore/QPointF>
#include <QtCore/QRectF>
#include <QtCore/QVarLengthArray>

// #define KOFFICE_RTREE_DEBUG
#ifdef KOFFICE_RTREE_DEBUG
#include <QtGui/QPainter>
#endif

/**
 * @brief The KoRTree class is a template class that provides a R-tree.
 *
 * This class implements a R-tree as described in
 * "R-TREES. A DYNAMIC INDEX STRUCTURE FOR SPATIAL SEARCHING" by Antomn Guttman
 *
 * It only supports 2 dimensional bounding boxes which are repesented by a QRectF.
 * For node splitting the Quadratic-Cost Algorithm is used as descibed by Guttman.
 */
template <typename T>
class KoRTree
{
public:
    /**
     * @brief Constructor
     *
     * @param capacity the capacity a node can take
     * @param minimum the minimum filling of a node max 0.5 * capacity
     */
    KoRTree(int capacity, int minimum);

    /**
     * @brief Destructor
     */
    virtual ~KoRTree();

    /**
     * @brief Insert data item into the tree
     *
     * This will insert a data item into the tree. If necessary the tree will
     * adjust itself.
     *
     * @param data
     * @param bb
     */
    virtual void insert(const QRectF& bb, const T& data);

    /**
     * @brief Remove a data item from the tree
     *
     * This removed a data item from the tree. If necessary the tree will
     * adjust itself.
     *
     * @param data
     */
    void remove(const T& data);

    /**
     * @brief Find all data items which intersects rect
     * The items are sorted by insertion time in ascending order.
     *
     * @param rect where the objects have to be in
     *
     * @return objects intersecting the rect
     */
    virtual QList<T> intersects(const QRectF& rect) const;

    /**
     * @brief Find all data item which contain the point
     * The items are sorted by insertion time in ascending order.
     *
     * @param point which should be contained in the objects
     *
     * @return objects which contain the point
     */
    QList<T> contains(const QPointF &point) const;

    /**
     * @brief Find all data rectangles
     * The order is guaranteed to be the same as that used by values().
     *
     * @return a list containing all the data rectangles used in the tree
     */
    QList<QRectF> keys() const;

    /**
     * @brief Find all data items
     * The order is guaranteed to be the same as that used by keys().
     *
     * @return a list containing all the data used in the tree
     */
    QList<T> values() const;

    void clear() {
        delete m_root;
        m_root = createLeafNode(m_capacity + 1, 0, 0);
        m_leafMap.clear();
    }

#ifdef KOFFICE_RTREE_DEBUG
    /**
     * @brief Paint the tree
     *
     * @param p painter which should be used for painting
     */
    void paint(QPainter & p) const;

    /**
     * @brief Print the tree using qdebug
     */
    void debug() const;
#endif

protected:
    class NonLeafNode;
    class LeafNode;

    class Node
    {
    public:
#ifdef KOFFICE_RTREE_DEBUG
        static int nodeIdCnt;
#endif
        Node(int capacity, int level, Node * parent);
        virtual ~Node() {}

        virtual void remove(int index);
        // move node between nodes of the same type from node
        virtual void move(Node * node, int index) = 0;

        virtual LeafNode * chooseLeaf(const QRectF& bb) = 0;
        virtual NonLeafNode * chooseNode(const QRectF& bb, int level) = 0;

        virtual void intersects(const QRectF& rect, QMap<int, T> & result) const = 0;
        virtual void contains(const QPointF & point, QMap<int, T> & result) const = 0;

        virtual void keys(QList<QRectF> & result) const = 0;
        virtual void values(QMap<int, T> & result) const = 0;

        virtual Node * parent() const {
            return m_parent;
        }
        virtual void setParent(Node * parent) {
            m_parent = parent;
        }

        virtual int childCount() const {
            return m_counter;
        }

        virtual const QRectF& boundingBox() const {
            return m_boundingBox;
        }
        virtual void updateBoundingBox();

        virtual const QRectF& childBoundingBox(int index) const {
            return m_childBoundingBox[index];
        }
        virtual void setChildBoundingBox(int index, const QRectF& rect) {
            m_childBoundingBox[index] = rect;
        }

        virtual void clear();
        virtual bool isRoot() const {
            return m_parent == 0;
        }
        virtual bool isLeaf() const {
            return false;
        }

        virtual int place() const {
            return m_place;
        }
        virtual void setPlace(int place) {
            m_place = place;
        }

        virtual int level() const {
            return m_level;
        }
        virtual void setLevel(int level) {
            m_level = level;
        }

#ifdef KOFFICE_RTREE_DEBUG
        virtual int nodeId() const {
            return m_nodeId;
        }

        virtual void paint(QPainter & p, int level) const = 0;
        virtual void debug(QString line) const = 0;

    protected:
#define levelColorSize 5
        static QColor levelColor[levelColorSize];
        virtual void paintRect(QPainter & p, int level) const;
#endif
    protected:
        Node * m_parent;
        QRectF m_boundingBox;
        QVector<QRectF> m_childBoundingBox;
        int m_counter;
        // the position in the parent
        int m_place;
#ifdef KOFFICE_RTREE_DEBUG
        int m_nodeId;
#endif
        int m_level;
    };

class NonLeafNode : virtual public Node
    {
    public:
        NonLeafNode(int capacity, int level, Node * parent);
        virtual ~NonLeafNode();

        virtual void insert(const QRectF& bb, Node * data);
        virtual void remove(int index);
        virtual void move(Node * node, int index);

        virtual LeafNode * chooseLeaf(const QRectF& bb);
        virtual NonLeafNode * chooseNode(const QRectF& bb, int level);

        virtual void intersects(const QRectF& rect, QMap<int, T> & result) const;
        virtual void contains(const QPointF & point, QMap<int, T> & result) const;

        virtual void keys(QList<QRectF> & result) const;
        virtual void values(QMap<int, T> & result) const;

        virtual Node * getNode(int index) const;

#ifdef KOFFICE_RTREE_DEBUG
        virtual void paint(QPainter & p, int level) const;
        virtual void debug(QString line) const;
#endif
    protected:
        virtual Node * getLeastEnlargement(const QRectF& bb) const;

        QVector<Node *> m_childs;
    };

class LeafNode : virtual public Node
    {
    public:
        static int dataIdCounter;

        LeafNode(int capacity, int level, Node * parent);
        virtual ~LeafNode();

        virtual void insert(const QRectF& bb, const T& data, int id);
        virtual void remove(int index);
        virtual void remove(const T& data);
        virtual void move(Node * node, int index);

        virtual LeafNode * chooseLeaf(const QRectF& bb);
        virtual NonLeafNode * chooseNode(const QRectF& bb, int level);

        virtual void intersects(const QRectF& rect, QMap<int, T> & result) const;
        virtual void contains(const QPointF & point, QMap<int, T> & result) const;

        virtual void keys(QList<QRectF> & result) const;
        virtual void values(QMap<int, T> & result) const;

        virtual const T& getData(int index) const;
        virtual int getDataId(int index) const;

        virtual bool isLeaf() const {
            return true;
        }

#ifdef KOFFICE_RTREE_DEBUG
        virtual void debug(QString line) const;
        virtual void paint(QPainter & p, int level) const;
#endif
    protected:
        QVector<T> m_data;
        QVector<int> m_dataIds;
    };

    // factory methods
    virtual LeafNode* createLeafNode(int capacity, int level, Node * parent) {
        return new LeafNode(capacity, level, parent);
    }
    virtual NonLeafNode* createNonLeafNode(int capacity, int level, Node * parent) {
        return new NonLeafNode(capacity, level, parent);
    }

    // methods for insert
    QPair<Node *, Node *> splitNode(Node * node);
    QPair<int, int> pickSeeds(Node * node);
    QPair<int, int> pickNext(Node * node, QVector<bool> & marker, Node * group1, Node * group2);
    void adjustTree(Node * node1, Node * node2);
    void insertHelper(const QRectF& bb, const T& data, int id);

    // methods for delete
    void insert(Node * node);
    void condenseTree(Node * node, QVector<Node *> & reinsert);

    int m_capacity;
    int m_minimum;
    Node * m_root;
    QMap<T, LeafNode *> m_leafMap;
};

template <typename T>
KoRTree<T>::KoRTree(int capacity, int minimum)
        : m_capacity(capacity)
        , m_minimum(minimum)
        , m_root(createLeafNode(m_capacity + 1, 0, 0))
{
    if (minimum > capacity / 2)
        qFatal("KoRTree::KoRTree minimum can be maximal capacity/2");
    //qDebug() << "root node " << m_root->nodeId();
}

template <typename T>
KoRTree<T>::~KoRTree()
{
    delete m_root;
}

template <typename T>
void KoRTree<T>::insert(const QRectF& bb, const T& data)
{
    insertHelper(bb, data, LeafNode::dataIdCounter++);
}

template <typename T>
void KoRTree<T>::insertHelper(const QRectF& bb, const T& data, int id)
{
    QRectF nbb(bb.normalized());
    // This has to be done as it is not possible to use QRectF::unite() with a isNull()
    if (nbb.isNull()) {
        nbb.setWidth(0.0001);
        nbb.setHeight(0.0001);
        kWarning(30003) <<  "KoRTree::insert boundingBox isNull setting size to" << nbb.size();
    }
    else {
        // This has to be done as QRectF::intersects() return false if the rect does not have any area overlapping.
        // If there is no width or height there is no area and therefore no overlapping.
        if ( nbb.width() == 0 ) {
            nbb.setWidth(0.0001);
        }
        if ( nbb.height() == 0 ) {
            nbb.setHeight(0.0001);
        }
    }

    LeafNode * leaf = m_root->chooseLeaf(nbb);
    //qDebug() << " leaf" << leaf->nodeId() << nbb;

    if (leaf->childCount() < m_capacity) {
        leaf->insert(nbb, data, id);
        m_leafMap[data] = leaf;
        adjustTree(leaf, 0);
    } else {
        leaf->insert(nbb, data, id);
        m_leafMap[data] = leaf;
        QPair<Node *, Node *> newNodes = splitNode(leaf);
        LeafNode * l = dynamic_cast<LeafNode *>(newNodes.first);
        if (l)
            for (int i = 0; i < l->childCount(); ++i)
                m_leafMap[l->getData(i)] = l;
        l = dynamic_cast<LeafNode *>(newNodes.second);
        if (l)
            for (int i = 0; i < l->childCount(); ++i)
                m_leafMap[l->getData(i)] = l;

        adjustTree(newNodes.first, newNodes.second);
    }
}

template <typename T>
void KoRTree<T>::insert(Node * node)
{
    if (node->level() == m_root->level()) {
        adjustTree(m_root, node);
    } else {
        QRectF bb(node->boundingBox());
        NonLeafNode * newParent = m_root->chooseNode(bb, node->level() + 1);

        newParent->insert(bb, node);

        QPair<Node *, Node *> newNodes(node, 0);
        if (newParent->childCount() > m_capacity) {
            newNodes = splitNode(newParent);
        }
        adjustTree(newNodes.first, newNodes.second);
    }
}

template <typename T>
void KoRTree<T>::remove(const T&data)
{
    //qDebug() << "KoRTree remove";
    LeafNode * leaf = m_leafMap[data];
    if (leaf == 0) {
        kWarning(30003) << "KoRTree<T>::remove( const T&data) data not found";
        return;
    }
    m_leafMap.remove(data);
    leaf->remove(data);

    QVector<Node *> reinsert;
    condenseTree(leaf, reinsert);

    for (int i = 0; i < reinsert.size(); ++i) {
        if (reinsert[i]->isLeaf()) {
            LeafNode * leaf = dynamic_cast<LeafNode *>(reinsert[i]);
            for (int j = 0; j < leaf->childCount(); ++j) {
                insertHelper(leaf->childBoundingBox(j), leaf->getData(j), leaf->getDataId(j));
            }
            // clear is needed as the data items are not removed when insert into a new node
            leaf->clear();
            delete leaf;
        } else {
            NonLeafNode * node = dynamic_cast<NonLeafNode *>(reinsert[i]);
            for (int j = 0; j < node->childCount(); ++j) {
                insert(node->getNode(j));
            }
            // clear is needed as the data items are not removed when insert into a new node
            node->clear();
            delete node;
        }
    }
}

template <typename T>
QList<T> KoRTree<T>::intersects(const QRectF& rect) const
{
    QMap<int, T> found;
    m_root->intersects(rect, found);
    return found.values();
}

template <typename T>
QList<T> KoRTree<T>::contains(const QPointF &point) const
{
    QMap<int, T> found;
    m_root->contains(point, found);
    return found.values();
}

template <typename T>
QList<QRectF> KoRTree<T>::keys() const
{
    QList<QRectF> found;
    m_root->keys(found);
    return found;
}

template <typename T>
QList<T> KoRTree<T>::values() const
{
    QMap<int, T> found;
    m_root->values(found);
    return found.values();
}

#ifdef KOFFICE_RTREE_DEBUG
template <typename T>
void KoRTree<T>::paint(QPainter & p) const
{
    if (m_root) {
        m_root->paint(p, 0);
    }
}

template <typename T>
void KoRTree<T>::debug() const
{
    QString prefix("");
    m_root->debug(prefix);
}
#endif

template <typename T>
QPair< typename KoRTree<T>::Node*, typename KoRTree<T>::Node* > KoRTree<T>::splitNode(typename KoRTree<T>::Node* node)
{
    //qDebug() << "KoRTree::splitNode" << node;
    Node * n1;
    Node * n2;
    if (node->isLeaf()) {
        n1 = createLeafNode(m_capacity + 1, node->level(), node->parent());
        n2 = createLeafNode(m_capacity + 1, node->level(), node->parent());
    } else {
        n1 = createNonLeafNode(m_capacity + 1, node->level(), node->parent());
        n2 = createNonLeafNode(m_capacity + 1, node->level(), node->parent());
    }
    //qDebug() << " n1" << n1 << n1->nodeId();
    //qDebug() << " n2" << n2 << n2->nodeId();

    QVector<bool> marker(m_capacity + 1);

    QPair<int, int> seeds(pickSeeds(node));

    n1->move(node, seeds.first);
    n2->move(node, seeds.second);

    marker[seeds.first] = true;
    marker[seeds.second] = true;

    // There is one more in a node to split than the capacity and as we
    // already put the seeds in the new nodes subtract them.
    int remaining = m_capacity + 1 - 2;

    while (remaining > 0) {
        if (m_minimum - n1->childCount() == remaining) {
            for (int i = 0; i < m_capacity + 1; ++i) {
                if (!marker[i]) {
                    n1->move(node, i);
                    --remaining;
                }
            }
        } else if (m_minimum - n2->childCount() == remaining) {
            for (int i = 0; i < m_capacity + 1; ++i) {
                if (!marker[i]) {
                    n2->move(node, i);
                    --remaining;
                }
            }
        } else {
            QPair<int, int> next(pickNext(node, marker, n1, n2));

            if (next.first == 0) {
                n1->move(node, next.second);
            } else {
                n2->move(node, next.second);
            }
            --remaining;
        }
    }
    Q_ASSERT(n1->childCount() + n2->childCount() == node->childCount());

    // move the data back to the old node
    // this has to be done as the current node is already in the tree.
    node->clear();
    for (int i = 0; i < n1->childCount(); ++i) {
        node->move(n1, i);
    }

    //qDebug() << " delete n1" << n1 << n1->nodeId();
    // clear is needed as the data items are not removed
    n1->clear();
    delete n1;
    return qMakePair(node, n2);
}

template <typename T>
QPair<int, int> KoRTree<T>::pickSeeds(Node *node)
{
    int s1 = 0;
    int s2 = 1;
    qreal max = 0;
    for (int i = 0; i < m_capacity + 1; ++i) {
        for (int j = i+1; j < m_capacity + 1; ++j) {
            if (i != j) {
                QRectF bb1(node->childBoundingBox(i));
                QRectF bb2(node->childBoundingBox(j));
                QRectF comp(node->childBoundingBox(i).unite(node->childBoundingBox(j)));
                qreal area = comp.width() * comp.height() - bb1.width() * bb1.height() - bb2.width() * bb2.height();
                //qDebug() << " ps" << i << j << area;
                if (area > max) {
                    max = area;
                    s1 = i;
                    s2 = j;
                }
            }
        }
    }
    return qMakePair(s1, s2);
}

template <typename T>
QPair<int, int> KoRTree<T>::pickNext(Node * node, QVector<bool> & marker, Node * group1, Node * group2)
{
    //qDebug() << "KoRTree::pickNext" << marker;
    qreal max = -1.0;
    int select = 0;
    int group = 0;
    for (int i = 0; i < m_capacity + 1; ++i) {
        if (marker[i] == false) {
            QRectF bb1 = group1->boundingBox().unite(node->childBoundingBox(i));
            QRectF bb2 = group2->boundingBox().unite(node->childBoundingBox(i));
            qreal d1 = bb1.width() * bb1.height() - group1->boundingBox().width() * group1->boundingBox().height();
            qreal d2 = bb2.width() * bb2.height() - group2->boundingBox().width() * group2->boundingBox().height();
            qreal diff = qAbs(d1 - d2);
            //qDebug() << " diff" << diff << i << d1 << d2;
            if (diff > max) {
                max = diff;
                select = i;
                //qDebug() << "  i =" <<  i;
                if (qAbs(d1) > qAbs(d2)) {
                    group = 1;
                } else {
                    group = 0;
                }
                //qDebug() << "  group =" << group;
            }
        }
    }
    marker[select] = true;
    return qMakePair(group, select);
}

template <typename T>
void KoRTree<T>::adjustTree(Node *node1, Node *node2)
{
    //qDebug() << "KoRTree::adjustTree";
    if (node1->isRoot()) {
        //qDebug() << "  root";
        if (node2) {
            NonLeafNode * newRoot = createNonLeafNode(m_capacity + 1, node1->level() + 1, 0);
            newRoot->insert(node1->boundingBox(), node1);
            newRoot->insert(node2->boundingBox(), node2);
            m_root = newRoot;
            //qDebug() << "new root" << m_root->nodeId();
        }
    } else {
        NonLeafNode * parent = dynamic_cast<NonLeafNode *>(node1->parent());
        if (!parent) {
            qFatal("KoRTree::adjustTree: no parent node found!");
            return;
        }
        //QRectF pbbold( parent->boundingBox() );
        parent->setChildBoundingBox(node1->place(), node1->boundingBox());
        parent->updateBoundingBox();
        //qDebug() << "  bb1 =" << node1->boundingBox() << node1->place() << pbbold << "->" << parent->boundingBox() << parent->nodeId();

        if (!node2) {
            //qDebug() << "  update";
            adjustTree(parent, 0);
        } else {
            if (parent->childCount() < m_capacity) {
                //qDebug() << "  no split needed";
                parent->insert(node2->boundingBox(), node2);
                adjustTree(parent, 0);
            } else {
                //qDebug() << "  split again";
                parent->insert(node2->boundingBox(), node2);
                QPair<Node *, Node *> newNodes = splitNode(parent);
                adjustTree(newNodes.first, newNodes.second);
            }
        }
    }
}

template <typename T>
void KoRTree<T>::condenseTree(Node *node, QVector<Node*> & reinsert)
{
    //qDebug() << "KoRTree::condenseTree begin reinsert.size()" << reinsert.size();
    if (!node->isRoot()) {
        Node * parent = node->parent();
        //qDebug() << " !node->isRoot us" << node->childCount();

        if (node->childCount() < m_minimum) {
            //qDebug() << "  remove node";
            parent->remove(node->place());
            reinsert.push_back(node);
        } else {
            //qDebug() << "  update BB parent is root" << parent->isRoot();
            parent->setChildBoundingBox(node->place(), node->boundingBox());
            parent->updateBoundingBox();
        }
        condenseTree(parent, reinsert);
    } else {
        //qDebug() << " node->isRoot us" << node->childCount();
        if (node->childCount() == 1 && !node->isLeaf()) {
            //qDebug() << "  usedSpace = 1";
            NonLeafNode * n = dynamic_cast<NonLeafNode *>(node);
            if (n) {
                Node * kid = n->getNode(0);
                // clear is needed as the data items are not removed
                m_root->clear();
                delete m_root;
                m_root = kid;
                m_root->setParent(0);
                //qDebug() << " new root" << m_root;
            } else {
                qFatal("KoRTree::condenseTree cast to NonLeafNode failed");
            }
        }
    }
    //qDebug() << "KoRTree::condenseTree end reinsert.size()" << reinsert.size();
}

#ifdef KOFFICE_RTREE_DEBUG
template <typename T>
QColor KoRTree<T>::Node::levelColor[] = {
    QColor(Qt::green),
    QColor(Qt::red),
    QColor(Qt::cyan),
    QColor(Qt::magenta),
    QColor(Qt::yellow),
};

template <class T>
int KoRTree<T>::Node::nodeIdCnt = 0;
#endif

template <typename T>
KoRTree<T>::Node::Node(int capacity, int level, Node * parent)
        : m_parent(parent)
        , m_childBoundingBox(capacity)
        , m_counter(0)
#ifdef KOFFICE_RTREE_DEBUG
        , m_nodeId(nodeIdCnt++)
#endif
        , m_level(level)
{
}

template <typename T>
void KoRTree<T>::Node::remove(int index)
{
    for (int i = index + 1; i < m_counter; ++i) {
        m_childBoundingBox[i-1] = m_childBoundingBox[i];
    }
    --m_counter;
    updateBoundingBox();
}

template <typename T>
void KoRTree<T>::Node::updateBoundingBox()
{
    QRectF oldBB = m_boundingBox;
    m_boundingBox = QRectF();
    for (int i = 0; i < m_counter; ++i) {
        m_boundingBox = m_boundingBox.unite(m_childBoundingBox[i]);
    }
}

template <typename T>
void KoRTree<T>::Node::clear()
{
    m_counter = 0;
    m_boundingBox = QRectF();
}

#ifdef KOFFICE_RTREE_DEBUG
template <typename T>
void KoRTree<T>::Node::paintRect(QPainter & p, int level) const
{
    QColor c(Qt::black);
    if (level < levelColorSize) {
        c = levelColor[level];
    }

    QPen pen(c);
    p.setPen(pen);

    QRectF bbdraw(this->m_boundingBox);
    bbdraw.adjust(level * 2, level * 2, -level * 2, -level * 2);
    p.drawRect(bbdraw);
}
#endif

template <typename T>
KoRTree<T>::NonLeafNode::NonLeafNode(int capacity, int level, Node * parent)
        : Node(capacity, level, parent)
        , m_childs(capacity)
{
    //qDebug() << "NonLeafNode::NonLeafNode()" << this;
}

template <typename T>
KoRTree<T>::NonLeafNode::~NonLeafNode()
{
    //qDebug() << "NonLeafNode::~NonLeafNode()" << this;
    for (int i = 0; i < this->m_counter; ++i) {
        delete m_childs[i];
    }
}

template <typename T>
void KoRTree<T>::NonLeafNode::insert(const QRectF& bb, Node * data)
{
    m_childs[this->m_counter] = data;
    data->setPlace(this->m_counter);
    data->setParent(this);
    this->m_childBoundingBox[this->m_counter] = bb;
    this->m_boundingBox = this->m_boundingBox.unite(bb);
    //qDebug() << "NonLeafNode::insert" << this->nodeId() << data->nodeId();
    ++this->m_counter;
}

template <typename T>
void KoRTree<T>::NonLeafNode::remove(int index)
{
    for (int i = index + 1; i < this->m_counter; ++i) {
        m_childs[i-1] = m_childs[i];
        m_childs[i-1]->setPlace(i - 1);
    }
    Node::remove(index);
}

template <typename T>
void KoRTree<T>::NonLeafNode::move(Node * node, int index)
{
    //qDebug() << "NonLeafNode::move" << this << node << index << node->nodeId() << "->" << this->nodeId();
    NonLeafNode * n = dynamic_cast<NonLeafNode *>(node);
    if (n) {
        QRectF bb = n->childBoundingBox(index);
        insert(bb, n->getNode(index));
    }
}

template <typename T>
typename KoRTree<T>::LeafNode * KoRTree<T>::NonLeafNode::chooseLeaf(const QRectF& bb)
{
    return getLeastEnlargement(bb)->chooseLeaf(bb);
}

template <typename T>
typename KoRTree<T>::NonLeafNode * KoRTree<T>::NonLeafNode::chooseNode(const QRectF& bb, int level)
{
    if (this->m_level > level) {
        return getLeastEnlargement(bb)->chooseNode(bb, level);
    } else {
        return this;
    }

}

template <typename T>
void KoRTree<T>::NonLeafNode::intersects(const QRectF& rect, QMap<int, T> & result) const
{
    for (int i = 0; i < this->m_counter; ++i) {
        if (this->m_childBoundingBox[i].intersects(rect)) {
            m_childs[i]->intersects(rect, result);
        }
    }
}

template <typename T>
void KoRTree<T>::NonLeafNode::contains(const QPointF & point, QMap<int, T> & result) const
{
    for (int i = 0; i < this->m_counter; ++i) {
        if (this->m_childBoundingBox[i].contains(point)) {
            m_childs[i]->contains(point, result);
        }
    }
}

template <typename T>
void KoRTree<T>::NonLeafNode::keys(QList<QRectF> & result) const
{
    for (int i = 0; i < this->m_counter; ++i) {
        m_childs[i]->keys(result);
    }
}

template <typename T>
void KoRTree<T>::NonLeafNode::values(QMap<int, T> & result) const
{
    for (int i = 0; i < this->m_counter; ++i) {
        m_childs[i]->values(result);
    }
}

template <typename T>
typename KoRTree<T>::Node * KoRTree<T>::NonLeafNode::getNode(int index) const
{
    return m_childs[index];
}

template <typename T>
typename KoRTree<T>::Node * KoRTree<T>::NonLeafNode::getLeastEnlargement(const QRectF& bb) const
{
    //qDebug() << "NonLeafNode::getLeastEnlargement";
    QVarLengthArray<qreal> area(this->m_counter);
    for (int i = 0; i < this->m_counter; ++i) {
        QSizeF big(this->m_childBoundingBox[i].unite(bb).size());
        area[i] = big.width() * big.height() - this->m_childBoundingBox[i].width() * this->m_childBoundingBox[i].height();
    }

    int minIndex = 0;
    qreal minArea = area[minIndex];
    //qDebug() << " min" << minIndex << minArea;

    for (int i = 1; i < this->m_counter; ++i) {
        if (area[i] < minArea) {
            minIndex = i;
            minArea = area[i];
            //qDebug() << " min" << minIndex << minArea;
        }
    }

    return m_childs[minIndex];
}

#ifdef KOFFICE_RTREE_DEBUG
template <typename T>
void KoRTree<T>::NonLeafNode::debug(QString line) const
{
    for (int i = 0; i < this->m_counter; ++i) {
        qDebug("%s %d %d", qPrintable(line), this->nodeId(), i);
        m_childs[i]->debug(line + "       ");
    }
}

template <typename T>
void KoRTree<T>::NonLeafNode::paint(QPainter & p, int level) const
{
    this->paintRect(p, level);
    for (int i = 0; i < this->m_counter; ++i) {
        m_childs[i]->paint(p, level + 1);
    }

}
#endif

template <class T>
int KoRTree<T>::LeafNode::dataIdCounter = 0;

template <typename T>
KoRTree<T>::LeafNode::LeafNode(int capacity, int level, Node * parent)
        : Node(capacity, level, parent)
        , m_data(capacity)
        , m_dataIds(capacity)
{
    //qDebug() << "LeafNode::LeafNode" << this;
}

template <typename T>
KoRTree<T>::LeafNode::~LeafNode()
{
    //qDebug() << "LeafNode::~LeafNode" << this;
}

template <typename T>
void KoRTree<T>::LeafNode::insert(const QRectF& bb, const T& data, int id)
{
    m_data[this->m_counter] = data;
    m_dataIds[this->m_counter] = id;
    this->m_childBoundingBox[this->m_counter] = bb;
    this->m_boundingBox = this->m_boundingBox.unite(bb);
    ++this->m_counter;
}

template <typename T>
void KoRTree<T>::LeafNode::remove(int index)
{
    for (int i = index + 1; i < this->m_counter; ++i) {
        m_data[i-1] = m_data[i];
        m_dataIds[i-1] = m_dataIds[i];
    }
    Node::remove(index);
}

template <typename T>
void KoRTree<T>::LeafNode::remove(const T& data)
{
    int old_counter = this->m_counter;
    for (int i = 0; i < this->m_counter; ++i) {
        if (m_data[i] == data) {
            //qDebug() << "LeafNode::remove id" << i;
            remove(i);
            break;
        }
    }
    if (old_counter == this->m_counter) {
        kWarning(30003) << "LeafNode::remove( const T&data) data not found";
    }
}

template <typename T>
void KoRTree<T>::LeafNode::move(Node * node, int index)
{
    LeafNode * n = dynamic_cast<LeafNode*>(node);
    if (n) {
        //qDebug() << "LeafNode::move" << this << node << index
        //         << node->nodeId() << "->" << this->nodeId() << n->childBoundingBox( index );
        QRectF bb = n->childBoundingBox(index);
        insert(bb, n->getData(index), n->getDataId(index));
    }
}

template <typename T>
typename KoRTree<T>::LeafNode * KoRTree<T>::LeafNode::chooseLeaf(const QRectF& bb)
{
    Q_UNUSED(bb);
    return this;
}

template <typename T>
typename KoRTree<T>::NonLeafNode * KoRTree<T>::LeafNode::chooseNode(const QRectF& bb, int level)
{
    Q_UNUSED(bb);
    Q_UNUSED(level);
    qFatal("LeafNode::chooseNode called. This should not happen!");
    return 0;
}

template <typename T>
void KoRTree<T>::LeafNode::intersects(const QRectF& rect, QMap<int, T> & result) const
{
    for (int i = 0; i < this->m_counter; ++i) {
        if (this->m_childBoundingBox[i].intersects(rect)) {
            result.insert(m_dataIds[i], m_data[i]);
        }
    }
}

template <typename T>
void KoRTree<T>::LeafNode::contains(const QPointF & point, QMap<int, T> & result) const
{
    for (int i = 0; i < this->m_counter; ++i) {
        if (this->m_childBoundingBox[i].contains(point)) {
            result.insert(m_dataIds[i], m_data[i]);
        }
    }
}

template <typename T>
void KoRTree<T>::LeafNode::keys(QList<QRectF> & result) const
{
    for (int i = 0; i < this->m_counter; ++i) {
        result.push_back(this->m_childBoundingBox[i]);
    }
}

template <typename T>
void KoRTree<T>::LeafNode::values(QMap<int, T> & result) const
{
    for (int i = 0; i < this->m_counter; ++i) {
        result.insert(m_dataIds[i], m_data[i]);
    }
}

template <typename T>
const T& KoRTree<T>::LeafNode::getData(int index) const
{
    return m_data[ index ];
}

template <typename T>
int KoRTree<T>::LeafNode::getDataId(int index) const
{
    return m_dataIds[ index ];
}

#ifdef KOFFICE_RTREE_DEBUG
template <typename T>
void KoRTree<T>::LeafNode::debug(QString line) const
{
    for (int i = 0; i < this->m_counter; ++i) {
        qDebug("%s %d %d %p", qPrintable(line), this->nodeId(), i, &(m_data[i]));
        qDebug() << this->m_childBoundingBox[i].toRect();
    }
}

template <typename T>
void KoRTree<T>::LeafNode::paint(QPainter & p, int level) const
{
    if (this->m_counter) {
        this->paintRect(p, level);
    }
}
#endif

#endif /* KORTREE_H */
