/* This file is part of the Calligra libraries
   Copyright (C) 2009 Boudewijn Rempt <boud@valdyas.org>

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
#include "KoFilterChainLinkList.h"
#include "KoFilterChainLink.h"

namespace CalligraFilter {

    ChainLinkList::ChainLinkList() {}

    ChainLinkList::~ChainLinkList()
    {
        deleteAll();
    }

    void ChainLinkList::deleteAll()
    {
        while(!m_chainLinks.isEmpty()) {
            delete m_chainLinks.takeFirst();
        }
    }

    int ChainLinkList::count() const
    {
        return m_chainLinks.count();
    }

    ChainLink* ChainLinkList::current() const
    {
        // use value() because m_current might be out of range for m_chainLinks
        return m_chainLinks.value(m_current);
    }

    ChainLink* ChainLinkList::first()
    {
        m_current = 0;
        return current();
    }

    ChainLink* ChainLinkList::next()
    {
        ++m_current;
        return current();
    }

    void ChainLinkList::prepend(ChainLink* link)
    {
        Q_ASSERT(link);
        m_chainLinks.prepend(link);
        m_current = 0;
    }

    void ChainLinkList::append(ChainLink* link)
    {
        Q_ASSERT(link);
        m_chainLinks.append(link);
        m_current = m_chainLinks.count() -1;
    }
}
