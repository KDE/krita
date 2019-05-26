/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_LAZY_FILL_GRAPH_H
#define __KIS_LAZY_FILL_GRAPH_H

#include <numeric>
#include <boost/limits.hpp>
#include <boost/operators.hpp>

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/properties.hpp>
#include <boost/iterator/counting_iterator.hpp>
#include <boost/iterator/transform_iterator.hpp>

#include <QRegion>

//#define USE_LAZY_FILL_SANITY_CHECKS 1

#ifdef USE_LAZY_FILL_SANITY_CHECKS
#define LF_SANITY_ASSERT(x) KIS_ASSERT(x)
#define LF_SANITY_ASSERT_RECOVER(x) KIS_ASSERT_RECOVER(x)
#else
#define LF_SANITY_ASSERT(x)
#define LF_SANITY_ASSERT_RECOVER(x) if (0)
#endif /* USE_LAZY_FILL_SANITY_CHECKS */


using namespace boost;

/*
BOOST_CONCEPT_ASSERT(( ReadablePropertyMapConcept<ScribbleMap, vertex_descriptor> )); //read flow-values from vertices
*/

class KisLazyFillGraph;

//===================
// Index Property Map
//===================

template <typename Graph,
          typename Descriptor,
          typename Index>
struct lazy_fill_graph_index_map {
public:
    typedef Index value_type;
    typedef Index reference_type;
    typedef reference_type reference;
    typedef Descriptor key_type;
    typedef readable_property_map_tag category;

    lazy_fill_graph_index_map() { }

    lazy_fill_graph_index_map(const Graph& graph) :
        m_graph(&graph) { }

    value_type operator[](key_type key) const {
        value_type index = m_graph->index_of(key);
        LF_SANITY_ASSERT(index >= 0);
        return index;
    }

    friend inline Index
    get(const lazy_fill_graph_index_map<Graph, Descriptor, Index>& index_map,
        const typename lazy_fill_graph_index_map<Graph, Descriptor, Index>::key_type& key)
        {
            return (index_map[key]);
        }

protected:
    const Graph* m_graph;
};

//==========================
// Reverse Edge Property Map
//==========================

template <typename Descriptor>
struct lazy_fill_graph_reverse_edge_map {
public:
    typedef Descriptor value_type;
    typedef Descriptor reference_type;
    typedef reference_type reference;
    typedef Descriptor key_type;
    typedef readable_property_map_tag category;

    lazy_fill_graph_reverse_edge_map() { }

    value_type operator[](const key_type& key) const {
        return (value_type(key.second, key.first));
    }

    friend inline Descriptor
    get(const lazy_fill_graph_reverse_edge_map<Descriptor>& rev_map,
        const typename lazy_fill_graph_reverse_edge_map<Descriptor>::key_type& key)
        {
            return (rev_map[key]);
        }
};

//=================
// Function Objects
//=================

namespace kis_detail {

    // vertex_at
    template <typename Graph>
    struct lazy_fill_graph_vertex_at {

        typedef typename graph_traits<Graph>::vertex_descriptor result_type;

        lazy_fill_graph_vertex_at() : m_graph(0) {}

        lazy_fill_graph_vertex_at(const Graph* graph) :
            m_graph(graph) { }

        result_type
        operator()
        (typename graph_traits<Graph>::vertices_size_type vertex_index) const {
            return (vertex(vertex_index, *m_graph));
        }

    private:
        const Graph* m_graph;
    };

    // out_edge_at
    template <typename Graph>
    struct lazy_fill_graph_out_edge_at {

    private:
        typedef typename graph_traits<Graph>::vertex_descriptor vertex_descriptor;

    public:
        typedef typename graph_traits<Graph>::edge_descriptor result_type;

        lazy_fill_graph_out_edge_at() : m_vertex(), m_graph(0) {}

        lazy_fill_graph_out_edge_at(vertex_descriptor source_vertex,
                               const Graph* graph) :
            m_vertex(source_vertex),
            m_graph(graph) { }

        result_type
        operator()
        (typename graph_traits<Graph>::degree_size_type out_edge_index) const {
            return (out_edge_at(m_vertex, out_edge_index, *m_graph));
        }

