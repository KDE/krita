/* This file is part of the KDE project
 * Copyright (C) 2011 KO GmbH <cbo@kogmbh.com>
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

#include "DummyDocumentLayout.h"

#include <KoPostscriptPaintDevice.h>

#include <QTextBlock>

#include <TextLayoutDebug.h>

// ------------------- DummyDocumentLayout --------------------
DummyDocumentLayout::DummyDocumentLayout(QTextDocument *doc)
        : QAbstractTextDocumentLayout(doc)
{
    setPaintDevice(new KoPostscriptPaintDevice());
}

DummyDocumentLayout::~DummyDocumentLayout()
{
}

QRectF DummyDocumentLayout::blockBoundingRect(const QTextBlock &block) const
{
    Q_UNUSED(block);
    return QRect();
}

QSizeF DummyDocumentLayout::documentSize() const
{
    return QSizeF();
}

void DummyDocumentLayout::draw(QPainter *painter, const QAbstractTextDocumentLayout::PaintContext &context)
{
    // WARNING Text shapes ask their root area directly to paint.
    // It saves a lot of extra traversal, that is quite costly for big
    // documents
    Q_UNUSED(painter);
    Q_UNUSED(context);
}


int DummyDocumentLayout::hitTest(const QPointF &point, Qt::HitTestAccuracy accuracy) const
{
    Q_UNUSED(point);
    Q_UNUSED(accuracy);
    Q_ASSERT(false); //we should not call this method.
    return -1;
}

QRectF DummyDocumentLayout::frameBoundingRect(QTextFrame*) const
{
    return QRectF();
}

int DummyDocumentLayout::pageCount() const
{
    return 1;
}

void DummyDocumentLayout::documentChanged(int position, int charsRemoved, int charsAdded)
{
    Q_UNUSED(position);
    Q_UNUSED(charsRemoved);
    Q_UNUSED(charsAdded);
}

/*
void DummyDocumentLayout::drawInlineObject(QPainter *, const QRectF &, QTextInlineObject , int , const QTextFormat &)
{
}

// This method is called by qt every time  QTextLine.setWidth()/setNumColumns() is called
void DummyDocumentLayout::positionInlineObject(QTextInlineObject , int , const QTextFormat &)
{
}

void DummyDocumentLayout::resizeInlineObject(QTextInlineObject , int , const QTextFormat &)
{
}
*/
