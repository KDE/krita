/* This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2006-2007, 2010 Thomas Zander <zander@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KOINTERACTIONTOOLPRIVATE_H
#define KOINTERACTIONTOOLPRIVATE_H

#include "KoToolBase_p.h"
#include "KoInteractionStrategy.h"
#include "KoInteractionStrategyFactory.h"

class KoInteractionToolPrivate : public KoToolBasePrivate
{
public:
    KoInteractionToolPrivate(KoToolBase *qq, KoCanvasBase *canvas)
        : KoToolBasePrivate(qq, canvas),
        currentStrategy(0)
    {
    }

    ~KoInteractionToolPrivate() {
        delete currentStrategy;
    }

    QPointF lastPoint;
    KoInteractionStrategy *currentStrategy;
    QList<QSharedPointer<KoInteractionStrategyFactory>> interactionFactories;
};

#endif