    private:
        vertex_descriptor m_vertex;
        const Graph* m_graph;
    };

    // in_edge_at
    template <typename Graph>
    struct lazy_fill_graph_in_edge_at {

    private:
        typedef typename graph_traits<Graph>::vertex_descriptor vertex_descriptor;

    public:
        typedef typename graph_traits<Graph>::edge_descriptor result_type;

        lazy_fill_graph_in_edge_at() : m_vertex(), m_graph(0) {}

        lazy_fill_graph_in_edge_at(vertex_descriptor target_vertex,
                              const Graph* graph) :
            m_vertex(target_vertex),
            m_graph(graph) { }

        result_type
        operator()
        (typename graph_traits<Graph>::degree_size_type in_edge_index) const {
            return (in_edge_at(m_vertex, in_edge_index, *m_graph));
        }

    private:
        vertex_descriptor m_vertex;
        const Graph* m_graph;
    };

    // edge_at
    template <typename Graph>
    struct lazy_fill_graph_edge_at {

        typedef typename graph_traits<Graph>::edge_descriptor result_type;

        lazy_fill_graph_edge_at() : m_graph(0) {}

        lazy_fill_graph_edge_at(const Graph* graph) :
            m_graph(graph) { }

        result_type
        operator()
        (typename graph_traits<Graph>::edges_size_type edge_index) const {
            return (edge_at(edge_index, *m_graph));
        }

    private:
        const Graph* m_graph;
    };

    // adjacent_vertex_at
    template <typename Graph>
    struct lazy_fill_graph_adjacent_vertex_at {

    public:
        typedef typename graph_traits<Graph>::vertex_descriptor result_type;

        lazy_fill_graph_adjacent_vertex_at(result_type source_vertex,
                                      const Graph* graph) :
            m_vertex(source_vertex),
            m_graph(graph) { }

        result_type
        operator()
        (typename graph_traits<Graph>::degree_size_type adjacent_index) const {
            return (target(out_edge_at(m_vertex, adjacent_index, *m_graph), *m_graph));
        }

    private:
        result_type m_vertex;
        const Graph* m_graph;
    };

} // namespace kis_detail


class KisLazyFillGraph
{
public:
    typedef KisLazyFillGraph type;


    typedef long VertexIndex;
    typedef long EdgeIndex;

    // sizes
    typedef VertexIndex vertices_size_type;
    typedef EdgeIndex edges_size_type;
    typedef EdgeIndex degree_size_type;

    struct VertexDescriptor : public equality_comparable<VertexDescriptor>
    {
        enum VertexType {
            NORMAL = 0,
            LABEL_A,
            LABEL_B
        };

        vertices_size_type x;
        vertices_size_type y;
        VertexType type;

        VertexDescriptor(vertices_size_type _x, vertices_size_type _y, VertexType _type = NORMAL)
            : x(_x), y(_y), type(_type) {}

        // TODO: Extra constructors look unnecessary, ask Dmitry before removing
        VertexDescriptor(VertexType _type)
            : x(0), y(0), type(_type) {}

        VertexDescriptor()
            : x(0), y(0), type(NORMAL) {}

        bool operator ==(const VertexDescriptor &rhs) const {
            return rhs.x == x && rhs.y == y && rhs.type == type;
        }
    };

    // descriptors
    typedef VertexDescriptor vertex_descriptor;
    typedef std::pair<vertex_descriptor, vertex_descriptor> edge_descriptor;

    friend QDebug operator<<(QDebug dbg, const KisLazyFillGraph::edge_descriptor &e);
    friend QDebug operator<<(QDebug dbg, const KisLazyFillGraph::vertex_descriptor &v);

    // vertex_iterator
    typedef counting_iterator<vertices_size_type> vertex_index_iterator;
    typedef kis_detail::lazy_fill_graph_vertex_at<KisLazyFillGraph> vertex_function;
    typedef transform_iterator<vertex_function, vertex_index_iterator> vertex_iterator;

    // edge_iterator
    typedef counting_iterator<edges_size_type> edge_index_iterator;
    typedef kis_detail::lazy_fill_graph_edge_at<type> edge_function;
    typedef transform_iterator<edge_function, edge_index_iterator> edge_iterator;

