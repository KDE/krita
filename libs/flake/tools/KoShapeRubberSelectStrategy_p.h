/* This file is part of the KDE project

   SPDX-FileCopyrightText: 2006, 2009 Thomas Zander <zander@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef KoShapeRubberSelectStrategyPrivate_H
#define KoShapeRubberSelectStrategyPrivate_H

#include "KoInteractionStrategy_p.h"
#include "KoSnapGuide.h"

class KoShapeRubberSelectStrategyPrivate : public KoInteractionStrategyPrivate
{
public:
    explicit KoShapeRubberSelectStrategyPrivate(KoToolBase *owner)
        : KoInteractionStrategyPrivate(owner),
        snapGuide(new KoSnapGuide(owner->canvas()))
    {
    }

    ~KoShapeRubberSelectStrategyPrivate()
    {
        delete snapGuide;
    }

    /**
     * Return the rectangle that the user dragged.
     * The rectangle is normalized and immutable.
     * @return a rectangle in pt.
     */
    QRectF selectedRect() const {
        return selectRect.normalized();
    }

    QRectF selectRect;
    QPointF lastPos;
    KoSnapGuide *snapGuide;
};

#endif
