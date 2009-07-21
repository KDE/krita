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

#include "KoFilterEdge.h"
#include "KoFilterVertex.h"
#include "PriorityQueue_p.h"

namespace KOfficeFilter {

Edge::Edge(Vertex* vertex, KoFilterEntry::Ptr filterEntry) :
        m_vertex(vertex), m_filterEntry(filterEntry), d(0)
{
}

void Edge::relax(const Vertex* predecessor, PriorityQueue<Vertex>& queue)
{
    if (!m_vertex || !predecessor || !m_filterEntry)
        return;
    if (m_vertex->setKey(predecessor->key() + m_filterEntry->weight)) {
        queue.keyDecreased(m_vertex);   // maintain the heap property
        m_vertex->setPredecessor(predecessor);
    }
}

void Edge::dump(const QByteArray& indent) const
{
    if (m_vertex)
        kDebug(30500) << indent << "Edge -> '" << m_vertex->mimeType()
        << "' (" << m_filterEntry->weight << ")" << endl;
    else
        kDebug(30500) << indent << "Edge -> '(null)' ("
        << m_filterEntry->weight << ")" << endl;
}

};
