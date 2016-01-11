/* This file is part of the KDE project
 * Copyright (C) 2011 Brijesh Patel <brijesh3105@gmail.com>
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

#ifndef KOTEXTLAYOUTNOTEAREA_H
#define KOTEXTLAYOUTNOTEAREA_H

#include "KoTextLayoutArea.h"

#include <KoTextDocumentLayout.h>

class KoInlineNote;

class KRITATEXTLAYOUT_EXPORT KoTextLayoutNoteArea : public KoTextLayoutArea
{
public:
    explicit KoTextLayoutNoteArea(KoInlineNote *note, KoTextLayoutArea *parent, KoTextDocumentLayout *documentLayout);
    virtual ~KoTextLayoutNoteArea();

    void paint(QPainter *painter, const KoTextDocumentLayout::PaintContext &context);

    bool layout(FrameIterator *cursor);

    void setAsContinuedArea(bool isContinuedArea);

    KoPointedAt hitTest(const QPointF &point, Qt::HitTestAccuracy accuracy) const;

    QRectF selectionBoundingBox(QTextCursor &cursor) const;

private:
    class Private;
    Private * const d;
};

#endif // KOTEXTLAYOUTNOTEAREA_H
