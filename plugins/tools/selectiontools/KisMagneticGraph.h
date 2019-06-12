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
#include <kis_paint_device.h>

#include <QDebug>
#include <QRect>
#include <QColor>

struct VertexDescriptor {

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

    bool operator==(VertexDescriptor const &rhs) const {
        return rhs.x == x && rhs.y == y;
    }

    bool operator==(QPoint const &rhs) const {
        return rhs.x() == x && rhs.y() == y;
    }

    bool operator !=(VertexDescriptor const &rhs) const {
        return rhs.x != x || rhs.y != y;
    }

    bool operator<(VertexDescriptor const &rhs) const {
        return x < rhs.x || (x == rhs.x && y < rhs.y);
    }
};

QDebug operator<<(QDebug dbg, const VertexDescriptor &v) {
    dbg.nospace() << "(" << v.x << ", " << v.y << ")";
    return dbg.space();
}

struct neighbour_iterator;

struct KisMagneticGraph{

    typedef KisMagneticGraph type;

    KisMagneticGraph() { }
    KisMagneticGraph(KisPaintDeviceSP dev, QRect graphRect):
        topLeft(graphRect.topLeft()), bottomRight(graphRect.bottomRight()), m_dev(dev)
    {
        outDegree = (bottomRight.y() - topLeft.y()) * (bottomRight.x() - topLeft.x()) - 1;
        qDebug() << outDegree;
    }

    typedef VertexDescriptor                                vertex_descriptor;
    typedef std::pair<vertex_descriptor, vertex_descriptor> edge_descriptor;
    typedef boost::undirected_tag                           directed_category;
    typedef boost::disallow_parallel_edge_tag               edge_parallel_category;
    typedef boost::incidence_graph_tag                      traversal_category;
    typedef neighbour_iterator                              out_edge_iterator;
    typedef unsigned long                                   degree_size_type;

    degree_size_type outDegree;
    QPoint topLeft, bottomRight;

    double getIntensity(VertexDescriptor pt){
        QColor *col = new QColor;
        m_dev->pixel(pt.x, pt.y, col);
        double intensity = col->blackF();
        delete col;
        return intensity;
    }

private:
    KisPaintDeviceSP m_dev;
};

struct neighbour_iterator : public boost::iterator_facade<neighbour_iterator,
                               std::pair<VertexDescriptor, VertexDescriptor>,
                                                boost::forward_traversal_tag,
                               std::pair<VertexDescriptor, VertexDescriptor>>
{
    enum position{
        begin, end
    };

    neighbour_iterator(VertexDescriptor v, KisMagneticGraph g, position p):
        graph(g), currentPoint(v), pos(p)
    {
        nextPoint = VertexDescriptor(g.topLeft.x(), g.topLeft.y());
        if(nextPoint == currentPoint){
            increment();
        }
    }

    neighbour_iterator()
    { }

    std::pair<VertexDescriptor, VertexDescriptor>
    operator*() const {
        std::pair<VertexDescriptor, VertexDescriptor> const result = std::make_pair(currentPoint,nextPoint);
        return result;
    }

    void operator++() {
        // I am darn sure that Dmitry is wrong, definitely wrong
        if(nextPoint == graph.bottomRight){
            pos = position::end;
            return;
        }
        if(nextPoint.x == graph.bottomRight.x()){ // end of a row move to next column
            nextPoint = VertexDescriptor(graph.topLeft.x(), nextPoint.y + 1);
        } else {
            nextPoint.x++;
        }

        if(nextPoint == currentPoint){
            increment();
        }

    }

    bool operator==(neighbour_iterator const& that) const {
        return pos == that.pos;
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
    position pos;
};

namespace boost{
template<>
struct graph_traits<KisMagneticGraph> {
    typedef typename KisMagneticGraph::vertex_descriptor      vertex_descriptor;
    typedef typename KisMagneticGraph::edge_descriptor        edge_descriptor;
    typedef typename KisMagneticGraph::out_edge_iterator      out_edge_iterator;
    typedef typename KisMagneticGraph::directed_category      directed_category;
    typedef typename KisMagneticGraph::edge_parallel_category edge_parallel_category;
    typedef typename KisMagneticGraph::traversal_category     traversal_category;
    typedef typename KisMagneticGraph::degree_size_type       degree_size_type;

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
    Q_UNUSED(g)
    return e.first;
}

typename KisMagneticGraph::vertex_descriptor
target(typename KisMagneticGraph::edge_descriptor e, KisMagneticGraph g) {
    Q_UNUSED(g)
    return e.second;
}

std::pair<KisMagneticGraph::out_edge_iterator, KisMagneticGraph::out_edge_iterator>
out_edges(typename KisMagneticGraph::vertex_descriptor v, KisMagneticGraph g) {
    return std::make_pair(
                KisMagneticGraph::out_edge_iterator(v, g, neighbour_iterator::begin),
                KisMagneticGraph::out_edge_iterator(v, g, neighbour_iterator::end)
                );
}

typename KisMagneticGraph::degree_size_type
out_degree(typename KisMagneticGraph::vertex_descriptor v, KisMagneticGraph g) {
    Q_UNUSED(v)
    return g.outDegree;
}

#endif
