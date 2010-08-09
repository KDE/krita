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
#ifndef KOFILTERCHAINLINK_H
#define KOFILTERCHAINLINK_H

#include <KoFilterChain.h>

class QByteArray;
class KoUpdater;

namespace KOfficeFilter {

/**
 * A small private helper class with represents one single filter
 * (one link of the chain)
 * @internal
 */
class ChainLink
{

public:
    ChainLink(KoFilterChain *chain, KoFilterEntry::Ptr filterEntry,
              const QByteArray& from, const QByteArray& to);

    ~ChainLink();

    KoFilter::ConversionStatus invokeFilter(const ChainLink * const parentChainLink);

    QByteArray from() const {
        return m_from;
    }
    QByteArray to() const {
        return m_to;
    }

    // debugging
    void dump() const;

    // This hack is only needed due to crappy Microsoft design and
    // circular dependencies in their embedded files :}
    int lruPartIndex() const;

    QPointer<KoUpdater> updater() const {
        return m_updater;
    }

private:
    ChainLink(const ChainLink& rhs);
    ChainLink& operator=(const ChainLink& rhs);

    void setupCommunication(const KoFilter * const parentFilter) const;
    void setupConnections(const KoFilter *sender, const KoFilter *receiver) const;

    KoFilterChain *m_chain;
    KoFilterEntry::Ptr m_filterEntry;
    QByteArray m_from, m_to;

    // This hack is only needed due to crappy Microsoft design and
    // circular dependencies in their embedded files :}
    KoFilter *m_filter;

    QPointer<KoUpdater> const m_updater;
};

}
#endif // KOFILTERCHAINLINK_H
