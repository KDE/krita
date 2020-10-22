#ifndef KISBEZIERMESH_H
#define KISBEZIERMESH_H

#include <kritaglobal_export.h>

#include <QDebug>

#include <KisBezierUtils.h>
#include <KisBezierPatch.h>

#include <boost/iterator/iterator_facade.hpp>

#include <functional>

namespace KisBezierMeshDetails {

struct BaseMeshNode {
    BaseMeshNode() {}
    BaseMeshNode(const QPointF &_node)
        : leftControl(_node),
          topControl(_node),
          node(_node),
          rightControl(_node),
          bottomControl(_node)
    {
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

KRITAGLOBAL_EXPORT
QDebug operator<<(QDebug dbg, const BaseMeshNode &n);

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

template<typename Node = BaseMeshNode,
         typename Patch = KisBezierPatch>
class Mesh
{
public:
    Mesh(const QRectF &mapRect, const QSize &size = QSize(2,2))
        : m_size(size),
          m_originalRect(mapRect)
    {
        for (int row = 0; row < m_size.height(); row++) {
            const qreal yPos = qreal(row) / (size.height() - 1) * mapRect.height() + mapRect.y();

            for (int col = 0; col < m_size.width(); col++) {
                const qreal xPos = qreal(col) / (size.width() - 1) * mapRect.width() + mapRect.x();

                m_nodes.push_back(Node(QPointF(xPos, yPos)));
            }
        }

        for (int col = 0; col < m_size.width(); col++) {
            m_columns.push_back(qreal(col) / (size.width() - 1));
        }

        for (int row = 0; row < m_size.height(); row++) {
            m_rows.push_back(qreal(row) / (size.height() - 1));
        }
    }

    void splitCurveHorizontally(Node &left, Node &right, qreal t, Node &newNode) {
        using KisBezierUtils::splitBezierCurve;
        using KisAlgebra2D::lerp;

        QPointF p1, p2, p3, q1, q2;

        splitBezierCurve(left.node, left.rightControl, right.leftControl, right.node, t,
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

    Node& node(int col, int row) {
        return m_nodes[row * m_size.width() + col];
    }

    const Node& node(int col, int row) const {
        return m_nodes[row * m_size.width() + col];
    }


    void splitCurveVertically(Node &top, Node &bottom, qreal t, Node &newNode) {
        using KisBezierUtils::splitBezierCurve;
        using KisAlgebra2D::lerp;

        QPointF p1, p2, p3, q1, q2;

        splitBezierCurve(top.node, top.bottomControl, bottom.topControl, bottom.node, t,
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

    void subdivideRow(qreal t) {
        if (qFuzzyCompare(t, 0.0) || qFuzzyCompare(t, 1.0)) return;

        KIS_SAFE_ASSERT_RECOVER_RETURN(t > 0.0 && t < 1.0);

        const auto it = prev(upper_bound(m_rows.begin(), m_rows.end(), t));
        const int topRow = distance(m_rows.begin(), it);
        const int bottomRow = topRow + 1;


        const qreal relT = (t - *it) / (*next(it) - *it);

        std::vector<Node> newRow;
        newRow.resize(m_size.width());
        for (int col = 0; col < m_size.width(); col++) {
            splitCurveVertically(node(col, topRow), node(col, bottomRow), relT, newRow[col]);
        }

        m_nodes.insert(m_nodes.begin() + bottomRow * m_size.width(),
                       newRow.begin(), newRow.end());

        m_size.rheight()++;
        m_rows.insert(next(it), t);
    }

    void subdivideColumn(qreal t) {
        if (qFuzzyCompare(t, 0.0) || qFuzzyCompare(t, 1.0)) return;

        KIS_SAFE_ASSERT_RECOVER_RETURN(t > 0.0 && t < 1.0);

        const auto it = prev(upper_bound(m_columns.begin(), m_columns.end(), t));
        const int leftColumn = distance(m_columns.begin(), it);
        const int rightColumn = leftColumn + 1;

        const qreal relT = (t - *it) / (*next(it) - *it);

        std::vector<Node> newColumn;
        newColumn.resize(m_size.height());
        for (int row = 0; row < m_size.height(); row++) {
            splitCurveHorizontally(node(leftColumn, row), node(rightColumn, row), relT, newColumn[row]);
        }

        auto dstIt = m_nodes.begin() + rightColumn;
        for (auto columnIt = newColumn.begin(); columnIt != newColumn.end(); ++columnIt) {
            dstIt = m_nodes.insert(dstIt, *columnIt);
            dstIt += m_size.width() + 1;
        }

        m_size.rwidth()++;
        m_columns.insert(next(it), t);
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

    class iterator :
        public boost::iterator_facade <iterator,
                                       Patch,
                                       boost::random_access_traversal_tag,
                                       Patch>
    {
    public:
        iterator()
            : m_mesh(0),
              m_col(0),
              m_row(0) {}

        iterator(const Mesh* mesh, int col, int row)
            : m_mesh(mesh),
              m_col(col),
              m_row(row)
        {
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

        int distance_to(const iterator &z) const {
            const int index = m_row * (m_mesh->m_size.width() - 1) + m_col;
            const int otherIndex = z.m_row * (m_mesh->m_size.width() - 1) + z.m_col;

            return otherIndex - index;
        }

        bool equal(iterator const& other) const {
            return m_row == other.m_row &&
                    m_col == other.m_col &&
                m_mesh == other.m_mesh;
        }

        Patch dereference() const {
            return m_mesh->makePatch(m_col, m_row);
        }

    private:

        const Mesh* m_mesh;
        int m_col;
        int m_row;
    };


    iterator begin() const {
        return iterator(this, 0, 0);
    }

    iterator end() const {
        return iterator(this, 0, m_size.height() - 1);
    }

    QSize size() const {
        return m_size;
    }

    QRectF originalRect() const {
        return m_originalRect;
    }

    QRectF dstBoundingRect() const {
        QRectF result;
        for (auto it = begin(); it != end(); ++it) {
            result |= it->dstBoundingRect();
        }
        return result;
    }

private:

    std::vector<Node> m_nodes;
    std::vector<qreal> m_rows;
    std::vector<qreal> m_columns;

    QSize m_size;
    QRectF m_originalRect;
};

template<typename Mesh>
QDebug operator<<(QDebug dbg, const Mesh &mesh)
{
    dbg.nospace() << "Mesh " << mesh.size() << "\n";

    for (int y = 0; y < mesh.size().height(); y++) {
        for (int x = 0; x < mesh.size().width(); x++) {
            dbg.nospace() << "    node(" << x << ", " << y << ") " << mesh.node(x, y) << "\n";
        }
    }
    return dbg.space();
}


}

template <typename Node, typename Patch>
using KisBezierMeshBase = KisBezierMeshDetails::Mesh<Node, Patch>;

using KisBezierMesh = KisBezierMeshDetails::Mesh<KisBezierMeshDetails::BaseMeshNode, KisBezierPatch>;


#endif // KISBEZIERMESH_H
