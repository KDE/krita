/* This file is part of the KDE project
 * Copyright (C) 2011 C. Boemann <cbo@boemann.dk>
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

MockRootAreaProvider::MockRootAreaProvider()
    : m_area(0),
    m_suggestedSize(QSizeF(200,1000)),
    m_askedForMoreThenOneArea(false)
{
}

KoTextLayoutRootArea *MockRootAreaProvider::provide(KoTextDocumentLayout *documentLayout)
{
    if(m_area == 0) {
        m_area = new KoTextLayoutRootArea(documentLayout);
        return m_area;
    }
    m_askedForMoreThenOneArea = true;
    return 0;
}

void MockRootAreaProvider::doPostLayout(KoTextLayoutRootArea *rootArea, bool isNewRootArea)
{
    Q_UNUSED(rootArea);
    Q_UNUSED(isNewRootArea);
}

void MockRootAreaProvider::releaseAllAfter(KoTextLayoutRootArea *afterThis)
{
    Q_UNUSED(afterThis);
}

QSizeF MockRootAreaProvider::suggestSize(KoTextLayoutRootArea *rootArea)
{
    Q_UNUSED(rootArea);
    return m_suggestedSize;
}

void MockRootAreaProvider::setSuggestedSize(QSizeF size)
{
    m_suggestedSize = size;
}

QList<KoTextLayoutObstruction *> MockRootAreaProvider::relevantObstructions(KoTextLayoutRootArea *rootArea)
{
    Q_UNUSED(rootArea);
    QList<KoTextLayoutObstruction*> obstructions;
    return obstructions;
}

