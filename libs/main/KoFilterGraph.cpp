/* This file is part of the KOffice libraries
   Copyright (C) 2001 Werner Trobin <trobin@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/
#include "KoFilterGraph.h"

#include "KoFilterManager.h"  // KoFilterManager::filterAvailable, private API
#include "KoDocumentEntry.h"
#include "KoFilterEntry.h"
#include "KoDocument.h"

#include "PriorityQueue_p.h"
#include "KoFilterEdge.h"
#include "KoFilterChainLink.h"
#include "KoFilterVertex.h"

#include <QMetaMethod>
#include <ktemporaryfile.h>
#include <kmimetype.h>
#include <kdebug.h>

#include <limits.h> // UINT_MAX


namespace KOfficeFilter {

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
    foreach(Vertex* vertex, m_vertices) {
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
    foreach(Vertex* vertex, m_vertices) {
        vertex->reset();
    }
    // ...and re-run the shortest path search for the new source mime
    shortestPaths();
}

KoFilterChain::Ptr Graph::chain(const KoFilterManager* manager, QByteArray& to) const
{
    if (!isValid() || !manager)
        return KoFilterChain::Ptr();

    if (to.isEmpty()) {    // if the destination is empty we search the closest KOffice part
        to = findKOfficePart();
        if (to.isEmpty())    // still empty? strange stuff...
            return KoFilterChain::Ptr();
    }

    const Vertex* vertex = m_vertices[ to ];
    if (!vertex || vertex->key() == UINT_MAX)
        return KoFilterChain::Ptr();

    KoFilterChain::Ptr ret(new KoFilterChain(manager));

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
    kDebug(30500) << "+++++++++ Graph::dump +++++++++";
    kDebug(30500) << "From:" << m_from;
    foreach(Vertex vertex, m_vertices) {
        vertex->dump("   ");
    }
    kDebug(30500) << "+++++++++ Graph::dump (done) +++++++++";
#endif
}

// Query the trader and create the vertices and edges representing
// available mime types and filters.
void Graph::buildGraph()
{
    // Make sure that all available parts are added to the graph
    const QList<KoDocumentEntry> parts(KoDocumentEntry::query(KoDocumentEntry::AllEntries));
    QList<KoDocumentEntry>::ConstIterator partIt(parts.constBegin());
    QList<KoDocumentEntry>::ConstIterator partEnd(parts.constEnd());

    while (partIt != partEnd) {
        QStringList nativeMimeTypes = (*partIt).service()->property("X-KDE-ExtraNativeMimeTypes").toStringList();
        nativeMimeTypes += (*partIt).service()->property("X-KDE-NativeMimeType").toString();
        QStringList::ConstIterator it = nativeMimeTypes.constBegin();
        QStringList::ConstIterator end = nativeMimeTypes.constEnd();
        for (; it != end; ++it)
            if (!(*it).isEmpty())
                m_vertices.insert((*it).toLatin1(), new Vertex((*it).toLatin1()));
        ++partIt;
    }

    // no constraint here - we want *all* :)
    const QList<KoFilterEntry::Ptr> filters(KoFilterEntry::query());
    QList<KoFilterEntry::Ptr>::ConstIterator it = filters.constBegin();
    QList<KoFilterEntry::Ptr>::ConstIterator end = filters.constEnd();

    for (; it != end; ++it) {
        // First add the "starting points" to the dict
        QStringList::ConstIterator importIt = (*it)->import.constBegin();
        QStringList::ConstIterator importEnd = (*it)->import.constEnd();
        for (; importIt != importEnd; ++importIt) {
            const QByteArray key = (*importIt).toLatin1();    // latin1 is okay here (werner)
            // already there?
            if (!m_vertices[ key ])
                m_vertices.insert(key, new Vertex(key));
        }

        // Are we allowed to use this filter at all?
        if (KoFilterManager::filterAvailable(*it)) {
            QStringList::ConstIterator exportIt = (*it)->export_.constBegin();
            QStringList::ConstIterator exportEnd = (*it)->export_.constEnd();

            for (; exportIt != exportEnd; ++exportIt) {
                // First make sure the export vertex is in place
                const QByteArray key = (*exportIt).toLatin1();    // latin1 is okay here
                Vertex* exp = m_vertices[ key ];
                if (!exp) {
                    exp = new Vertex(key);
                    m_vertices.insert(key, exp);
                }
                // Then create the appropriate edges
                importIt = (*it)->import.constBegin();
                for (; importIt != importEnd; ++importIt)
                    m_vertices[(*importIt).toLatin1()]->addEdge(new Edge(exp, *it));
            }
        } else
            kDebug(30500) << "Filter:" << (*it)->service()->name() << " doesn't apply.";
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
    Vertex* from = m_vertices[ m_from ];
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

QByteArray Graph::findKOfficePart() const
{
    // Here we simply try to find the closest KOffice mimetype
    const QList<KoDocumentEntry> parts(KoDocumentEntry::query(KoDocumentEntry::AllEntries));
    QList<KoDocumentEntry>::ConstIterator partIt(parts.constBegin());
    QList<KoDocumentEntry>::ConstIterator partEnd(parts.constEnd());

    const Vertex *v = 0;

    // Be sure that v gets initialized correctly
    while (!v && partIt != partEnd) {
        QStringList nativeMimeTypes = (*partIt).service()->property("X-KDE-ExtraNativeMimeTypes").toStringList();
        nativeMimeTypes += (*partIt).service()->property("X-KDE-NativeMimeType").toString();
        QStringList::ConstIterator it = nativeMimeTypes.constBegin();
        QStringList::ConstIterator end = nativeMimeTypes.constEnd();
        for (; !v && it != end; ++it)
            if (!(*it).isEmpty())
                v = m_vertices[(*it).toLatin1()];
        ++partIt;
    }
    if (!v)
        return "";

    // Now we try to find the "cheapest" KOffice vertex
    while (partIt != partEnd) {
        QStringList nativeMimeTypes = (*partIt).service()->property("X-KDE-ExtraNativeMimeTypes").toStringList();
        nativeMimeTypes += (*partIt).service()->property("X-KDE-NativeMimeType").toString();
        QStringList::ConstIterator it = nativeMimeTypes.constBegin();
        QStringList::ConstIterator end = nativeMimeTypes.constEnd();
        for (; !v && it != end; ++it) {
            QString key = *it;
            if (!key.isEmpty()) {
                Vertex* tmp = m_vertices[ key.toLatin1()];
                if (!v || (tmp && tmp->key() < v->key()))
                    v = tmp;
            }
        }
        ++partIt;
    }

    // It seems it already is a KOffice part
    if (v->key() == 0)
        return "";

    return v->mimeType();
}
};
