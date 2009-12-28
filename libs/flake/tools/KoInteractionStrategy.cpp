/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
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

#include <QUndoCommand>

KoInteractionStrategy::KoInteractionStrategy(KoTool *parent)
    : d_ptr(new KoInteractionStrategyPrivate(parent))
{
}

void KoInteractionStrategy::cancelInteraction()
{
    QUndoCommand *cmd = createCommand();
    if (cmd) {
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
    Q_D(KoInteractionStrategy);
    delete d;
}

void KoInteractionStrategy::handleCustomEvent(KoPointerEvent *event)
{
    Q_UNUSED(event);
}

void KoInteractionStrategy::paint(QPainter &, const KoViewConverter &)
{
}

KoTool *KoInteractionStrategy::tool() const
{
    Q_D(const KoInteractionStrategy);
    return d->tool;
}