    // out_edge_iterator
    typedef counting_iterator<degree_size_type> degree_iterator;
    typedef kis_detail::lazy_fill_graph_out_edge_at<type> out_edge_function;
    typedef transform_iterator<out_edge_function, degree_iterator> out_edge_iterator;

    // adjacency_iterator
    typedef kis_detail::lazy_fill_graph_adjacent_vertex_at<type> adjacent_vertex_function;
    typedef transform_iterator<adjacent_vertex_function, degree_iterator> adjacency_iterator;

    // categories
    typedef directed_tag directed_category;
    typedef disallow_parallel_edge_tag edge_parallel_category;
    struct traversal_category : virtual public incidence_graph_tag,
                                virtual public adjacency_graph_tag,
                                virtual public vertex_list_graph_tag,
                                virtual public edge_list_graph_tag,
                                virtual public adjacency_matrix_tag { };

    static inline vertex_descriptor null_vertex()
    {
        vertex_descriptor maxed_out_vertex(
            std::numeric_limits<vertices_size_type>::max(),
            std::numeric_limits<vertices_size_type>::max(),
            vertex_descriptor::NORMAL);

        return (maxed_out_vertex);
    }

    KisLazyFillGraph() {}

    KisLazyFillGraph(const QRect &mainRect,
                     const QRegion &aLabelRegion,
                     const QRegion &bLabelRegion)
        : m_x(mainRect.x()),
          m_y(mainRect.y()),
          m_width(mainRect.width()),
          m_height(mainRect.height())
    {
        m_mainArea = mainRect;
        m_aLabelArea = aLabelRegion.boundingRect();
        m_bLabelArea = bLabelRegion.boundingRect();
        m_aLabelRects = aLabelRegion.rects();
        m_bLabelRects = bLabelRegion.rects();

        KIS_ASSERT(m_mainArea.contains(m_aLabelArea));
        KIS_ASSERT(m_mainArea.contains(m_bLabelArea));

        m_numVertices = m_width * m_height + 2;

        m_edgeBins << EdgeIndexBin(0,                 m_mainArea.adjusted(0, 0, -1, 0), HORIZONTAL);
        m_edgeBins << EdgeIndexBin(m_edgeBins.last(), m_mainArea.adjusted(0, 0, -1, 0), HORIZONTAL_REVERSED);

        m_edgeBins << EdgeIndexBin(m_edgeBins.last(), m_mainArea.adjusted(0, 0, 0, -1), VERTICAL);
        m_edgeBins << EdgeIndexBin(m_edgeBins.last(), m_mainArea.adjusted(0, 0, 0, -1), VERTICAL_REVERSED);

        Q_FOREACH (const QRect &rc, m_aLabelRects) {
            m_edgeBins << EdgeIndexBin(m_edgeBins.last(), rc, LABEL_A);
        }

        // out_edge_at relies on the sequential layout of reversed edges of one type
        m_aReversedEdgesStart = m_edgeBins.last().last() + 1;

        Q_FOREACH (const QRect &rc, m_aLabelRects) {
            m_edgeBins << EdgeIndexBin(m_edgeBins.last(), rc, LABEL_A_REVERSED);
        }

        m_numAEdges = m_edgeBins.last().last() + 1 - m_aReversedEdgesStart;

        Q_FOREACH (const QRect &rc, m_bLabelRects) {
            m_edgeBins << EdgeIndexBin(m_edgeBins.last(), rc, LABEL_B);
        }

        // out_edge_at relies on the sequential layout of reversed edges of one type
        m_bReversedEdgesStart = m_edgeBins.last().last() + 1;

        Q_FOREACH (const QRect &rc, m_bLabelRects) {
            m_edgeBins << EdgeIndexBin(m_edgeBins.last(), rc, LABEL_B_REVERSED);
        }

        m_numBEdges = m_edgeBins.last().last() + 1 - m_bReversedEdgesStart;

        m_numEdges = m_edgeBins.last().last() + 1;
    }

    ~KisLazyFillGraph() {

    }

