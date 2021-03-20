/*
 *  SPDX-FileCopyrightText: 2020 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISBEZIERMESH_H
#define KISBEZIERMESH_H

#include <kritaglobal_export.h>

#include <QDebug>

#include <KisBezierUtils.h>
#include <KisBezierPatch.h>

#include <boost/iterator/iterator_facade.hpp>
#include <boost/operators.hpp>

#include <functional>
#include "KisCppQuirks.h"

#include "kis_debug.h"

class QDomElement;

namespace KisBezierMeshDetails {

struct BaseMeshNode : public boost::equality_comparable<BaseMeshNode> {
    BaseMeshNode() {}
    BaseMeshNode(const QPointF &_node)
        : leftControl(_node),
          topControl(_node),
          node(_node),
          rightControl(_node),
          bottomControl(_node)
    {
    }

    bool operator==(const BaseMeshNode &rhs) const {
        return leftControl == rhs.leftControl &&
                topControl == rhs.topControl &&
                node == rhs.node &&
                rightControl == rhs.rightControl &&
                bottomControl == rhs.bottomControl;
    }

    void setLeftControlRelative(const QPointF &value) {
        leftControl = value + node;
    }

    QPointF leftControlRelative() const {
        return leftControl - node;
    }

    void setRightControlRelative(const QPointF &value) {
        rightControl = value + node;
    }

    QPointF rightControlRelative() const {
        return rightControl - node;
    }

    void setTopControlRelative(const QPointF &value) {
        topControl = value + node;
    }

    QPointF topControlRelative() const {
        return topControl - node;
    }

    void setBottomControlRelative(const QPointF &value) {
        bottomControl = value + node;
    }

    QPointF bottomControlRelative() const {
        return bottomControl - node;
    }

    void translate(const QPointF &offset) {
        leftControl += offset;
        topControl += offset;
        node += offset;
        rightControl += offset;
        bottomControl += offset;
    }

    void transform(const QTransform &t) {
        leftControl = t.map(leftControl);
        topControl = t.map(topControl);
        node = t.map(node);
        rightControl = t.map(rightControl);
        bottomControl = t.map(bottomControl);
    }

    QPointF leftControl;
    QPointF topControl;
    QPointF node;
    QPointF rightControl;
    QPointF bottomControl;
};

inline void lerpNodeData(const BaseMeshNode &left, const BaseMeshNode &right, qreal t, BaseMeshNode &dst)
{
    Q_UNUSED(left);
    Q_UNUSED(right);
    Q_UNUSED(t);
    Q_UNUSED(dst);
}

inline void assignPatchData(KisBezierPatch *patch,
                            const QRectF &srcRect,
                            const BaseMeshNode &tl,
                            const BaseMeshNode &tr,
                            const BaseMeshNode &bl,
                            const BaseMeshNode &br)
{
    patch->originalRect = srcRect;
    Q_UNUSED(tl);
    Q_UNUSED(tr);
    Q_UNUSED(bl);
    Q_UNUSED(br);
}

template<typename NodeArg = BaseMeshNode,
         typename PatchArg = KisBezierPatch>
class Mesh : public boost::equality_comparable<Mesh<NodeArg, PatchArg>>
{
public:
    using Node = NodeArg;
    using Patch = PatchArg;

    struct PatchIndex : public QPoint, boost::additive<PatchIndex, QPoint>
    {
        using QPoint::QPoint;
        PatchIndex& operator+=(const QPoint &rhs) {
            QPoint::operator+=(rhs);
            return *this;
        }
        PatchIndex& operator-=(const QPoint &rhs) {
            QPoint::operator-=(rhs);
            return *this;
        }
    };

    struct NodeIndex : public QPoint, boost::additive<NodeIndex, QPoint>
    {
        using QPoint::QPoint;
        NodeIndex& operator+=(const QPoint &rhs) {
            QPoint::operator+=(rhs);
            return *this;
        }
        NodeIndex& operator-=(const QPoint &rhs) {
            QPoint::operator-=(rhs);
            return *this;
        }
    };

    using SegmentIndex = std::pair<NodeIndex, int>;

    struct ControlPointIndex : public boost::equality_comparable<ControlPointIndex>
    {
        enum ControlType {
            LeftControl = 0,
            TopControl,
            RightControl,
            BottomControl,
            Node
        };

        ControlPointIndex()  = default;
        ControlPointIndex(const ControlPointIndex &rhs) = default;
        ControlPointIndex(NodeIndex _nodeIndex, ControlType _controlType)
            : nodeIndex(_nodeIndex),
              controlType(_controlType)
        {
        }

        NodeIndex nodeIndex;
        ControlType controlType;

        inline bool isNode() const {
            return controlType == Node;
        }

        inline bool isControlPoint() const {
            return controlType != Node;
        }

        template <class NodeType,
                  class PointType = std::add_const_if_t<std::is_const<NodeType>::value, QPointF>>
        static
        PointType& controlPoint(NodeType &node, ControlType controlType) {
            return
                controlType == LeftControl ? node.leftControl :
                controlType == RightControl ? node.rightControl :
                controlType == TopControl ? node.topControl :
                controlType == BottomControl ? node.bottomControl :
                node.node;
        }

        QPointF& controlPoint(Mesh::Node &node) {
            return controlPoint(node, controlType);
        }

        friend bool operator==(const ControlPointIndex &lhs, const ControlPointIndex &rhs) {
            return lhs.nodeIndex == rhs.nodeIndex && lhs.controlType == rhs.controlType;
        }

        friend QDebug operator<<(QDebug dbg, const Mesh::ControlPointIndex &index) {
            dbg.nospace() << "ControlPointIndex ("
                          << index.nodeIndex.x() << ", " << index.nodeIndex.x() << ", ";

            switch (index.controlType) {
            case Mesh::ControlType::Node:
                dbg.nospace() << "Node";
                break;
            case Mesh::ControlType::LeftControl:
                dbg.nospace() << "LeftControl";
                break;
            case Mesh::ControlType::RightControl:
                dbg.nospace() << "RightControl";
                break;
            case Mesh::ControlType::TopControl:
                dbg.nospace() << "TopControl";
                break;
            case Mesh::ControlType::BottomControl:
                dbg.nospace() << "BottomControl";
                break;
            }

            dbg.nospace() << ")";
            return dbg.space();
        }
    };

    using ControlType = typename ControlPointIndex::ControlType;

private:
    template<bool is_const>
    class segment_iterator_impl;

    template<bool is_const>
    class control_point_iterator_impl;

    template<bool is_const>
    class patch_iterator_impl :
        public boost::iterator_facade <patch_iterator_impl<is_const>,
                                       Patch,
                                       boost::random_access_traversal_tag,
                                       Patch>
    {
        using PointType = std::add_const_if_t<is_const, QPointF>;
        using MeshType = std::add_const_if_t<is_const, Mesh>;
        using SegmentIteratorType = segment_iterator_impl<is_const>;
        using ControlPointIteratorType = control_point_iterator_impl<is_const>;

    public:
        patch_iterator_impl()
            : m_mesh(0),
              m_col(0),
              m_row(0) {}

        patch_iterator_impl(MeshType* mesh, int col, int row)
            : m_mesh(mesh),
              m_col(col),
              m_row(row)
        {
        }

        Mesh::PatchIndex patchIndex() const {
            return {m_col, m_row};
        }

        SegmentIteratorType segmentP() const;
        SegmentIteratorType segmentQ() const;
        SegmentIteratorType segmentR() const;
        SegmentIteratorType segmentS() const;

        ControlPointIteratorType nodeTopLeft() const;
        ControlPointIteratorType nodeTopRight() const;
        ControlPointIteratorType nodeBottomLeft() const;
        ControlPointIteratorType nodeBottomRight() const;

        bool isValid() const {
            return
                m_col >= 0 &&
                m_col < m_mesh->size().width() - 1 &&
                m_row >= 0 &&
                m_row < m_mesh->size().height() - 1;
        }

    private:
        friend class boost::iterator_core_access;

        void increment() {
            m_col++;
            if (m_col >= m_mesh->m_size.width() - 1) {
                m_col = 0;
                m_row++;
            }
        }

        void decrement() {
            m_col--;
            if (m_col < 0) {
                m_col = m_mesh->m_size.width() - 2;
                m_row--;
            }
        }

        void advance(int n) {
            const int index = m_row * (m_mesh->m_size.width() - 1) + m_col + n;

            m_row = index / (m_mesh->m_size.width() - 1);
            m_col = index % (m_mesh->m_size.width() - 1);

            KIS_SAFE_ASSERT_RECOVER_NOOP(m_row < m_mesh->m_size.height() - 1);
        }

        int distance_to(const patch_iterator_impl &z) const {
            const int index = m_row * (m_mesh->m_size.width() - 1) + m_col;
            const int otherIndex = z.m_row * (m_mesh->m_size.width() - 1) + z.m_col;

            return otherIndex - index;
        }

        bool equal(patch_iterator_impl const& other) const {
            return m_row == other.m_row &&
                    m_col == other.m_col &&
                m_mesh == other.m_mesh;
        }

        Patch dereference() const {
            return m_mesh->makePatch(m_col, m_row);
        }

    private:

        MeshType *m_mesh;
        int m_col;
        int m_row;
    };

    template<bool is_const>
    class control_point_iterator_impl :
        public boost::iterator_facade <control_point_iterator_impl<is_const>,
                                       std::add_const_if_t<is_const, QPointF>,
                                       boost::bidirectional_traversal_tag>
    {
        using PointType = std::add_const_if_t<is_const, QPointF>;
        using NodeType = std::add_const_if_t<is_const, Node>;
        using MeshType = std::add_const_if_t<is_const, Mesh>;
        using SegmentIteratorType = segment_iterator_impl<is_const>;

    public:
        control_point_iterator_impl()
            : m_mesh(0),
              m_col(0),
              m_row(0),
              m_controlIndex(0)
        {}

        control_point_iterator_impl(MeshType* mesh, int col, int row, int controlIndex)
            : m_mesh(mesh),
              m_col(col),
              m_row(row),
              m_controlIndex(controlIndex)
        {
        }

        Mesh::ControlType type() const {
            return Mesh::ControlType(m_controlIndex);
        }

        int col() const {
            return m_col;
        }

        int row() const {
            return m_row;
        }

        Mesh::ControlPointIndex controlIndex() const {
            return Mesh::ControlPointIndex(nodeIndex(), type());
        }

        Mesh::NodeIndex nodeIndex() const {
            return Mesh::NodeIndex(m_col, m_row);
        }

        NodeType& node() const {
            return m_mesh->node(m_col, m_row);
        }

        bool isLeftBorder() const {
            return m_col == 0;
        }

        bool isRightBorder() const {
            return m_col == m_mesh->size().width() - 1;
        }

        bool isTopBorder() const {
            return m_row == 0;
        }

        bool isBottomBorder() const {
            return m_row == m_mesh->size().height() - 1;
        }


        bool isBorderNode() const {
            return isLeftBorder() || isRightBorder() || isTopBorder() || isBottomBorder();
        }

        bool isCornerNode() const {
            return (isLeftBorder() + isRightBorder() + isTopBorder() + isBottomBorder()) > 1;
        }

        bool isNode() const {
            return type() == Mesh::ControlPointIndex::Node;
        }

        control_point_iterator_impl symmetricControl() const {
            typename Mesh::ControlPointIndex::ControlType newIndex =
                    Mesh::ControlPointIndex::Node;

            switch (type()) {
            case Mesh::ControlPointIndex::Node:
                newIndex = Mesh::ControlPointIndex::Node;
                break;
            case Mesh::ControlPointIndex::LeftControl:
                newIndex = Mesh::ControlPointIndex::RightControl;
                break;
            case Mesh::ControlPointIndex::RightControl:
                newIndex = Mesh::ControlPointIndex::LeftControl;
                break;
            case Mesh::ControlPointIndex::TopControl:
                newIndex = Mesh::ControlPointIndex::BottomControl;
                break;
            case Mesh::ControlPointIndex::BottomControl:
                newIndex = Mesh::ControlPointIndex::TopControl;
                break;
            }

            control_point_iterator_impl it(m_mesh, m_col, m_row, newIndex);

            if (!it.controlIsValid()) {
                it = m_mesh->endControlPoints();
            }

            return it;
        }

        SegmentIteratorType topSegment() const;
        SegmentIteratorType bottomSegment() const;
        SegmentIteratorType leftSegment() const;
        SegmentIteratorType rightSegment() const;

        bool isValid() const {
            return nodeIsValid() && controlIsValid();
        }

    private:
        friend class boost::iterator_core_access;

        bool nodeIsValid() const {
            return m_col >= 0 && m_row >= 0 && m_col < m_mesh->size().width() && m_row < m_mesh->size().height();
        }

        bool controlIsValid() const {
            if (m_col == 0 && m_controlIndex == Mesh::ControlType::LeftControl) {
                return false;
            }

            if (m_col == m_mesh->m_size.width() - 1 && m_controlIndex == Mesh::ControlType::RightControl) {
                return false;
            }

            if (m_row == 0 && m_controlIndex == Mesh::ControlType::TopControl) {
                return false;
            }

            if (m_row == m_mesh->m_size.height() - 1 && m_controlIndex == Mesh::ControlType::BottomControl) {
                return false;
            }

            return true;
        }

        void increment() {
            do {
                m_controlIndex++;
                if (m_controlIndex > 4) {
                    m_controlIndex = 0;
                    m_col++;
                    if (m_col >= m_mesh->m_size.width()) {
                        m_col = 0;
                        m_row++;
                    }
                }
            } while (nodeIsValid() && !controlIsValid());
        }

        void decrement() {
            do {
                m_controlIndex--;
                if (m_controlIndex < 0) {
                    m_controlIndex = 4;
                    m_col--;
                    if (m_col < 0) {
                        m_col = m_mesh->m_size.width() - 1;
                        m_row--;
                    }
                }
            } while (nodeIsValid() && !controlIsValid());
        }


        bool equal(control_point_iterator_impl const& other) const {
            return m_controlIndex == other.m_controlIndex &&
                m_row == other.m_row &&
                m_col == other.m_col &&
                m_mesh == other.m_mesh;
        }

        PointType& dereference() const {
            return Mesh::ControlPointIndex::controlPoint(m_mesh->node(m_col, m_row), Mesh::ControlType(m_controlIndex));
        }

    private:
        MeshType* m_mesh;
        int m_col;
        int m_row;
        int m_controlIndex;
    };

    template<bool is_const>
    class segment_iterator_impl :
        public boost::iterator_facade <segment_iterator_impl<is_const>,
                                       Mesh::SegmentIndex,
                                       boost::bidirectional_traversal_tag,
                                       Mesh::SegmentIndex>
    {
        using PointType = std::add_const_if_t<is_const, QPointF>;
        using NodeType = std::add_const_if_t<is_const, Node>;
        using MeshType = std::add_const_if_t<is_const, Mesh>;
        using ControlPointIteratorType = control_point_iterator_impl<is_const>;

    public:
        segment_iterator_impl()
            : m_mesh(0),
              m_col(0),
              m_row(0),
              m_isHorizontal(0)
        {}

        segment_iterator_impl(MeshType* mesh, int col, int row, int isHorizontal)
            : m_mesh(mesh),
              m_col(col),
              m_row(row),
              m_isHorizontal(isHorizontal)
        {
        }

        Mesh::SegmentIndex segmentIndex() const {
            return
                 { Mesh::NodeIndex(m_col, m_row),
                   m_isHorizontal};
        }

        Mesh::NodeIndex firstNodeIndex() const {
            return Mesh::NodeIndex(m_col, m_row);
        }

        Mesh::NodeIndex secondNodeIndex() const {
            return m_isHorizontal ? Mesh::NodeIndex(m_col + 1, m_row) : Mesh::NodeIndex(m_col, m_row + 1);
        }

        NodeType& firstNode() const {
            return m_mesh->node(firstNodeIndex());
        }

        NodeType& secondNode() const {
            return m_mesh->node(secondNodeIndex());
        }

        PointType& p0() const {
            return firstNode().node;
        }

        PointType& p1() const {
            return m_isHorizontal ? firstNode().rightControl : firstNode().bottomControl;
        }

        PointType& p2() const {
            return m_isHorizontal ? secondNode().leftControl : secondNode().topControl;
        }

        PointType& p3() const {
            return secondNode().node;
        }

        ControlPointIteratorType itP0() const {
            return m_mesh->find(ControlPointIndex(firstNodeIndex(), Mesh::ControlType::Node));
        }

        ControlPointIteratorType itP1() const {
            return m_mesh->find(ControlPointIndex(firstNodeIndex(),
                                                  m_isHorizontal ?
                                                      Mesh::ControlType::RightControl :
                                                      Mesh::ControlType::BottomControl));
        }

        ControlPointIteratorType itP2() const {
            return m_mesh->find(ControlPointIndex(secondNodeIndex(),
                                                  m_isHorizontal ?
                                                      Mesh::ControlType::LeftControl :
                                                      Mesh::ControlType::TopControl));
        }

        ControlPointIteratorType itP3() const {
            return m_mesh->find(ControlPointIndex(secondNodeIndex(), Mesh::ControlType::Node));
        }

        QPointF pointAtParam(qreal t) const {
            return KisBezierUtils::bezierCurve(p0(), p1(), p2(), p3(), t);
        }

        qreal length() const {
            const qreal eps = 1e-3;
            return KisBezierUtils::curveLength(p0(), p1(), p2(), p3(), eps);
        }

        int degree() const {
            return KisBezierUtils::bezierDegree(p0(), p1(), p2(), p3());
        }

        bool isHorizontal() const {
            return m_isHorizontal;
        }

        bool isValid() const {
            return nodeIsValid() && controlIsValid();
        }

    private:
        friend class boost::iterator_core_access;

        bool nodeIsValid() const {
            return m_col >= 0 && m_row >= 0 && m_col < m_mesh->size().width() && m_row < m_mesh->size().height();
        }

        bool controlIsValid() const {
            if (m_col == m_mesh->m_size.width() - 1 && m_isHorizontal) {
                return false;
            }

            if (m_row == m_mesh->m_size.height() - 1 && !m_isHorizontal) {
                return false;
            }

            return true;
        }

        void increment() {
            do {
                m_isHorizontal++;
                if (m_isHorizontal > 1) {
                    m_isHorizontal = 0;
                    m_col++;
                    if (m_col >= m_mesh->m_size.width()) {
                        m_col = 0;
                        m_row++;
                    }
                }
            } while (nodeIsValid() && !controlIsValid());
        }

        void decrement() {
            do {
                m_isHorizontal--;
                if (m_isHorizontal < 0) {
                    m_isHorizontal = 1;
                    m_col--;
                    if (m_col < 0) {
                        m_col = m_mesh->m_size.width() - 1;
                        m_row--;
                    }
                }
            } while (nodeIsValid() && !controlIsValid());
        }


        bool equal(segment_iterator_impl const& other) const {
            return m_isHorizontal == other.m_isHorizontal &&
                m_row == other.m_row &&
                m_col == other.m_col &&
                m_mesh == other.m_mesh;
        }

        Mesh::SegmentIndex dereference() const {
            return segmentIndex();
        }

    private:

        MeshType* m_mesh;
        int m_col;
        int m_row;
        int m_isHorizontal;
    };

public:
    Mesh()
        : Mesh(QRectF(0.0, 0.0, 1.0, 1.0))
    {
    }

    Mesh(const QRectF &mapRect, const QSize &size = QSize(2,2))
        : m_size(size),
          m_originalRect(mapRect)
    {
        const qreal xControlOffset = 0.2 * (mapRect.width() / size.width());
        const qreal yControlOffset = 0.2 * (mapRect.height() / size.height());

        for (int row = 0; row < m_size.height(); row++) {
            const qreal yPos = qreal(row) / (size.height() - 1) * mapRect.height() + mapRect.y();

            for (int col = 0; col < m_size.width(); col++) {
                const qreal xPos = qreal(col) / (size.width() - 1) * mapRect.width() + mapRect.x();

                Node node(QPointF(xPos, yPos));
                node.setLeftControlRelative(QPointF(-xControlOffset, 0));
                node.setRightControlRelative(QPointF(xControlOffset, 0));
                node.setTopControlRelative(QPointF(0, -yControlOffset));
                node.setBottomControlRelative(QPointF(0, yControlOffset));

                m_nodes.push_back(node);
            }
        }

        for (int col = 0; col < m_size.width(); col++) {
            m_columns.push_back(qreal(col) / (size.width() - 1));
        }

        for (int row = 0; row < m_size.height(); row++) {
            m_rows.push_back(qreal(row) / (size.height() - 1));
        }
    }

    bool operator==(const Mesh &rhs) const {
        return m_size == rhs.m_size &&
                m_rows == rhs.m_rows &&
                m_columns == rhs.m_columns &&
                m_originalRect == rhs.m_originalRect &&
                m_nodes == rhs.m_nodes;
    }

    Node& node(int col, int row) {
        KIS_ASSERT(col >= 0 && col < m_size.width() && row >= 0 && row < m_size.height());
        return m_nodes[row * m_size.width() + col];
    }

    const Node& node(int col, int row) const {
        KIS_ASSERT(col >= 0 && col < m_size.width() && row >= 0 && row < m_size.height());
        return m_nodes[row * m_size.width() + col];
    }

    Node& node(const NodeIndex &index) {
        return node(index.x(), index.y());
    }

    const Node& node(const NodeIndex &index) const {
        return node(index.x(), index.y());
    }


    int subdivideRow(qreal proportionalT) {
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(proportionalT >= 0.0 && proportionalT <= 1.0, -1);

        {
            auto existing = std::find(m_rows.begin(), m_rows.end(), proportionalT);
            if (existing != m_rows.end()) {
                return distance(m_rows.begin(), existing);
            }
        }

        const auto it = prev(upper_bound(m_rows.begin(), m_rows.end(), proportionalT));
        const int topRow = distance(m_rows.begin(), it);
        const qreal relT = (proportionalT - *it) / (*next(it) - *it);

        return subdivideRow(topRow, relT);
    }

    int subdivideRow(int topRow, qreal relProportionalT) {
        const auto it = m_rows.begin() + topRow;
        const int bottomRow = topRow + 1;
        const qreal absProportionalT = KisAlgebra2D::lerp(*it, *next(it), relProportionalT);

        std::vector<Node> newRow;
        newRow.resize(m_size.width());
        for (int col = 0; col < m_size.width(); col++) {
            const qreal t =
                KisBezierUtils::curveParamByProportion(node(col, topRow).node,
                                                       node(col, topRow).bottomControl,
                                                       node(col, bottomRow).topControl,
                                                       node(col, bottomRow).node,
                                                       relProportionalT,
                                                       0.01);

            splitCurveVertically(node(col, topRow), node(col, bottomRow), t, newRow[col]);
        }

        m_nodes.insert(m_nodes.begin() + bottomRow * m_size.width(),
                       newRow.begin(), newRow.end());

        m_size.rheight()++;
        auto insertedIt = m_rows.insert(next(it), absProportionalT);
        return distance(m_rows.begin(), insertedIt);
    }

    int subdivideColumn(qreal proportionalT) {
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(proportionalT >= 0.0 && proportionalT <= 1.0, -1);

        {
            auto existing = std::find(m_columns.begin(), m_columns.end(), proportionalT);
            if (existing != m_columns.end()) {
                return distance(m_columns.begin(), existing);
            }
        }

        const auto it = prev(upper_bound(m_columns.begin(), m_columns.end(), proportionalT));
        const int leftColumn = distance(m_columns.begin(), it);

        const qreal relT = (proportionalT - *it) / (*next(it) - *it);

        return subdivideColumn(leftColumn, relT);
    }

    int subdivideColumn(int leftColumn, qreal relProportionalT) {
        const auto it = m_columns.begin() + leftColumn;
        const int rightColumn = leftColumn + 1;
        const qreal absProportinalT = KisAlgebra2D::lerp(*it, *next(it), relProportionalT);

        std::vector<Node> newColumn;
        newColumn.resize(m_size.height());
        for (int row = 0; row < m_size.height(); row++) {
            const qreal t =
                KisBezierUtils::curveParamByProportion(node(leftColumn, row).node,
                                                       node(leftColumn, row).rightControl,
                                                       node(rightColumn, row).leftControl,
                                                       node(rightColumn, row).node,
                                                       relProportionalT,
                                                       0.01);

            splitCurveHorizontally(node(leftColumn, row), node(rightColumn, row), t, newColumn[row]);
        }

        auto dstIt = m_nodes.begin() + rightColumn;
        for (auto columnIt = newColumn.begin(); columnIt != newColumn.end(); ++columnIt) {
            dstIt = m_nodes.insert(dstIt, *columnIt);
            dstIt += m_size.width() + 1;
        }

        m_size.rwidth()++;
        auto insertedIt = m_columns.insert(next(it), absProportinalT);
        return distance(m_columns.begin(), insertedIt);
    }

    void removeColumn(int column) {
        const bool hasNeighbours = column > 0 || column < m_size.width() - 1;

        if (hasNeighbours) {
            for (int row = 0; row < m_size.height(); row++) {
                unlinkNodeHorizontally(node(column - 1, row), node(column, row), node(column + 1, row));
            }
        }

        auto it = m_nodes.begin() + column;
        for (int row = 0; row < m_size.height(); row++) {
            it = m_nodes.erase(it);
            it += m_size.width() - 1;
        }

        m_size.rwidth()--;
        m_columns.erase(m_columns.begin() + column);
    }

    void removeRow(int row) {
        const bool hasNeighbours = row > 0 || row < m_size.height() - 1;

        if (hasNeighbours) {
            for (int column = 0; column < m_size.width(); column++) {
                unlinkNodeVertically(node(column, row - 1), node(column, row), node(column, row + 1));
            }
        }

        auto it = m_nodes.begin() + row * m_size.width();
        m_nodes.erase(it, it + m_size.width());

        m_size.rheight()--;
        m_rows.erase(m_rows.begin() + row);
    }

    void removeColumnOrRow(NodeIndex index, bool row) {
        if (row) {
            removeRow(index.y());
        } else {
            removeColumn(index.x());
        }
    }

    void subdivideSegment(SegmentIndex index, qreal proportionalT) {
        auto it = find(index);
        KIS_SAFE_ASSERT_RECOVER_RETURN(it != endSegments());

        if (it.isHorizontal()) {
            subdivideColumn(it.firstNodeIndex().x(), proportionalT);
        } else {
            subdivideRow(it.firstNodeIndex().y(), proportionalT);
        }
    }

    Patch makePatch(const PatchIndex &index) const {
        return makePatch(index.x(), index.y());
    }

    Patch makePatch(int col, int row) const
    {
        const Node &tl = node(col, row);
        const Node &tr = node(col + 1, row);
        const Node &bl = node(col, row + 1);
        const Node &br = node(col + 1, row + 1);

        Patch patch;

        patch.points[Patch::TL] = tl.node;
        patch.points[Patch::TL_HC] = tl.rightControl;
        patch.points[Patch::TL_VC] = tl.bottomControl;

        patch.points[Patch::TR] = tr.node;
        patch.points[Patch::TR_HC] = tr.leftControl;
        patch.points[Patch::TR_VC] = tr.bottomControl;

        patch.points[Patch::BL] = bl.node;
        patch.points[Patch::BL_HC] = bl.rightControl;
        patch.points[Patch::BL_VC] = bl.topControl;

        patch.points[Patch::BR] = br.node;
        patch.points[Patch::BR_HC] = br.leftControl;
        patch.points[Patch::BR_VC] = br.topControl;

        const QRectF relRect(m_columns[col],
                             m_rows[row],
                             m_columns[col + 1] - m_columns[col],
                             m_rows[row + 1] - m_rows[row]);

        assignPatchData(&patch, KisAlgebra2D::relativeToAbsolute(relRect, m_originalRect),
                        tl, tr, bl, br);

        return patch;
    }

    using patch_iterator = patch_iterator_impl<false>;
    using patch_const_iterator = patch_iterator_impl<true>;

    using control_point_iterator = control_point_iterator_impl<false>;
    using control_point_const_iterator = control_point_iterator_impl<true>;

    using segment_iterator = segment_iterator_impl<false>;
    using segment_const_iterator = segment_iterator_impl<true>;

    patch_iterator beginPatches() { return beginPatches(*this); }
    patch_const_iterator beginPatches() const { return beginPatches(*this); }
    patch_const_iterator constBeginPatches() const { return beginPatches(*this); }

    patch_iterator endPatches() { return endPatches(*this); }
    patch_const_iterator endPatches() const { return endPatches(*this); }
    patch_const_iterator constEndPatches() const { return endPatches(*this); }

    control_point_iterator beginControlPoints() { return beginControlPoints(*this); }
    control_point_const_iterator beginControlPoints() const { return beginControlPoints(*this); }
    control_point_const_iterator constBeginControlPoints() const { return beginControlPoints(*this); }

    control_point_iterator endControlPoints() { return endControlPoints(*this); }
    control_point_const_iterator endControlPoints() const { return endControlPoints(*this); }
    control_point_const_iterator constEndControlPoints() const { return endControlPoints(*this); }

    segment_iterator beginSegments() { return beginSegments(*this); }
    segment_const_iterator beginSegments() const { return beginSegments(*this); }
    segment_const_iterator constBeginSegments() const { return beginSegments(*this); }

    segment_iterator endSegments() { return endSegments(*this); }
    segment_const_iterator endSegments() const {  return endSegments(*this); }
    segment_const_iterator constEndSegments() const { return endSegments(*this); }

    control_point_iterator find(const ControlPointIndex &index) { return find(*this, index); }
    control_point_const_iterator find(const ControlPointIndex &index) const { return find(*this, index); }
    control_point_const_iterator constFind(const ControlPointIndex &index) const { return find(*this, index); }

    control_point_iterator find(const NodeIndex &index) { return find(*this, index); }
    control_point_const_iterator find(const NodeIndex &index) const { return find(*this, index); }
    control_point_const_iterator constFind(const NodeIndex &index) const { return find(*this, index); }

    segment_iterator find(const SegmentIndex &index) { return find(*this, index); }
    segment_const_iterator find(const SegmentIndex &index) const { return find(*this, index); }
    segment_const_iterator constFind(const SegmentIndex &index) const { return find(*this, index); }

    patch_iterator find(const PatchIndex &index) { return find(*this, index); }
    patch_const_iterator find(const PatchIndex &index) const { return find(*this, index); }
    patch_const_iterator constFind(const PatchIndex &index) const { return find(*this, index); }

    QSize size() const {
        return m_size;
    }

    QRectF originalRect() const {
        return m_originalRect;
    }

    QRectF dstBoundingRect() const {
        QRectF result;
        for (auto it = beginPatches(); it != endPatches(); ++it) {
            result |= it->dstBoundingRect();
        }
        return result;
    }

    bool isIdentity() const {
        Mesh identityMesh(m_originalRect, m_size);
        return *this == identityMesh;
    }

    void translate(const QPointF &offset) {
        for (auto it = m_nodes.begin(); it != m_nodes.end(); ++it) {
            it->translate(offset);
        }
    }

    void transform(const QTransform &t) {
        for (auto it = m_nodes.begin(); it != m_nodes.end(); ++it) {
            it->transform(t);
        }
    }

    void transformSrcAndDst(const QTransform &t) {
        KIS_SAFE_ASSERT_RECOVER_RETURN(t.type() <= QTransform::TxScale);
        transform(t);
        m_originalRect = t.mapRect(m_originalRect);
    }

    ControlPointIndex hitTestNode(const QPointF &pt, qreal distanceThreshold) const {
        return hitTestPointImpl(pt, distanceThreshold, true);
    }

    ControlPointIndex hitTestControlPoint(const QPointF &pt, qreal distanceThreshold) const {
        return hitTestPointImpl(pt, distanceThreshold, false);
    }

    SegmentIndex hitTestSegment(const QPointF &pt, qreal distanceThreshold, qreal *t = 0) const {
        auto result = endSegments();
        qreal minDistance = std::numeric_limits<qreal>::max();

        for (auto it = beginSegments(); it != endSegments(); ++it) {

            qreal foundDistance = 0.0;
            const qreal foundT = KisBezierUtils::nearestPoint({it.p0(), it.p1(), it.p2(), it.p3()}, pt, &foundDistance);

            if (foundDistance < minDistance && foundDistance < distanceThreshold) {
                result = it;
                minDistance = foundDistance;
                if (t) {
                    *t = foundT;
                }
            }
        }

        return result.segmentIndex();
    }

    template <typename T>
    bool isIndexValid(const T &index) const {
        return find(index).isValid();
    }

    void reshapeMeshHorizontally(int numColumns) {
        KIS_SAFE_ASSERT_RECOVER_RETURN(numColumns >= 2);

        std::vector<int> insertedIndexes;

        for (int i = 1; i < numColumns - 1; i++) {
            const qreal pos = qreal(i) / (numColumns - 1);
            int inserted = subdivideColumn(pos);
            KIS_SAFE_ASSERT_RECOVER(inserted >= 0) { continue; }

            insertedIndexes.push_back(inserted);
        }

        for (int i = m_columns.size() - 2; i >= 1; i--) {
            if (std::find(insertedIndexes.begin(), insertedIndexes.end(), i) == insertedIndexes.end()) {
                removeColumn(i);
            }
        }
    }

    void reshapeMeshVertically(int numRows) {
        KIS_SAFE_ASSERT_RECOVER_RETURN(numRows >= 2);

        std::vector<int> insertedIndexes;

        for (int i = 1; i < numRows - 1; i++) {
            const qreal pos = qreal(i) / (numRows - 1);
            int inserted = subdivideRow(pos);
            KIS_SAFE_ASSERT_RECOVER(inserted >= 0) { continue; }

            insertedIndexes.push_back(inserted);
        }

        for (int i = m_rows.size() - 2; i >= 1; i--) {
            if (std::find(insertedIndexes.begin(), insertedIndexes.end(), i) == insertedIndexes.end()) {
                removeRow(i);
            }
        }
    }

private:
    void splitCurveHorizontally(Node &left, Node &right, qreal t, Node &newNode) {
        using KisBezierUtils::deCasteljau;
        using KisAlgebra2D::lerp;

        QPointF p1, p2, p3, q1, q2;

        deCasteljau(left.node, left.rightControl, right.leftControl, right.node, t,
                    &p1, &p2, &p3, &q1, &q2);

        left.rightControl = p1;
        newNode.leftControl = p2;
        newNode.node = p3;
        newNode.rightControl = q1;
        right.leftControl = q2;

        newNode.topControl = newNode.node + lerp(left.topControl - left.node, right.topControl - right.node, t);
        newNode.bottomControl = newNode.node + lerp(left.bottomControl - left.node, right.bottomControl - right.node, t);

        lerpNodeData(left, right, t, newNode);
    }

    void splitCurveVertically(Node &top, Node &bottom, qreal t, Node &newNode) {
        using KisBezierUtils::deCasteljau;
        using KisAlgebra2D::lerp;

        QPointF p1, p2, p3, q1, q2;

        deCasteljau(top.node, top.bottomControl, bottom.topControl, bottom.node, t,
                    &p1, &p2, &p3, &q1, &q2);

        top.bottomControl = p1;
        newNode.topControl = p2;
        newNode.node = p3;
        newNode.bottomControl = q1;
        bottom.topControl = q2;

        newNode.leftControl = newNode.node + lerp(top.leftControl - top.node, bottom.leftControl - bottom.node, t);
        newNode.rightControl = newNode.node + lerp(top.rightControl - top.node, bottom.rightControl - bottom.node, t);

        lerpNodeData(top, bottom, t, newNode);
    }

    void unlinkNodeHorizontally(Mesh::Node &left, const Mesh::Node &node, Mesh::Node &right)
    {
        std::tie(left.rightControl, right.leftControl) =
            KisBezierUtils::removeBezierNode(left.node, left.rightControl,
                                             node.leftControl, node.node, node.rightControl,
                                             right.leftControl, right.node);
    }

    void unlinkNodeVertically(Mesh::Node &top, const Mesh::Node &node, Mesh::Node &bottom)
    {
        std::tie(top.bottomControl, bottom.topControl) =
            KisBezierUtils::removeBezierNode(top.node, top.bottomControl,
                                             node.topControl, node.node, node.bottomControl,
                                             bottom.topControl, bottom.node);
    }

    ControlPointIndex hitTestPointImpl(const QPointF &pt, qreal distanceThreshold, bool onlyNodeMode) const {
        const qreal distanceThresholdSq = pow2(distanceThreshold);

        auto result = endControlPoints();
        qreal minDistanceSq = std::numeric_limits<qreal>::max();

        for (auto it = beginControlPoints(); it != endControlPoints(); ++it) {
            if (onlyNodeMode != (it.type() == ControlType::Node)) continue;

            const qreal distSq = kisSquareDistance(*it, pt);
            if (distSq < minDistanceSq && distSq < distanceThresholdSq) {
                result = it;
                minDistanceSq = distSq;
            }
        }

        return result.controlIndex();
    }

private:
    template <class MeshType,
              class IteratorType = control_point_iterator_impl<std::is_const<MeshType>::value>>
    static
    IteratorType find(MeshType &mesh, const ControlPointIndex &index) {
        IteratorType it(&mesh, index.nodeIndex.x(), index.nodeIndex.y(), index.controlType);
        return it.isValid() ? it : mesh.endControlPoints();
    }

    template <class MeshType,
              class IteratorType = control_point_iterator_impl<std::is_const<MeshType>::value>>
    static
    IteratorType find(MeshType &mesh, const NodeIndex &index) {
        IteratorType it(&mesh, index.x(), index.y(), Mesh::ControlType::Node);
        return it.isValid() ? it : mesh.endControlPoints();
    }

    template <class MeshType,
              class IteratorType = segment_iterator_impl<std::is_const<MeshType>::value>>
    static
    IteratorType find(MeshType &mesh, const SegmentIndex &index) {
        IteratorType it(&mesh, index.first.x(), index.first.y(), index.second);
        return it.isValid() ? it : mesh.endSegments();
    }

    template <class MeshType,
              class IteratorType = patch_iterator_impl<std::is_const<MeshType>::value>>
    static
    IteratorType find(MeshType &mesh, const PatchIndex &index) {
        IteratorType it(&mesh, index.x(), index.y());
        return it.isValid() ? it : mesh.endPatches();
    }

    template <class MeshType,
              class IteratorType = patch_iterator_impl<std::is_const<MeshType>::value>>
    static
    IteratorType beginPatches(MeshType &mesh) {
        return IteratorType(&mesh, 0, 0);
    }

    template <class MeshType,
              class IteratorType = patch_iterator_impl<std::is_const<MeshType>::value>>
    static
    IteratorType endPatches(MeshType &mesh) {
        return IteratorType(&mesh, 0, mesh.m_size.height() - 1);
    }

    template <class MeshType,
              class IteratorType = control_point_iterator_impl<std::is_const<MeshType>::value>>
    static
    IteratorType beginControlPoints(MeshType &mesh) {
        return IteratorType(&mesh, 0, 0, ControlType::RightControl);
    }

    template <class MeshType,
              class IteratorType = control_point_iterator_impl<std::is_const<MeshType>::value>>
    static
    IteratorType endControlPoints(MeshType &mesh) {
        return IteratorType(&mesh, 0, mesh.m_size.height(), 0);
    }

    template <class MeshType,
              class IteratorType = segment_iterator_impl<std::is_const<MeshType>::value>>
    static
    IteratorType beginSegments(MeshType &mesh) {
        return IteratorType(&mesh, 0, 0, 0);
    }

    template <class MeshType,
              class IteratorType = segment_iterator_impl<std::is_const<MeshType>::value>>
    static
    IteratorType endSegments(MeshType &mesh) {
        return IteratorType(&mesh, 0, mesh.m_size.height(), 0);
    }

protected:

    std::vector<Node> m_nodes;
    std::vector<qreal> m_rows;
    std::vector<qreal> m_columns;

    QSize m_size;
    QRectF m_originalRect;
};

template<typename Node, typename Patch>
QDebug operator<<(QDebug dbg, const Mesh<Node, Patch> &mesh)
{
    dbg.nospace() << "Mesh " << mesh.size() << "\n";

    for (int y = 0; y < mesh.size().height(); y++) {
        for (int x = 0; x < mesh.size().width(); x++) {
            dbg.nospace() << "    node(" << x << ", " << y << ") " << mesh.node(x, y) << "\n";
        }
    }
    return dbg.space();
}

template<typename NodeArg, typename PatchArg>
template<bool is_const>
typename Mesh<NodeArg, PatchArg>::template patch_iterator_impl<is_const>::SegmentIteratorType
Mesh<NodeArg, PatchArg>::patch_iterator_impl<is_const>::segmentP() const
{
    using SegmentIteratorType = typename Mesh<NodeArg, PatchArg>::template patch_iterator_impl<is_const>::SegmentIteratorType;
    return SegmentIteratorType(m_mesh, m_col, m_row, 1);
}

template<typename NodeArg, typename PatchArg>
template<bool is_const>
typename Mesh<NodeArg, PatchArg>::template patch_iterator_impl<is_const>::SegmentIteratorType
Mesh<NodeArg, PatchArg>::patch_iterator_impl<is_const>::segmentQ() const
{
    using SegmentIteratorType = typename Mesh<NodeArg, PatchArg>::template patch_iterator_impl<is_const>::SegmentIteratorType;
    return SegmentIteratorType(const_cast<Mesh<NodeArg, PatchArg>*>(m_mesh), m_col, m_row + 1, 1);
}

template<typename NodeArg, typename PatchArg>
template<bool is_const>
typename Mesh<NodeArg, PatchArg>::template patch_iterator_impl<is_const>::SegmentIteratorType
Mesh<NodeArg, PatchArg>::patch_iterator_impl<is_const>::segmentR() const
{
    using SegmentIteratorType = typename Mesh<NodeArg, PatchArg>::template patch_iterator_impl<is_const>::SegmentIteratorType;
    return SegmentIteratorType(const_cast<Mesh<NodeArg, PatchArg>*>(m_mesh), m_col, m_row, 0);
}

template<typename NodeArg, typename PatchArg>
template<bool is_const>
typename Mesh<NodeArg, PatchArg>::template patch_iterator_impl<is_const>::SegmentIteratorType
Mesh<NodeArg, PatchArg>::patch_iterator_impl<is_const>::segmentS() const
{
    using SegmentIteratorType = typename Mesh<NodeArg, PatchArg>::template patch_iterator_impl<is_const>::SegmentIteratorType;
    return SegmentIteratorType(const_cast<Mesh<NodeArg, PatchArg>*>(m_mesh), m_col + 1, m_row, 0);
}

template<typename NodeArg, typename PatchArg>
template<bool is_const>
typename Mesh<NodeArg, PatchArg>::template patch_iterator_impl<is_const>::ControlPointIteratorType
Mesh<NodeArg, PatchArg>::patch_iterator_impl<is_const>::nodeTopLeft() const
{
    using ControlPointIteratorType = typename Mesh<NodeArg, PatchArg>::template patch_iterator_impl<is_const>::ControlPointIteratorType;
    return ControlPointIteratorType(const_cast<Mesh<NodeArg, PatchArg>*>(m_mesh), m_col, m_row, Mesh::ControlType::Node);
}

template<typename NodeArg, typename PatchArg>
template<bool is_const>
typename Mesh<NodeArg, PatchArg>::template patch_iterator_impl<is_const>::ControlPointIteratorType
Mesh<NodeArg, PatchArg>::patch_iterator_impl<is_const>::nodeTopRight() const
{
    using ControlPointIteratorType = typename Mesh<NodeArg, PatchArg>::template patch_iterator_impl<is_const>::ControlPointIteratorType;
    return ControlPointIteratorType(const_cast<Mesh<NodeArg, PatchArg>*>(m_mesh), m_col + 1, m_row, Mesh::ControlType::Node);
}

template<typename NodeArg, typename PatchArg>
template<bool is_const>
typename Mesh<NodeArg, PatchArg>::template patch_iterator_impl<is_const>::ControlPointIteratorType
Mesh<NodeArg, PatchArg>::patch_iterator_impl<is_const>::nodeBottomLeft() const
{
    using ControlPointIteratorType = typename Mesh<NodeArg, PatchArg>::template patch_iterator_impl<is_const>::ControlPointIteratorType;
    return ControlPointIteratorType(const_cast<Mesh<NodeArg, PatchArg>*>(m_mesh), m_col, m_row + 1, Mesh::ControlType::Node);
}

template<typename NodeArg, typename PatchArg>
template<bool is_const>
typename Mesh<NodeArg, PatchArg>::template patch_iterator_impl<is_const>::ControlPointIteratorType
Mesh<NodeArg, PatchArg>::patch_iterator_impl<is_const>::nodeBottomRight() const
{
    using ControlPointIteratorType = typename Mesh<NodeArg, PatchArg>::template patch_iterator_impl<is_const>::ControlPointIteratorType;
    return ControlPointIteratorType(const_cast<Mesh<NodeArg, PatchArg>*>(m_mesh), m_col + 1, m_row + 1, Mesh::ControlType::Node);
}


template<typename NodeArg, typename PatchArg>
template<bool is_const>
typename Mesh<NodeArg, PatchArg>::template control_point_iterator_impl<is_const>::SegmentIteratorType
Mesh<NodeArg, PatchArg>::control_point_iterator_impl<is_const>::topSegment() const
{
    using SegmentIteratorType = typename Mesh<NodeArg, PatchArg>::template control_point_iterator_impl<is_const>::SegmentIteratorType;

    if (isTopBorder()) {
        return m_mesh->endSegments();
    }

    return SegmentIteratorType(m_mesh, m_col, m_row - 1, false);
}

template<typename NodeArg, typename PatchArg>
template<bool is_const>
typename Mesh<NodeArg, PatchArg>::template control_point_iterator_impl<is_const>::SegmentIteratorType
Mesh<NodeArg, PatchArg>::control_point_iterator_impl<is_const>::bottomSegment() const
{
    using SegmentIteratorType = typename Mesh<NodeArg, PatchArg>::template control_point_iterator_impl<is_const>::SegmentIteratorType;

    if (isBottomBorder()) {
        return m_mesh->endSegments();
    }

    return SegmentIteratorType(m_mesh, m_col, m_row, false);
}

template<typename NodeArg, typename PatchArg>
template<bool is_const>
typename Mesh<NodeArg, PatchArg>::template control_point_iterator_impl<is_const>::SegmentIteratorType
Mesh<NodeArg, PatchArg>::control_point_iterator_impl<is_const>::leftSegment() const
{
    using SegmentIteratorType = typename Mesh<NodeArg, PatchArg>::template control_point_iterator_impl<is_const>::SegmentIteratorType;

    if (isLeftBorder()) {
        return m_mesh->endSegments();
    }

    return SegmentIteratorType(m_mesh, m_col - 1, m_row, true);
}

template<typename NodeArg, typename PatchArg>
template<bool is_const>
typename Mesh<NodeArg, PatchArg>::template control_point_iterator_impl<is_const>::SegmentIteratorType
Mesh<NodeArg, PatchArg>::control_point_iterator_impl<is_const>::rightSegment() const
{
    using SegmentIteratorType = typename Mesh<NodeArg, PatchArg>::template control_point_iterator_impl<is_const>::SegmentIteratorType;

    if (isRightBorder()) {
        return m_mesh->endSegments();
    }

    return SegmentIteratorType(m_mesh, m_col, m_row, true);
}

enum SmartMoveMeshControlMode {
    MoveFree,
    MoveSymmetricLock,
    MoveRotationLock
};

template<typename NodeArg, typename PatchArg>
void smartMoveControl(Mesh<NodeArg, PatchArg> &mesh,
                      typename Mesh<NodeArg, PatchArg>::ControlPointIndex index,
                      const QPointF &move,
                      SmartMoveMeshControlMode mode,
                      bool scaleNodeMoves)
{
    using ControlType = typename Mesh<NodeArg, PatchArg>::ControlType;
    using ControlPointIndex = typename Mesh<NodeArg, PatchArg>::ControlPointIndex;
    using ControlPointIterator = typename Mesh<NodeArg, PatchArg>::control_point_iterator;
    using SegmentIterator = typename Mesh<NodeArg, PatchArg>::segment_iterator;

    auto it = mesh.find(index);
    KIS_SAFE_ASSERT_RECOVER_RETURN(it != mesh.endControlPoints());

    if (it.isNode()) {
        auto preAdjustSegment = [] (Mesh<NodeArg, PatchArg> &mesh,
                                    SegmentIterator it,
                                    const QPointF &normalizedOffset) {

            if (it == mesh.endSegments()) return;

            const QPointF base1 = it.p3() - it.p0();
            const QPointF base2 = it.p3() - it.p0() - normalizedOffset;

            {
                const QPointF control = it.p1() - it.p0();
                const qreal dist0 = KisAlgebra2D::norm(base1);
                const qreal dist1 = KisAlgebra2D::dotProduct(base2, base1) / dist0;
                const qreal coeff = dist1 / dist0;

                it.p1() = it.p0() + coeff * (control);
            }
            {
                const QPointF control = it.p2() - it.p3();
                const qreal dist0 = KisAlgebra2D::norm(base1);
                const qreal dist1 = KisAlgebra2D::dotProduct(base2, base1) / dist0;
                const qreal coeff = dist1 / dist0;

                it.p2() = it.p3() + coeff * (control);
            }
        };

        if (scaleNodeMoves) {
            preAdjustSegment(mesh, it.topSegment(), -move);
            preAdjustSegment(mesh, it.leftSegment(), -move);
            preAdjustSegment(mesh, it.bottomSegment(), move);
            preAdjustSegment(mesh, it.rightSegment(), move);
        }

        it.node().translate(move);

    } else {
        const QPointF newPos = *it + move;

        if (mode == MoveRotationLock || mode == MoveSymmetricLock) {
            const qreal rotation = KisAlgebra2D::angleBetweenVectors(*it - it.node().node,
                                                                     newPos - it.node().node);
            QTransform R;
            R.rotateRadians(rotation);

            const QTransform t =
                    QTransform::fromTranslate(-it.node().node.x(), -it.node().node.y()) *
                    R *
                    QTransform::fromTranslate(it.node().node.x(), it.node().node.y());

            if (mode == MoveRotationLock) {
                for (int intType = 0; intType < 4; intType++) {
                    ControlType type = static_cast<ControlType>(intType);

                    if (type == ControlType::Node ||
                            type == index.controlType) {

                        continue;
                    }

                    auto neighbourIt = mesh.find(ControlPointIndex(index.nodeIndex, type));
                    if (neighbourIt == mesh.endControlPoints()) continue;

                    *neighbourIt = t.map(*neighbourIt);
                }
            } else {
                auto neighbourIt = it.symmetricControl();
                if (neighbourIt != mesh.endControlPoints()) {
                    *neighbourIt = t.map(*neighbourIt);
                }
            }
        }

        *it = newPos;
    }
}

KRITAGLOBAL_EXPORT
QDebug operator<<(QDebug dbg, const BaseMeshNode &n);

KRITAGLOBAL_EXPORT
void saveValue(QDomElement *parent, const QString &tag, const BaseMeshNode &node);

KRITAGLOBAL_EXPORT
bool loadValue(const QDomElement &parent, BaseMeshNode *node);

}

namespace KisDomUtils {
using KisBezierMeshDetails::loadValue;
using KisBezierMeshDetails::saveValue;
}

template <typename Node, typename Patch>
using KisBezierMeshBase = KisBezierMeshDetails::Mesh<Node, Patch>;

using KisSmartMoveMeshControlMode = KisBezierMeshDetails::SmartMoveMeshControlMode;
using KisBezierMesh = KisBezierMeshDetails::Mesh<KisBezierMeshDetails::BaseMeshNode, KisBezierPatch>;


#endif // KISBEZIERMESH_H
