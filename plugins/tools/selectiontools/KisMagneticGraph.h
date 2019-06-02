/*
 *  Copyright (c) 2019 Kuntal Majumder <hellozee@disroot.org>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2.1 of the License.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KISMAGNETICGRAPH_H
#define KISMAGNETICGRAPH_H

#include <boost/limits.hpp>
#include <boost/operators.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/properties.hpp>
#include <boost/iterator/counting_iterator.hpp>
#include <boost/iterator/transform_iterator.hpp>

#include <QDebug>


// vertex_at
template <typename Graph>
struct magnetic_graph_vertex_at {

    typedef typename boost::graph_traits<Graph>::vertex_descriptor result_type;

    magnetic_graph_vertex_at() : m_graph(0) {}

    magnetic_graph_vertex_at(const Graph* graph) :
        m_graph(graph) { }

    result_type
    operator() (typename boost::graph_traits<Graph>::vertices_size_type vertex_index) const {
        return (vertex(vertex_index, *m_graph));
    }

private:
    const Graph* m_graph;
};

// out_edge_at
template <typename Graph>
struct magnetic_graph_out_edge_at {

private:
    typedef typename boost::graph_traits<Graph>::vertex_descriptor vertex_descriptor;

public:
    typedef typename boost::graph_traits<Graph>::edge_descriptor result_type;

    magnetic_graph_out_edge_at() :
        m_vertex(), m_graph(0)
    { }

    magnetic_graph_out_edge_at(vertex_descriptor source_vertex, const Graph* graph) :
        m_vertex(source_vertex), m_graph(graph)
    { }

    result_type
    operator() (typename boost::graph_traits<Graph>::degree_size_type out_edge_index) const {
        return (out_edge_at(m_vertex, out_edge_index, *m_graph));
    }

private:
    vertex_descriptor m_vertex;
    const Graph* m_graph;
};

// in_edge_at
template <typename Graph>
struct magnetic_graph_in_edge_at {

private:
    typedef typename boost::graph_traits<Graph>::vertex_descriptor vertex_descriptor;

public:
    typedef typename boost::graph_traits<Graph>::edge_descriptor result_type;

    magnetic_graph_in_edge_at() :
        m_vertex(), m_graph(0)
    { }

    magnetic_graph_in_edge_at(vertex_descriptor target_vertex, const Graph* graph) :
        m_vertex(target_vertex), m_graph(graph)
    { }

    result_type
    operator() (typename boost::graph_traits<Graph>::degree_size_type in_edge_index) const {
        return (in_edge_at(m_vertex, in_edge_index, *m_graph));
    }

private:
    vertex_descriptor m_vertex;
    const Graph* m_graph;
};

// edge_at
template <typename Graph>
struct magnetic_graph_edge_at {

    typedef typename boost::graph_traits<Graph>::edge_descriptor result_type;

    magnetic_graph_edge_at () :
        m_graph(0)
    { }

    magnetic_graph_edge_at (const Graph* graph) :
        m_graph(graph)
    { }

    result_type operator()
    (typename boost::graph_traits<Graph>::edges_size_type edge_index) const {
        return (edge_at(edge_index, *m_graph));
    }

private:
    const Graph* m_graph;
};

// adjacent_vertex_at
template <typename Graph>
struct magnetic_graph_adjacent_vertex_at {

public:
    typedef typename boost::graph_traits<Graph>::vertex_descriptor result_type;

    magnetic_graph_adjacent_vertex_at
    (result_type source_vertex, const Graph* graph) :
        m_vertex(source_vertex), m_graph(graph)
    { }

    result_type operator()
    (typename boost::graph_traits<Graph>::degree_size_type adjacent_index) const {
        return (target(out_edge_at(m_vertex, adjacent_index, *m_graph), *m_graph));
    }

private:
    result_type m_vertex;
    const Graph* m_graph;
};


class KisMagneticGraph{
public:
    typedef KisMagneticGraph type;


    typedef long VertexIndex;
    typedef long EdgeIndex;

    typedef VertexIndex vertices_size_type;
    typedef EdgeIndex edges_size_type;
    typedef EdgeIndex degree_size_type;

    struct VertexDescriptor : public boost::equality_comparable<VertexDescriptor>{
        vertices_size_type x,y;

        VertexDescriptor(vertices_size_type _x, vertices_size_type _y):
            x(_x),y(_y)
        { }

        VertexDescriptor():
            x(0),y(0)
        { }

        bool operator ==(const VertexDescriptor &rhs) const {
            return rhs.x == x && rhs.y == y;
        }
    };

    typedef VertexDescriptor vertex_descriptor;
    typedef std::pair<vertex_descriptor, vertex_descriptor> edge_descriptor;

    friend QDebug operator<<(QDebug dbg, const KisMagneticGraph::edge_descriptor &e);
    friend QDebug operator<<(QDebug dbg, const KisMagneticGraph::vertex_descriptor &v);

    typedef boost::directed_tag directed_category;
    typedef boost::disallow_parallel_edge_tag edge_parallel_category;
    struct traversal_category : virtual public boost::incidence_graph_tag,
                                virtual public boost::vertex_list_graph_tag
    { };

    KisMagneticGraph() { }
};

QDebug operator<<(QDebug dbg, const KisMagneticGraph::vertex_descriptor &v) {

    dbg.nospace() << "(" << v.x << ", " << v.y << ")";
    return dbg.space();
}

QDebug operator<<(QDebug dbg, const KisMagneticGraph::edge_descriptor &e) {
    KisMagneticGraph::vertex_descriptor src = e.first;
    KisMagneticGraph::vertex_descriptor dst = e.second;

    dbg.nospace() << "(" << src << " -> " << dst << ")";
    return dbg.space();
}

#endif
