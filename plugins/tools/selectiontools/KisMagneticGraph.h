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

#include <boost/operators.hpp>
#include <boost/graph/graph_traits.hpp>

#include <QDebug>
#include <QRect>

struct VertexDescriptor : public boost::equality_comparable<VertexDescriptor>{

    long x,y;

    VertexDescriptor(long _x, long _y):
        x(_x), y(_y)
    { }

    VertexDescriptor(QPoint pt):
        x(pt.x()), y(pt.y())
    { }

    VertexDescriptor():
        x(0), y(0)
    { }

    bool operator ==(const VertexDescriptor &rhs) const {
        return rhs.x == x && rhs.y == y;
    }

    bool operator ==(const QPoint &rhs) const {
        return rhs.x() == x && rhs.y() == y;
    }
};

struct neighbour_iterator;

struct KisMagneticGraph{

    KisMagneticGraph() { }
    KisMagneticGraph(long _outDegree, QRect graphRect):
        outDegree(_outDegree), topLeft(graphRect.topLeft()), bottomRight(graphRect.bottomRight())
    { }

    typedef VertexDescriptor                                vertex_descriptor;
    typedef std::pair<vertex_descriptor, vertex_descriptor> edge_descriptor;
    typedef boost::undirected_tag                           directed_category;
    typedef boost::disallow_parallel_edge_tag               edge_parallel_category;
    typedef boost::incidence_graph_tag                      traversal_category;
    typedef neighbour_iterator                              out_edge_iterator;
    typedef long                                            degree_size_type;

    degree_size_type outDegree;
    QPoint topLeft, bottomRight;
};

struct neighbour_iterator : public boost::iterator_facade<neighbour_iterator,
                               std::pair<VertexDescriptor, VertexDescriptor>,
                                                boost::forward_traversal_tag,
                               std::pair<VertexDescriptor, VertexDescriptor>>
{
    neighbour_iterator(VertexDescriptor v, KisMagneticGraph g):
        currentPoint(v), graph(g)
    {
        nextPoint = VertexDescriptor(g.topLeft.x(), g.topLeft.y());
        if(nextPoint == currentPoint){
            operator++();
        }
    }

    std::pair<VertexDescriptor, VertexDescriptor>
    operator*() const {
        std::pair const result = std::make_pair(currentPoint,nextPoint);
        return result;
    }

    void operator++() {
        if(nextPoint == graph.bottomRight)
            return; // we are done, no more points

        if(nextPoint.x == graph.bottomRight.x()) // end of a row move to next column
            nextPoint = VertexDescriptor(graph.topLeft.x(), nextPoint.y++);
        else
            nextPoint.x++;
    }

    bool operator==(neighbour_iterator const& that) const {
        return point == that.point;
    }

    bool equal(neighbour_iterator const& that) const {
        return operator==(that);
    }

    void increment() {
        operator++();
    }

private:
    KisMagneticGraph graph;
    VertexDescriptor currentPoint, nextPoint;
};

namespace boost{
template<>
struct graph_traits<KisMagneticGraph> {
    typedef KisMagneticGraph type;
    typedef typename type::vertex_descriptor      vertex_descriptor;
    typedef typename type::edge_descriptor        edge_descriptor;
    typedef typename type::out_edge_iterator      out_edge_iterator;
    typedef typename type::directed_category      directed_category;
    typedef typename type::edge_parallel_category edge_parallel_category;
    typedef typename type::traversal_category     traversal_category;
    typedef typename type::degree_size_type       degree_size_type;

    typedef void in_edge_iterator;
    typedef void vertex_iterator;
    typedef void vertices_size_type;
    typedef void edge_iterator;
    typedef void edges_size_type;
};
}

// Requirements for an Incidence Graph,
// https://www.boost.org/doc/libs/1_70_0/libs/graph/doc/IncidenceGraph.html

typename KisMagneticGraph::vertex_descriptor
source(typename KisMagneticGraph::edge_descriptor e, KisMagneticGraph g) {
    return e.first;
}

typename KisMagneticGraph::vertex_descriptor
target(typename KisMagneticGraph::edge_descriptor e, KisMagneticGraph g) {
    return e.second;
}

std::pair<KisMagneticGraph::out_edge_iterator, KisMagneticGraph::out_edge_iterator>
out_edges(typename KisMagneticGraph::vertex_descriptor v, KisMagneticGraph g) {
    return std::make_pair(
                KisMagneticGraph::out_edge_iterator(v),
                KisMagneticGraph::out_edge_iterator(v)
                );
}

typename KisMagneticGraph::degree_size_type
out_degree(typename KisMagneticGraph::vertex_descriptor v, KisMagneticGraph g) {
    return g.outDegree;
}

QDebug operator<<(QDebug dbg, const KisMagneticGraph::vertex_descriptor &v) {
    dbg.nospace() << "(" << v.x << ", " << v.y << ")";
    return dbg.space();
}

#endif
