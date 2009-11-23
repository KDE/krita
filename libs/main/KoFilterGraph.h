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
#ifndef KOFILTERGRAPH_H
#define KOFILTERGRAPH_H

#include "komain_export.h"
#include "KoFilterChain.h"
#include "KoFilterVertex.h"
#include <QByteArray>
#include <QHash>

namespace KOfficeFilter {

/**
 * The main worker behind the scenes. Manages the creation of the graph,
 * processing the information in it, and creating the filter chains.
 * @internal
 * Only exported for unit tests.
 */
class KOMAIN_TEST_EXPORT Graph
{

public:
    explicit Graph(const QByteArray& from);
    ~Graph();

    bool isValid() const {
        return m_graphValid;
    }

    QByteArray sourceMimeType() const {
        return m_from;
    }
    void setSourceMimeType(const QByteArray& from);

    // Creates a chain from "from" to the "to" mimetype
    // If the "to" mimetype isEmpty() then we try to find the
    // closest KOffice mimetype and use that as destination.
    // After such a search "to" will contain the dest. mimetype (return value)
    // if the search was successful. Might return 0!
    KoFilterChain::Ptr chain(const KoFilterManager* manager, QByteArray& to) const;

    // debugging
    void dump() const;

private:
    Graph(const Graph& rhs);
    Graph& operator=(const Graph& rhs);

    void buildGraph();
    void shortestPaths();
    QByteArray findKOfficePart() const;

    QHash<QByteArray, KOfficeFilter::Vertex*> m_vertices;
    QByteArray m_from;
    bool m_graphValid;

    class Private;
    Private * const d;
};

}
#endif // KOFILTERGRAPH_H
