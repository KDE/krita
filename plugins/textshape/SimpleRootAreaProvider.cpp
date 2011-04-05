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

#include "SimpleRootAreaProvider.h"

#include "KoTextLayoutRootArea.h"
#include "TextShape.h"

SimpleRootAreaProvider::SimpleRootAreaProvider(KoTextShapeData *data, TextShape *textshape)
    : m_textShape(textshape)
    , m_area(0)
    , m_textShapeData(data)

{
}

KoTextLayoutRootArea *SimpleRootAreaProvider::provide(KoTextDocumentLayout *documentLayout)
{
    if(m_area == 0) {
        m_area = new KoTextLayoutRootArea(documentLayout);
        m_area->setAssociatedShape(m_textShape);
        m_textShapeData->setRootArea(m_area);

        return m_area;
    }
    return 0;
}

void SimpleRootAreaProvider::releaseAllAfter(KoTextLayoutRootArea *afterThis)
{
}

void SimpleRootAreaProvider::doPostLayout(KoTextLayoutRootArea *rootArea)
{
    rootArea->setBottom(rootArea->top()
            + rootArea->associatedShape()->size().height());

    rootArea->associatedShape()->update();
}

QSizeF SimpleRootAreaProvider::suggestSize(KoTextLayoutRootArea *rootArea)
{
    return m_textShape->size();
}
