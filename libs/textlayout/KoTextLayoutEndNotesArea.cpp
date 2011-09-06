/* This file is part of the KDE project
 * Copyright (C) 2011 Casper Boemann <cbo@kogmbh.com>
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

#include "KoTextLayoutEndNotesArea.h"
#include "FrameIterator.h"

#include <KoTextPage.h>

#include <QPainter>

class KoTextLayoutEndNotesArea::Private
{
public:
    Private()
        : startOfArea(0)
    {
    }
    QList<KoTextLayoutArea *> endNoteAreas;
    FrameIterator *startOfArea;
    FrameIterator *endOfArea;
};

KoTextLayoutEndNotesArea::KoTextLayoutEndNotesArea(KoTextLayoutArea *parent, KoTextDocumentLayout *documentLayout)
  : KoTextLayoutArea(parent, documentLayout)
  , d(new Private)
{
}

KoTextLayoutEndNotesArea::~KoTextLayoutEndNotesArea()
{
    qDeleteAll(d->endNoteAreas);
    delete d;
}

bool KoTextLayoutEndNotesArea::layout(FrameIterator *cursor)
{
    qDeleteAll(d->endNoteAreas);
    d->endNoteAreas.clear();

    d->startOfArea = new FrameIterator(cursor);
    d->endOfArea = 0;

    qreal y = top()+15;
    setBottom(y);

    while (true) {
        QTextFrame *subFrame = cursor->it.currentFrame();
        if (subFrame) {
            // This is an actual endNote frame. Just use a normal Area to layout
            KoTextLayoutArea *noteArea = new KoTextLayoutArea(this, documentLayout());
            d->endNoteAreas.append(noteArea);
            noteArea->setReferenceRect(left(), right(), y, maximumAllowedBottom());
            if (noteArea->layout(cursor->subFrameIterator(subFrame)) == false) {
                d->endOfArea = new FrameIterator(cursor);
                setBottom(noteArea->bottom());
                return false;
            }
            y = noteArea->bottom();
            setBottom(y);
            delete cursor->currentSubFrameIterator;
            cursor->currentSubFrameIterator = 0;
        }

        ++(cursor->it);

        if (cursor->it.atEnd()) {
            d->endOfArea = new FrameIterator(cursor);
            return true; // we have layouted till the end of the frame
        }
    }
}
KoPointedAt KoTextLayoutEndNotesArea::hitTest(const QPointF &p, Qt::HitTestAccuracy accuracy) const
{
    KoPointedAt pointedAt;
    int endNoteIndex = 0;
    while (endNoteIndex<d->endNoteAreas.length()) {
        // check if p is over end notes area
        if (p.y() > d->endNoteAreas[endNoteIndex]->top()
                && p.y() < d->endNoteAreas[endNoteIndex]->bottom()) {
            pointedAt = d->endNoteAreas[endNoteIndex]->hitTest(p, accuracy);
            return pointedAt;
        }
        ++endNoteIndex;
    }
    return KoPointedAt();
}

/*QRectF KoTextLayoutEndNotesArea::selectionBoundingBox(QTextCursor &cursor) const
{

}*/

void KoTextLayoutEndNotesArea::paint(QPainter *painter, const KoTextDocumentLayout::PaintContext &context)
{
    if (d->startOfArea == 0) // We have not been layouted yet
        return;

    if (!d->endNoteAreas.isEmpty()) {
        painter->drawLine(2, top()+10, 150, top()+10);
    }
    foreach(KoTextLayoutArea *area, d->endNoteAreas) {
        area->paint(painter, context);
    }
}
