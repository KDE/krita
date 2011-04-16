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

KoTextLayoutRootArea *SimpleRootAreaProvider::provide(KoTextDocumentLayout *documentLayout, QString mastePageName)
{
    Q_UNUSED(mastePageName);

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

void SimpleRootAreaProvider::doPostLayout(KoTextLayoutRootArea *rootArea, bool isNewRootArea)
{
    Q_UNUSED(isNewRootArea);

    if (m_textShapeData->resizeMethod() == KoTextShapeData::AutoGrowWidthAndHeight
        ||m_textShapeData->resizeMethod() == KoTextShapeData::AutoGrowHeight) {
        rootArea->associatedShape()->setSize(QSize(
                        rootArea->associatedShape()->size().width(),
                        qMax(rootArea->associatedShape()->size().height(), rootArea->bottom() - rootArea->top())));
    }

    qreal newBottom = rootArea->top() + rootArea->associatedShape()->size().height();

    if (m_textShapeData->verticalAlignment() & Qt::AlignBottom) {
        if (true /*FIXME test no page based shapes interfering*/) {
            rootArea->setVerticalAlignOffset(newBottom - rootArea->bottom());
        }
    }
    if (m_textShapeData->verticalAlignment() & Qt::AlignVCenter) {
        if (true /*FIXME test no page based shapes interfering*/) {
            rootArea->setVerticalAlignOffset((newBottom - rootArea->bottom()) / 2);
        }
    }
    rootArea->setBottom(newBottom);

    rootArea->associatedShape()->update();
}

QSizeF SimpleRootAreaProvider::suggestSize(KoTextLayoutRootArea *rootArea)
{
    if (m_textShapeData->resizeMethod() == KoTextShapeData::AutoGrowWidthAndHeight
        ||m_textShapeData->resizeMethod() == KoTextShapeData::AutoGrowHeight) {
        QSizeF size = m_textShape->size();
        size.setHeight(1E6);
        return size;
    } else {
        return m_textShape->size();
    }
}
