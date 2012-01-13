/* This file is part of the KDE project
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
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

#include "KoShapeRunAroundCommand.h"

#include <QString>
#include <klocale.h>
#include "KoShape.h"

class KoShapeRunAroundCommand::Private
{
public:
    Private(KoShape *s, KoShape::TextRunAroundSide side, int runThrough, qreal distance, qreal threshold)
    : shape(s)
    , newSide(side)
    , newRunThrough(runThrough)
    , newDistance(distance)
    , newThreshold(threshold)
    , oldSide(shape->textRunAroundSide())
    , oldRunThrough(shape->runThrough())
    , oldDistance(shape->textRunAroundDistance())
    , oldThreshold(shape->textRunAroundThreshold())
    {}

    KoShape *shape;
    KoShape::TextRunAroundSide newSide;
    int newRunThrough;
    qreal newDistance;
    qreal newThreshold;
    KoShape::TextRunAroundSide oldSide;
    int oldRunThrough;
    qreal oldDistance;
    qreal oldThreshold;
};

KoShapeRunAroundCommand::KoShapeRunAroundCommand(KoShape *shape, KoShape::TextRunAroundSide side, int runThrough, qreal distance, qreal threshold, KUndo2Command *parent)
: KUndo2Command(parent)
, d(new Private(shape, side, runThrough, distance, threshold))
{
    setText(i18nc("(qtundo-format)", "Change Shape RunAround"));
}

KoShapeRunAroundCommand::~KoShapeRunAroundCommand()
{
   delete d;
}

void KoShapeRunAroundCommand::redo()
{
    KUndo2Command::redo();
    d->shape->setTextRunAroundSide(d->newSide, KoShape::Background);
    d->shape->setRunThrough(d->newRunThrough);
    d->shape->setTextRunAroundDistance(d->newDistance);
    d->shape->setTextRunAroundThreshold(d->newThreshold);
    d->shape->notifyChanged();
}

void KoShapeRunAroundCommand::undo()
{
    KUndo2Command::undo();
    d->shape->setTextRunAroundSide(d->oldSide, KoShape::Background);
    d->shape->setRunThrough(d->oldRunThrough);
    d->shape->setTextRunAroundDistance(d->oldDistance);
    d->shape->setTextRunAroundThreshold(d->oldThreshold);
    d->shape->notifyChanged();
}