    QSize size() const { return QSize(m_width, m_height); }
    QRect rect() const { return QRect(m_x, m_y, m_width, m_height); }


    vertices_size_type m_x;
    vertices_size_type m_y;

    vertices_size_type m_width;
    vertices_size_type m_height;

    vertices_size_type m_numVertices;
    vertices_size_type m_numEdges;

    vertices_size_type m_aReversedEdgesStart;
    vertices_size_type m_bReversedEdgesStart;
    vertices_size_type m_numAEdges;
    vertices_size_type m_numBEdges;

    enum EdgeIndexBinId {
        HORIZONTAL,
        HORIZONTAL_REVERSED,
        VERTICAL,
        VERTICAL_REVERSED,
        LABEL_A,
        LABEL_A_REVERSED,
        LABEL_B,
        LABEL_B_REVERSED,
    };

    struct EdgeIndexBin {
        EdgeIndexBin()
            : start(0), stride(0), size(0) {}

        EdgeIndexBin(edges_size_type _start,
                     const QRect &_rect,
                     EdgeIndexBinId _binId)
            : start(_start),
              stride(_rect.width()),
              size(_rect.width() * _rect.height()),
              xOffset(_rect.x()),
              yOffset(_rect.y()),
              binId(_binId),
              isReversed(int(_binId) & 0x1),
              rect(_rect) {}

        EdgeIndexBin(const EdgeIndexBin &putAfter,
                     const QRect &_rect,
                     EdgeIndexBinId _binId)
            : start(putAfter.last() + 1),
              stride(_rect.width()),
              size(_rect.width() * _rect.height()),
              xOffset(_rect.x()),
              yOffset(_rect.y()),
              binId(_binId),
              isReversed(int(_binId) & 0x1),
              rect(_rect) {}

        edges_size_type last() const {
            return start + size - 1;
        }

        bool contains(edges_size_type index) const {
            index -= start;
            return index >= 0 && index < size;
        }

        bool contains(const edge_descriptor &edge) const {
            return indexOf(edge) >= 0;
        }

        edges_size_type indexOf(const edge_descriptor &edge) const {
            vertex_descriptor src_vertex = source(edge, *this);
            vertex_descriptor dst_vertex = target(edge, *this);

            const bool srcColoredA = src_vertex.type == vertex_descriptor::LABEL_A;
            const bool dstColoredA = dst_vertex.type == vertex_descriptor::LABEL_A;
            const bool srcColoredB = src_vertex.type == vertex_descriptor::LABEL_B;
            const bool dstColoredB = dst_vertex.type == vertex_descriptor::LABEL_B;

            if (srcColoredA || dstColoredA) {
                const bool edgeReversed = srcColoredA;

                if (isReversed != edgeReversed ||
                    (binId != LABEL_A && binId != LABEL_A_REVERSED) ||
                    (srcColoredA && (dst_vertex.type != vertex_descriptor::NORMAL)) ||
                    (dstColoredA && (src_vertex.type != vertex_descriptor::NORMAL))) {

                    return -1;
                }
            } else if (srcColoredB || dstColoredB) {
                const bool edgeReversed = srcColoredB;

                if (isReversed != edgeReversed ||
                    (binId != LABEL_B && binId != LABEL_B_REVERSED) ||
                    (srcColoredB && (dst_vertex.type != vertex_descriptor::NORMAL)) ||
                    (dstColoredB && (src_vertex.type != vertex_descriptor::NORMAL))) {

                    return -1;
                }
            } else {
                const vertices_size_type xDiff = dst_vertex.x - src_vertex.x;
                const vertices_size_type yDiff = dst_vertex.y - src_vertex.y;
                const vertices_size_type xAbsDiff = qAbs(xDiff);
                const vertices_size_type yAbsDiff = qAbs(yDiff);
                const bool edgeReversed = xDiff < 0 || yDiff < 0;

                if (isReversed != edgeReversed ||
                    (xDiff && binId != HORIZONTAL && binId != HORIZONTAL_REVERSED) ||
                    (yDiff && binId != VERTICAL && binId != VERTICAL_REVERSED) ||
                    xAbsDiff > 1 ||
                    yAbsDiff > 1 ||
                    xAbsDiff == yAbsDiff) {

                    return -1;
                }
            }

            if (isReversed) {
                std::swap(src_vertex, dst_vertex);
            }

            // using direct QRect::contains makes the code 30% slower
            const int x = src_vertex.x;
            const int y = src_vertex.y;
            if (x < rect.x() || x > rect.right() ||
                y < rect.y() || y > rect.bottom()) {
                return -1;
            }

            edges_size_type internalIndex =
                (src_vertex.x - xOffset) +
                (src_vertex.y - yOffset) * stride;

            LF_SANITY_ASSERT_RECOVER(internalIndex >= 0 && internalIndex < size) {
                return -1;
            }

            return internalIndex + start;
        }


