/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2006 Thomas Zander <zander@kde.org>
 * SPDX-FileCopyrightText: 2011 Jan Hambrecht <jaham@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
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
