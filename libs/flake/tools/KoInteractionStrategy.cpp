/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
 * Copyright (C) 2011 Jan Hambrecht <jaham@gmx.net>
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

#include "KoInteractionStrategy.h"
#include "KoInteractionStrategy_p.h"
#include "KoCanvasBase.h"
#include "KoShapeController.h"
#include "KoDocumentResourceManager.h"

#include <kundo2command.h>

KoInteractionStrategy::KoInteractionStrategy(KoToolBase *parent)
    : d_ptr(new KoInteractionStrategyPrivate(parent))
{
}

void KoInteractionStrategy::cancelInteraction()
{
    KUndo2Command *cmd = createCommand();
    if (cmd) {
        cmd->redo(); //some applications rely an redo being called here
        cmd->undo();
        delete cmd;
    }
}

KoInteractionStrategy::KoInteractionStrategy(KoInteractionStrategyPrivate &dd)
    : d_ptr(&dd)
{
}

KoInteractionStrategy::~KoInteractionStrategy()
{
    delete d_ptr;
}

void KoInteractionStrategy::handleCustomEvent(KoPointerEvent *event)
{
    Q_UNUSED(event);
}

void KoInteractionStrategy::paint(QPainter &, const KoViewConverter &)
{
}

KoToolBase *KoInteractionStrategy::tool() const
{
    Q_D(const KoInteractionStrategy);
    return d->tool;
}

uint KoInteractionStrategy::handleRadius() const
{
    return tool()->canvas()->shapeController()->resourceManager()->handleRadius();
}

uint KoInteractionStrategy::grabSensitivity() const
{
    return tool()->canvas()->shapeController()->resourceManager()->grabSensitivity();
}