        edge_descriptor edgeAt(edges_size_type index) const {
            edges_size_type localOffset = index - start;

            if (localOffset < 0 || localOffset >= size) {
                return edge_descriptor();
            }

            const edges_size_type x = localOffset % stride + xOffset;
            const edges_size_type y = localOffset / stride + yOffset;

            vertex_descriptor src_vertex(x, y, vertex_descriptor::NORMAL);
            vertex_descriptor dst_vertex;

            switch (binId) {
            case HORIZONTAL:
            case HORIZONTAL_REVERSED:
                dst_vertex.x = x + 1;
                dst_vertex.y = y;
                dst_vertex.type = vertex_descriptor::NORMAL;
                break;
            case VERTICAL:
            case VERTICAL_REVERSED:
                dst_vertex.x = x;
                dst_vertex.y = y + 1;
                dst_vertex.type = vertex_descriptor::NORMAL;
                break;
            case LABEL_A:
            case LABEL_A_REVERSED:
                dst_vertex.type = vertex_descriptor::LABEL_A;
                break;
            case LABEL_B:
            case LABEL_B_REVERSED:
                dst_vertex.type = vertex_descriptor::LABEL_B;
                break;
            }

            if (isReversed) {
                std::swap(src_vertex, dst_vertex);
            }

            return std::make_pair(src_vertex, dst_vertex);
        }

        edges_size_type start;
        edges_size_type stride;
        edges_size_type size;
        edges_size_type xOffset;
        edges_size_type yOffset;
        EdgeIndexBinId binId;
        bool isReversed;
        QRect rect;
    };

    QVector<EdgeIndexBin> m_edgeBins;

    QRect m_aLabelArea;
    QRect m_bLabelArea;
    QRect m_mainArea;

    QVector<QRect> m_aLabelRects;
    QVector<QRect> m_bLabelRects;

public:

    // Returns the number of vertices in the graph
    inline vertices_size_type num_vertices() const {
      return (m_numVertices);
    }

    // Returns the number of edges in the graph
    inline edges_size_type num_edges() const {
      return (m_numEdges);
    }

    // Returns the index of [vertex] (See also vertex_at)
    vertices_size_type index_of(vertex_descriptor vertex) const {
        vertices_size_type vertex_index = -1;

        switch (vertex.type) {
        case vertex_descriptor::NORMAL:
            vertex_index = vertex.x - m_x + (vertex.y - m_y) * m_width;
            break;
        case vertex_descriptor::LABEL_A:
            vertex_index = m_numVertices - 2;
            break;
        case vertex_descriptor::LABEL_B:
            vertex_index = m_numVertices - 1;
            break;
        }

        return vertex_index;
    }

    // Returns the vertex whose index is [vertex_index] (See also
    // index_of(vertex_descriptor))
    vertex_descriptor vertex_at (vertices_size_type vertex_index) const {
        vertex_descriptor vertex;

        if (vertex_index == m_numVertices - 2) {
            vertex.type = vertex_descriptor::LABEL_A;
        } else if (vertex_index == m_numVertices - 1) {
            vertex.type = vertex_descriptor::LABEL_B;
        } else if (vertex_index >= 0) {
            vertex.x = vertex_index % m_width + m_x;
            vertex.y = vertex_index / m_width + m_y;
            vertex.type = vertex_descriptor::NORMAL;
        }

      return vertex;
    }

