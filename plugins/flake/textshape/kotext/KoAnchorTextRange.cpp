/* This file is part of the KDE project
 * Copyright (C) 2007, 2009-2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2010 Ko Gmbh <cbo@kogmbh.com>
 * Copyright (C) 2011 Matus Hanzes <matus.hanzes@ixonos.com>
 * Copyright (C) 2013 C. Boemann <cbo@boemann.dk>
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

#include "KoAnchorTextRange.h"
#include "KoShapeAnchor.h"

#include <KoShapeSavingContext.h>
#include <KoShape.h>

#include "TextDebug.h"


class KoAnchorTextRangePrivate
{
public:
    KoAnchorTextRangePrivate(KoShapeAnchor *p)
        : parent(p)
    {
    }

    KoShapeAnchor *parent;
};

KoAnchorTextRange::KoAnchorTextRange(KoShapeAnchor *parent, const QTextCursor &cursor)
    : KoTextRange(cursor)
    , d_ptr(new KoAnchorTextRangePrivate(parent))
{
    Q_ASSERT(parent);
    parent->setTextLocation(this);
}

KoAnchorTextRange::~KoAnchorTextRange()
{
    delete d_ptr;
}

KoShapeAnchor *KoAnchorTextRange::anchor() const
{
    Q_D(const KoAnchorTextRange);
    return d->parent;
}

const QTextDocument *KoAnchorTextRange::document() const
{
    return KoTextRange::document();
}

int KoAnchorTextRange::position() const
{
    return rangeStart();
}

void KoAnchorTextRange::updateContainerModel()
{
    Q_D(KoAnchorTextRange);

    if (!d->parent->shape()->isVisible()) {
        // Per default the shape this anchor presents is hidden and we only make it visible once an
        // explicit placement is made. This prevents shapes that are anchored at e.g. hidden
        // textboxes to not become visible.
        d->parent->shape()->setVisible(true);
    }

    if (d->parent->placementStrategy() != 0) {
        d->parent->placementStrategy()->updateContainerModel();
    }
}

bool KoAnchorTextRange::loadOdf(const KoXmlElement &, KoShapeLoadingContext &)
{
    return true;
}

void KoAnchorTextRange::saveOdf(KoShapeSavingContext &context, int position, KoTextRange::TagType tagType) const
{
    Q_UNUSED(position);
    Q_UNUSED(tagType);

    Q_D(const KoAnchorTextRange);
    if (tagType == KoTextRange::StartTag) {
        d->parent->saveOdf(context);
    }
}
