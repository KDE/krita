/* This file is part of the KDE project
 * Copyright (C) 2011 Casper Boemann <cbo@boemann.dk>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "MockRootAreaProvider.h"

#include "KoTextLayoutRootArea.h"

class MockRootArea : public KoTextLayoutRootArea
{
public:
    MockRootArea() : KoTextLayoutRootArea() {}
 
    virtual qreal maximalAllowedY() const
    {
        return 1000;
    }

    virtual qreal right() const
    {
        return 200;
    }
};

MockRootAreaProvider::MockRootAreaProvider()
    : m_area(new MockRootArea())
{
}

KoTextLayoutRootArea *MockRootAreaProvider::provide(KoTextLayoutRootArea *previous)
{
    if (previous) {
        return 0;;
    }
    else
        return m_area;
}