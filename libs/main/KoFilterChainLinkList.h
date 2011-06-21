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
#ifndef KOFILTERCHAINLINKLIST_H
#define KOFILTERCHAINLINKLIST_H

#include <QList>

namespace CalligraFilter {

    class ChainLink;


    class ChainLinkList
    {
    public:
        ChainLinkList();
        ~ChainLinkList();

        void deleteAll();
        int count() const;

        /**
         * Return a pointer to the current position in the chain.
         * @return pointer to the current ChainLink or 0 if the ChainLinkList is empty.
         **/
        ChainLink* current() const;

        /**
         * Move the position to the first position in the chain.
         * @return pointer to the first ChainLink or 0 if the ChainLinkList is empty.
         **/
        ChainLink* first();

        ChainLink* next();

        void prepend(ChainLink* link);

        void append(ChainLink* link);

    private:

        QList<ChainLink*> m_chainLinks;
        int m_current;

    };

}

#endif // KOFILTERCHAINLINKLIST_H
