/* This file is part of the Calligra libraries
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
#include "KisFilterGraph.h"

#include "KisImportExportManager.h"  // KisImportExportManager::filterAvailable, private API
#include "KisDocumentEntry.h"
#include "KisFilterEntry.h"
#include "KisDocument.h"

#include "PriorityQueue_p.h"
#include "KisFilterEdge.h"
#include "KisFilterChainLink.h"
#include "KisFilterVertex.h"

#include <QMetaMethod>
#include <QTemporaryFile>
#include <QPluginLoader>
#include <kis_debug.h>

#include <limits.h> // UINT_MAX


namespace CalligraFilter {

Graph::Graph(const QByteArray& from)
    : m_from(from)
    , m_graphValid(false)
    , d(0)
{
    buildGraph();
    shortestPaths();  // Will return after a single lookup if "from" is invalid (->no check here)
}

Graph::~Graph()
{
    Q_FOREACH (Vertex* vertex, m_vertices) {
        delete vertex;
    }
    m_vertices.clear();
}

void Graph::setSourceMimeType(const QByteArray& from)
{
    if (from == m_from)
        return;
    m_from = from;
    m_graphValid = false;

    // Initialize with "infinity" ...
    Q_FOREACH (Vertex* vertex, m_vertices) {
        vertex->reset();
    }
    // ...and re-run the shortest path search for the new source mime
    shortestPaths();
}

KisFilterChainSP Graph::chain(const KisImportExportManager* manager, QByteArray& to) const
{
    if (!isValid() || !manager)
        return KisFilterChainSP();

    Q_ASSERT(!to.isEmpty());

    const Vertex* vertex = m_vertices.value(to);
    if (!vertex || vertex->key() == UINT_MAX)
        return KisFilterChainSP();

    KisFilterChainSP ret(new KisFilterChain(manager));

    // Fill the filter chain with all filters on the path
    const Vertex* tmp = vertex->predecessor();
    while (tmp) {
        const Edge* const edge = tmp->findEdge(vertex);
        Q_ASSERT(edge);
        ret->prependChainLink(edge->filterEntry(), tmp->mimeType(), vertex->mimeType());
        vertex = tmp;
        tmp = tmp->predecessor();
    }
    return ret;
}

void Graph::dump() const
{
#ifndef NDEBUG
    dbgFile << "+++++++++ Graph::dump +++++++++";
    dbgFile << "From:" << m_from;
    Q_FOREACH (Vertex *vertex, m_vertices) {
        vertex->dump("   ");
    }
    dbgFile << "+++++++++ Graph::dump (done) +++++++++";
#endif
}

// Query the trader and create the vertices and edges representing
// available mime types and filters.
void Graph::buildGraph()
{
    // no constraint here - we want *all* :)
    const QList<KisFilterEntrySP> filters(KisFilterEntry::query());

    Q_FOREACH (KisFilterEntrySP filter, filters) {

        // First add the "starting points" to the dict
        Q_FOREACH (const QString& import, filter->import) {
            const QByteArray key = import.toLatin1();    // latin1 is okay here (werner)
            // already there?
            if (!m_vertices.contains(key))
                m_vertices.insert(key, new Vertex(key));
        }

        Q_FOREACH (const QString& exportIt, filter->export_) {

            // First make sure the export vertex is in place
            const QByteArray key = exportIt.toLatin1();    // latin1 is okay here
            Vertex* exp = m_vertices.value(key);
            if (!exp) {
                exp = new Vertex(key);
                m_vertices.insert(key, exp);
            }
            // Then create the appropriate edges
            Q_FOREACH (const QString& import, filter->import) {
                m_vertices[import.toLatin1()]->addEdge(new Edge(exp, filter));
            }

        }
    }
}

// As all edges (=filters) are required to have a positive weight
// we can use Dijkstra's shortest path algorithm from Cormen's
// "Introduction to Algorithms" (p. 527)
// Note: I did some adaptions as our data structures are slightly
// different from the ones used in the book. Further we simply stop
// the algorithm is we don't find any node with a weight != Infinity
// (==UINT_MAX), as this means that the remaining nodes in the queue
// aren't connected anyway.
void Graph::shortestPaths()
{
    // Is the requested start mime type valid?
    Vertex* from = m_vertices.value(m_from);
    if (!from)
        return;

    // Inititalize start vertex
    from->setKey(0);

    // Fill the priority queue with all the vertices
    PriorityQueue<Vertex> queue(m_vertices);

    while (!queue.isEmpty()) {
        Vertex *min = queue.extractMinimum();
        // Did we already relax all connected vertices?
        if (min->key() == UINT_MAX)
            break;
        min->relaxVertices(queue);
    }
    m_graphValid = true;
}

}
