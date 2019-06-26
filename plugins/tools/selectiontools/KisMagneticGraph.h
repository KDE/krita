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
#include <kis_random_accessor_ng.h>

#include <QDebug>
#include <QRect>
#include <QColor>

struct VertexDescriptor {

    long x,y;

    enum Direction {
        MIN = 0,
        N = MIN, S, E, W, NW, NE, SW, SE, NONE
    };

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

    // returns one of the 8 neighboring pixel based on the direction
    // it gives out multiple warnings, but I am lazy, sorry
    VertexDescriptor neighbor(Direction direction) const {

        int dx = 0, dy = 0;

        switch (direction){
        case W:
        case SW:
        case NW:
            dx = -1;
            break;
        case E:
        case SE:
        case NE:
            dx = 1;
        }

        switch(direction){
        case N:
        case NW:
        case NE:
            dy = -1;
            break;
        case S:
        case SW:
        case SE:
            dy = 1;
        }

        VertexDescriptor const neighbor(x + dx, y + dy);
        return neighbor;
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
        m_rect(graphRect), m_dev(dev)
    {
        m_randAccess = m_dev->createRandomAccessorNG(m_dev->exactBounds().x(), m_dev->exactBounds().y());
    }

    typedef VertexDescriptor                                vertex_descriptor;
    typedef std::pair<vertex_descriptor, vertex_descriptor> edge_descriptor;
    typedef boost::undirected_tag                           directed_category;
    typedef boost::disallow_parallel_edge_tag               edge_parallel_category;
    typedef boost::incidence_graph_tag                      traversal_category;
    typedef neighbour_iterator                              out_edge_iterator;
    typedef unsigned                                        degree_size_type;


    double getIntensity(VertexDescriptor pt) {
        m_randAccess->moveTo(pt.x, pt.y);
        qint8 val = *(m_randAccess->rawData());
        //offsetting the value, so we don't get negatives
        return val + 255;
    }

    unsigned outDegree(VertexDescriptor pt){
        //corners
        if(pt == m_rect.topLeft() || pt == m_rect.topRight() ||
                pt == m_rect.bottomLeft() || pt == m_rect.bottomRight()){
            if(m_rect.width() == 1 || m_rect.height() == 1)
                return 1;
            return 3;
        }

        //edges
        if(pt.x == m_rect.topLeft().x() || pt.y == m_rect.topLeft().y()  ||
                pt.x == m_rect.bottomRight().x() || pt.y == m_rect.bottomRight().y()){
            if(m_rect.width() == 1 || m_rect.height() == 1)
                return 2;
            return 5;
        }
        return 8;
    }

    QRect m_rect;

private:
    KisPaintDeviceSP m_dev;
    KisRandomAccessorSP m_randAccess;
};

struct neighbour_iterator : public boost::iterator_facade<neighbour_iterator,
                               std::pair<VertexDescriptor, VertexDescriptor>,
                                                boost::forward_traversal_tag,
                               std::pair<VertexDescriptor, VertexDescriptor>>
{

    neighbour_iterator(VertexDescriptor v, KisMagneticGraph g, VertexDescriptor::Direction d):
        m_point(v), m_direction(d), m_graph(g)
    { }

    neighbour_iterator()
    { }

    std::pair<VertexDescriptor, VertexDescriptor>
    operator*() const {
        std::pair<VertexDescriptor, VertexDescriptor> const result = std::make_pair(m_point, m_point.neighbor(m_direction));
        return result;
    }

    void operator++() {
        m_direction = static_cast<VertexDescriptor::Direction>(int(m_direction)+1);
        VertexDescriptor next = m_point.neighbor(m_direction);
        if(m_direction == VertexDescriptor::NONE){
            return;
        }
        if(!m_graph.m_rect.contains(next.x, next.y)){
            operator++();
        }
    }

    bool operator==(neighbour_iterator const& that) const {
        return m_point == that.m_point && m_direction == that.m_direction;
    }

    bool equal(neighbour_iterator const& that) const {
        return operator==(that);
    }

    void increment() {
        operator++();
    }

private:
    VertexDescriptor m_point;
    VertexDescriptor::Direction m_direction;
    KisMagneticGraph m_graph;
};

// Requirements for an Incidence Graph,
// https://www.boost.org/doc/libs/1_70_0/libs/graph/doc/IncidenceGraph.html

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
                KisMagneticGraph::out_edge_iterator(v, g, VertexDescriptor::Direction::MIN),
                KisMagneticGraph::out_edge_iterator(v, g, VertexDescriptor::Direction::NONE)
                );
}

typename KisMagneticGraph::degree_size_type
out_degree(typename KisMagneticGraph::vertex_descriptor v, KisMagneticGraph g) {
    return g.outDegree(v);
}

#endif