    // Returns the edge whose index is [edge_index] (See also
    // index_of(edge_descriptor)).  NOTE: The index mapping is
    // dependent upon dimension wrapping.
    edge_descriptor edge_at(edges_size_type edge_index) const {

        int binIndex = 0;

        while (binIndex < m_edgeBins.size() &&
               !m_edgeBins[binIndex].contains(edge_index)) {

            binIndex++;
        }

        if (binIndex >= m_edgeBins.size()) {
            return edge_descriptor();
        }

        return m_edgeBins[binIndex].edgeAt(edge_index);
    }

    // Returns the index for [edge] (See also edge_at)
    edges_size_type index_of(edge_descriptor edge) const {
        edges_size_type index = -1;

        auto it = m_edgeBins.constBegin();
        for (; it != m_edgeBins.constEnd(); ++it) {

            index = it->indexOf(edge);
            if (index >= 0) break;
        }

        return index;
    }

private:
    static vertices_size_type numVacantEdges(const vertex_descriptor &vertex, const QRect &rc) {
        vertices_size_type vacantEdges = 4;

        if (vertex.x == rc.x()) {
            vacantEdges--;
        }

        if (vertex.y == rc.y()) {
            vacantEdges--;
        }

        if (vertex.x == rc.right()) {
            vacantEdges--;
        }

        if (vertex.y == rc.bottom()) {
            vacantEdges--;
        }

        return vacantEdges;
    }

    static inline bool findInRects(const QVector<QRect> &rects, const QPoint &pt) {
        bool result = false;

        auto it = rects.constBegin();
        for (; it != rects.constEnd(); ++it) {

            if (it->contains(pt)) {
                result = true;
                break;
            }
        }
        return result;
    }
public:
    // Returns the number of out-edges for [vertex]
    degree_size_type out_degree(vertex_descriptor vertex) const {
        degree_size_type out_edge_count = 0;
        if (index_of(vertex) < 0) return out_edge_count;

        switch (vertex.type) {
        case vertex_descriptor::NORMAL: {
            out_edge_count = numVacantEdges(vertex, m_mainArea);

            const QPoint pt = QPoint(vertex.x, vertex.y);

            if (m_aLabelArea.contains(pt) && findInRects(m_aLabelRects, pt)) {
                out_edge_count++;
            }

            if (m_bLabelArea.contains(pt) && findInRects(m_bLabelRects, pt)) {
                out_edge_count++;
            }

            break;
        }
        case vertex_descriptor::LABEL_A:
            out_edge_count = m_numAEdges;
            break;
        case vertex_descriptor::LABEL_B:
            out_edge_count = m_numBEdges;
            break;
        }

        return (out_edge_count);
    }

    // Returns an out-edge for [vertex] by index. Indices are in the
    // range [0, out_degree(vertex)).
    edge_descriptor out_edge_at (vertex_descriptor vertex,
                                 degree_size_type out_edge_index) const {

        const QPoint pt = QPoint(vertex.x, vertex.y);
        vertex_descriptor dst_vertex = vertex;

        switch (vertex.type) {
        case vertex_descriptor::NORMAL:
            if (vertex.x > m_mainArea.x() && !out_edge_index--) {
                dst_vertex.x--;
            } else if (vertex.y > m_mainArea.y() && !out_edge_index--) {
                dst_vertex.y--;
            } else if (vertex.x < m_mainArea.right() && !out_edge_index--) {
                dst_vertex.x++;
            } else if (vertex.y < m_mainArea.bottom() && !out_edge_index--) {
                dst_vertex.y++;
            } else if (m_aLabelArea.contains(pt) && findInRects(m_aLabelRects, pt) && !out_edge_index--) {
                dst_vertex = vertex_descriptor(0, 0, vertex_descriptor::LABEL_A);
            } else if (m_bLabelArea.contains(pt) && findInRects(m_bLabelRects, pt) && !out_edge_index--) {
                dst_vertex = vertex_descriptor(0, 0, vertex_descriptor::LABEL_B);
            } else {
                dbgImage << ppVar(vertex) << ppVar(out_edge_index) << ppVar(out_degree(vertex));
                qFatal("Wrong edge sub-index");
            }
            break;
        case vertex_descriptor::LABEL_A: {
            edge_descriptor edge = edge_at(m_aReversedEdgesStart + out_edge_index);
            dst_vertex = edge.second;
            break;
        }
        case vertex_descriptor::LABEL_B: {
            edge_descriptor edge = edge_at(m_bReversedEdgesStart + out_edge_index);
            dst_vertex = edge.second;
            break;
        }
        }

        return std::make_pair(vertex, dst_vertex);
    }

public:

