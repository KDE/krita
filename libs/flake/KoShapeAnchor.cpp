/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2007, 2009-2010 Thomas Zander <zander@kde.org>
 * SPDX-FileCopyrightText: 2010 Ko Gmbh <cbo@kogmbh.com>
 * SPDX-FileCopyrightText: 2011 Matus Hanzes <matus.hanzes@ixonos.com>
 * SPDX-FileCopyrightText: 2013 C. Boemann <cbo@boemann.dk>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KoShapeAnchor.h"

#include <KoShapeContainer.h>
#include <KoXmlWriter.h>
#include <KoXmlReader.h>
#include <KoXmlNS.h>
#include <KoShapeSavingContext.h>
#include <KoShapeLoadingContext.h>

#include <QRectF>
#include <QTransform>
#include <FlakeDebug.h>

class Q_DECL_HIDDEN KoShapeAnchor::Private
{
public:
    Private(KoShape *s)
            : shape(s)
            , verticalPos(KoShapeAnchor::VTop)
            , verticalRel(KoShapeAnchor::VLine)
            , horizontalPos(KoShapeAnchor::HLeft)
            , horizontalRel(KoShapeAnchor::HChar)
            , flowWithText(true)
            , anchorType(KoShapeAnchor::AnchorToCharacter)
            , placementStrategy(0)
            , pageNumber(-1)
            , textLocation(0)
    {
    }


    QDebug printDebug(QDebug dbg) const
    {
#ifndef NDEBUG
        dbg.space() << "KoShapeAnchor" << this;
        dbg.space() << "offset:" << offset;
        dbg.space() << "shape:" << shape->name();
#endif
        return dbg.space();
    }

    KoShape * const shape;
    QPointF offset;
    KoShapeAnchor::VerticalPos verticalPos;
    KoShapeAnchor::VerticalRel verticalRel;
    KoShapeAnchor::HorizontalPos horizontalPos;
    KoShapeAnchor::HorizontalRel horizontalRel;
    QString wrapInfluenceOnPosition;
    bool flowWithText;
    KoShapeAnchor::AnchorType anchorType;
    KoShapeAnchor::PlacementStrategy *placementStrategy;
    int pageNumber;
    KoShapeAnchor::TextLocation *textLocation;
};

KoShapeAnchor::KoShapeAnchor(KoShape *shape)
    : d(new Private(shape))
{
}

KoShapeAnchor::~KoShapeAnchor()
{
    if (d->placementStrategy != 0) {
        delete d->placementStrategy;
    }
    delete d;
}

KoShape *KoShapeAnchor::shape() const
{
    return d->shape;
}

KoShapeAnchor::AnchorType KoShapeAnchor::anchorType() const
{
    return d->anchorType;
}

void KoShapeAnchor::setHorizontalPos(HorizontalPos hp)
{
    d->horizontalPos = hp;
}

KoShapeAnchor::HorizontalPos KoShapeAnchor::horizontalPos() const
{
    return d->horizontalPos;
}

void KoShapeAnchor::setHorizontalRel(HorizontalRel hr)
{
    d->horizontalRel = hr;
}

KoShapeAnchor::HorizontalRel KoShapeAnchor::horizontalRel() const
{
    return d->horizontalRel;
}

void KoShapeAnchor::setVerticalPos(VerticalPos vp)
{
    d->verticalPos = vp;
}

KoShapeAnchor::VerticalPos KoShapeAnchor::verticalPos() const
{
    return d->verticalPos;
}

void KoShapeAnchor::setVerticalRel(VerticalRel vr)
{
    d->verticalRel = vr;
}

KoShapeAnchor::VerticalRel KoShapeAnchor::verticalRel() const
{
    return d->verticalRel;
}

QString KoShapeAnchor::wrapInfluenceOnPosition() const
{
    return d->wrapInfluenceOnPosition;
}

bool KoShapeAnchor::flowWithText() const
{
    return d->flowWithText;
}

int KoShapeAnchor::pageNumber() const
{
    return d->pageNumber;
}

const QPointF &KoShapeAnchor::offset() const
{
    return d->offset;
}

void KoShapeAnchor::setOffset(const QPointF &offset)
{
    d->offset = offset;
}

void KoShapeAnchor::setAnchorType(KoShapeAnchor::AnchorType type)
{
    d->anchorType = type;
    if (type == AnchorAsCharacter) {
        d->horizontalRel = HChar;
        d->horizontalPos = HLeft;
    }
}

KoShapeAnchor::TextLocation *KoShapeAnchor::textLocation() const
{
    return d->textLocation;
}

void KoShapeAnchor::setTextLocation(TextLocation *textLocation)
{
    d->textLocation = textLocation;
}

KoShapeAnchor::PlacementStrategy *KoShapeAnchor::placementStrategy() const
{
    return d->placementStrategy;
}

void KoShapeAnchor::setPlacementStrategy(PlacementStrategy *placementStrategy)
{
    if (placementStrategy != d->placementStrategy) {
        delete d->placementStrategy;

        d->placementStrategy = placementStrategy;
    }
}
