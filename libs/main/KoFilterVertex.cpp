/* This file is part of the KOffice libraries
   Copyright (C) 2001 Werner Trobin <trobin@kde.org>

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
*/
#include "KoFilterVertex.h"
#include <limits.h> // UINT_MAX
#include "KoFilterEdge.h"

namespace KOfficeFilter {

Vertex::Vertex(const QByteArray& mimeType)
        : m_predecessor(0)
        , m_mimeType(mimeType)
        , m_weight(UINT_MAX)
        , m_index(-1)
        , d(0)
{
}

Vertex::~Vertex()
{
    qDeleteAll(m_edges);
}

bool Vertex::setKey(unsigned int key)
{
    if (m_weight > key) {
        m_weight = key;
        return true;
    }
    return false;
}

void Vertex::reset()
{
    m_weight = UINT_MAX;
    m_predecessor = 0;
}

void Vertex::addEdge(Edge* edge)
{
    if (!edge || edge->weight() == 0)
        return;
    m_edges.append(edge);
}

const Edge* Vertex::findEdge(const Vertex* vertex) const
{
    if (!vertex)
        return 0;
    const Edge* edge = 0;
    foreach(Edge* e, m_edges) {
        if (e->vertex() == vertex &&
            (!edge || e->weight() < edge->weight())) {
            edge = e;
        }
    }
    return edge;
}

void Vertex::relaxVertices(PriorityQueue<Vertex>& queue)
{
    foreach(Edge* e, m_edges) {
        e->relax(this, queue);
    }
}

void Vertex::dump(const QByteArray& indent) const
{
#ifndef NDEBUG
    kDebug(30500) << indent << "Vertex:" << m_mimeType << " (" << m_weight << "):";
    const QByteArray i(indent + "   ");
    foreach(Edge* edge, m_edges) {
        edge->dump(i);
    }
#endif
}

}
