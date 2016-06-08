/* This file is part of the KDE project
 * Copyright (C) 2011 C. Boemann <cbo@kogmbh.com>
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

#include "KoTextLayoutRootArea.h"

#include "FrameIterator.h"

#include <KoShapeContainer.h>
#include <KoTextShapeData.h>
#include <KoTextPage.h>

class Q_DECL_HIDDEN KoTextLayoutRootArea::Private
{
public:
    Private()
        : shape(0)
        , dirty(true)
        , textpage(0)
        , nextStartOfArea(0)
    {
    }
    KoShape *shape;
    bool dirty;
    KoTextPage *textpage;
    FrameIterator *nextStartOfArea;
};

KoTextLayoutRootArea::KoTextLayoutRootArea(KoTextDocumentLayout *documentLayout)
  : KoTextLayoutArea(0, documentLayout)
  , d(new Private)
{
}

KoTextLayoutRootArea::~KoTextLayoutRootArea()
{
    if (d->shape)
    {
        KoTextShapeData *data = qobject_cast<KoTextShapeData*>(d->shape->userData());
        if (data)
            data->setRootArea(0);
    }
    delete d->nextStartOfArea;
    delete d->textpage;
    delete d;
}

bool KoTextLayoutRootArea::layoutRoot(FrameIterator *cursor)
{
    d->dirty = false;

    setVirginPage(true);

    bool retval = KoTextLayoutArea::layout(cursor);

    delete d->nextStartOfArea;
    d->nextStartOfArea = new FrameIterator(cursor);
    return retval;
}

void KoTextLayoutRootArea::setAssociatedShape(KoShape *shape)
{
    d->shape = shape;
}

KoShape *KoTextLayoutRootArea::associatedShape() const
{
    return d->shape;
}

void KoTextLayoutRootArea::setPage(KoTextPage *textpage)
{
    delete d->textpage;
    d->textpage = textpage;
}

KoTextPage* KoTextLayoutRootArea::page() const
{
    if (d->textpage) {
        return d->textpage;
    }
    // If this root area has no KoTextPage then walk up the shape-hierarchy and look if we
    // have a textshape-parent that has a valid KoTextPage. This handles the in Words valid
    // case that the associatedShape is nested in another shape.
    KoTextPage *p = 0;
    for(KoShape *shape = associatedShape() ? associatedShape()->parent() : 0; shape; shape = shape->parent()) {
        if (KoTextShapeData *data = qobject_cast<KoTextShapeData*>(shape->userData())) {
            if (KoTextLayoutRootArea *r = data->rootArea())
                p = r->page();
            break;
        }
    }
    return p;
}

void KoTextLayoutRootArea::setDirty()
{
    d->dirty = true;
    documentLayout()->emitLayoutIsDirty();
}

bool KoTextLayoutRootArea::isDirty() const
{
    return d->dirty;
}

FrameIterator *KoTextLayoutRootArea::nextStartOfArea() const
{
    return d->nextStartOfArea;
}


KoText::Direction KoTextLayoutRootArea::parentTextDirection() const
{
    return KoText::LeftRightTopBottom;
}

void KoTextLayoutRootArea::setBottom(qreal b)
{
    KoTextLayoutArea::setBottom(b);
}