    //================
    // VertexListGraph
    //================

    friend inline std::pair<typename type::vertex_iterator,
                            typename type::vertex_iterator>
    vertices(const type& graph) {
        typedef typename type::vertex_iterator vertex_iterator;
        typedef typename type::vertex_function vertex_function;
        typedef typename type::vertex_index_iterator vertex_index_iterator;

        return (std::make_pair
                (vertex_iterator(vertex_index_iterator(0),
                                 vertex_function(&graph)),
                 vertex_iterator(vertex_index_iterator(graph.num_vertices()),
                                 vertex_function(&graph))));
    }

    friend inline typename type::vertices_size_type
    num_vertices(const type& graph) {
        return (graph.num_vertices());
    }

    friend inline typename type::vertex_descriptor
    vertex(typename type::vertices_size_type vertex_index,
           const type& graph) {

        return (graph.vertex_at(vertex_index));
    }

    //===============
    // IncidenceGraph
    //===============

    friend inline std::pair<typename type::out_edge_iterator,
                            typename type::out_edge_iterator>
    out_edges(typename type::vertex_descriptor vertex,
              const type& graph) {
        typedef typename type::degree_iterator degree_iterator;
        typedef typename type::out_edge_function out_edge_function;
        typedef typename type::out_edge_iterator out_edge_iterator;

        return (std::make_pair
                (out_edge_iterator(degree_iterator(0),
                                   out_edge_function(vertex, &graph)),
                 out_edge_iterator(degree_iterator(graph.out_degree(vertex)),
                                   out_edge_function(vertex, &graph))));
    }

    friend inline typename type::degree_size_type
    out_degree
    (typename type::vertex_descriptor vertex,
     const type& graph) {
        return (graph.out_degree(vertex));
    }

    friend inline typename type::edge_descriptor
    out_edge_at(typename type::vertex_descriptor vertex,
                typename type::degree_size_type out_edge_index,
                const type& graph) {
        return (graph.out_edge_at(vertex, out_edge_index));
    }

    //===============
    // AdjacencyGraph
    //===============

    friend typename std::pair<typename type::adjacency_iterator,
                              typename type::adjacency_iterator>
    adjacent_vertices (typename type::vertex_descriptor vertex,
                       const type& graph) {
      typedef typename type::degree_iterator degree_iterator;
      typedef typename type::adjacent_vertex_function adjacent_vertex_function;
      typedef typename type::adjacency_iterator adjacency_iterator;

      return (std::make_pair
              (adjacency_iterator(degree_iterator(0),
                                 adjacent_vertex_function(vertex, &graph)),
               adjacency_iterator(degree_iterator(graph.out_degree(vertex)),
                                 adjacent_vertex_function(vertex, &graph))));
    }

    //==================
    // Adjacency Matrix
    //==================

    friend std::pair<typename type::edge_descriptor, bool>
    edge (typename type::vertex_descriptor source_vertex,
          typename type::vertex_descriptor destination_vertex,
          const type& graph) {

        std::pair<typename type::edge_descriptor, bool> edge_exists =
            std::make_pair(std::make_pair(source_vertex, destination_vertex), false);

        const edges_size_type index = graph.index_of(edge_exists.first);

        edge_exists.second = index >= 0;

        return edge_exists;
    }

    //==============
    // EdgeListGraph
    //==============

    friend inline typename type::edges_size_type
    num_edges(const type& graph) {
      return (graph.num_edges());
    }

    friend inline typename type::edge_descriptor
    edge_at(typename type::edges_size_type edge_index,
            const type& graph) {
      return (graph.edge_at(edge_index));
    }

