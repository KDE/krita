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

};

#endif