    friend inline std::pair<typename type::edge_iterator,
                            typename type::edge_iterator>
    edges(const type& graph) {
      typedef typename type::edge_index_iterator edge_index_iterator;
      typedef typename type::edge_function edge_function;
      typedef typename type::edge_iterator edge_iterator;

      return (std::make_pair
              (edge_iterator(edge_index_iterator(0),
                             edge_function(&graph)),
               edge_iterator(edge_index_iterator(graph.num_edges()),
                             edge_function(&graph))));
    }

    //=============================
    // Index Property Map Functions
    //=============================

    friend inline typename type::vertices_size_type
    get(vertex_index_t,
        const type& graph,
        typename type::vertex_descriptor vertex) {

        type::vertices_size_type index = graph.index_of(vertex);
        LF_SANITY_ASSERT(index >= 0);
        return index;
    }

    friend inline typename type::edges_size_type
    get(edge_index_t,
        const type& graph,
        typename type::edge_descriptor edge) {

        type::edges_size_type index = graph.index_of(edge);
        LF_SANITY_ASSERT(index >= 0);
        return index;
    }

    friend inline lazy_fill_graph_index_map<
        type,
        typename type::vertex_descriptor,
        typename type::vertices_size_type>
    get(vertex_index_t, const type& graph) {
        return (lazy_fill_graph_index_map<
                type,
                typename type::vertex_descriptor,
                typename type::vertices_size_type>(graph));
    }

    friend inline lazy_fill_graph_index_map<
        type,
        typename type::edge_descriptor,
        typename type::edges_size_type>
    get(edge_index_t, const type& graph) {
        return (lazy_fill_graph_index_map<
                type,
                typename type::edge_descriptor,
                typename type::edges_size_type>(graph));
    }

    friend inline lazy_fill_graph_reverse_edge_map<
        typename type::edge_descriptor>
    get(edge_reverse_t, const type& graph) {
        Q_UNUSED(graph);
        return (lazy_fill_graph_reverse_edge_map<
                typename type::edge_descriptor>());
    }

    template<typename Graph,
             typename Descriptor,
             typename Index>
    friend struct lazy_fill_graph_index_map;

    template<typename Descriptor>
    friend struct lazy_fill_graph_reverse_edge_map;
};

namespace boost {
    template <>
    struct property_map<KisLazyFillGraph, vertex_index_t> {
        typedef lazy_fill_graph_index_map<KisLazyFillGraph,
                                          typename graph_traits<KisLazyFillGraph>::vertex_descriptor,
                                          typename graph_traits<KisLazyFillGraph>::vertices_size_type> type;
        typedef type const_type;
    };

    template<>
    struct property_map<KisLazyFillGraph, edge_index_t> {
        typedef lazy_fill_graph_index_map<KisLazyFillGraph,
                                          typename graph_traits<KisLazyFillGraph>::edge_descriptor,
                                          typename graph_traits<KisLazyFillGraph>::edges_size_type> type;
        typedef type const_type;
    };
}

namespace boost {
    template <>
    struct property_map<KisLazyFillGraph, edge_reverse_t> {
        typedef lazy_fill_graph_reverse_edge_map<typename graph_traits<KisLazyFillGraph>::edge_descriptor> type;
        typedef type const_type;
    };
}

QDebug operator<<(QDebug dbg, const KisLazyFillGraph::vertex_descriptor &v) {
    const QString type =
        v.type == KisLazyFillGraph::vertex_descriptor::NORMAL ? "normal" :
        v.type == KisLazyFillGraph::vertex_descriptor::LABEL_A ? "label_A" :
        v.type == KisLazyFillGraph::vertex_descriptor::LABEL_B ? "label_B" : "<unknown>";

    dbg.nospace() << "(" << v.x << ", " << v.y << ", " << type << ")";
    return dbg.space();
}

QDebug operator<<(QDebug dbg, const KisLazyFillGraph::edge_descriptor &e) {
    KisLazyFillGraph::vertex_descriptor src = e.first;
    KisLazyFillGraph::vertex_descriptor dst = e.second;

    dbg.nospace() << "(" << src << " -> " << dst << ")";
    return dbg.space();
}

#endif /* __KIS_LAZY_FILL_GRAPH_H */
