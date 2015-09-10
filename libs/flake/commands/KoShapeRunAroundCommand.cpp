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
#include <klocalizedstring.h>
#include "KoShape.h"

class Q_DECL_HIDDEN KoShapeRunAroundCommand::Private
{
public:
    Private(KoShape *s, KoShape::TextRunAroundSide side, int runThrough, qreal distanceLeft, qreal distanceTop, qreal distanceRight, qreal distanceBottom, qreal threshold, KoShape::TextRunAroundContour contour)
    : shape(s)
    , newSide(side)
    , newRunThrough(runThrough)
    , newDistanceLeft(distanceLeft)
    , newDistanceTop(distanceTop)
    , newDistanceRight(distanceRight)
    , newDistanceBottom(distanceBottom)
    , newThreshold(threshold)
    , newContour(contour)
    , oldSide(shape->textRunAroundSide())
    , oldRunThrough(shape->runThrough())
    , oldDistanceLeft(shape->textRunAroundDistanceLeft())
    , oldDistanceTop(shape->textRunAroundDistanceTop())
    , oldDistanceRight(shape->textRunAroundDistanceRight())
    , oldDistanceBottom(shape->textRunAroundDistanceBottom())
    , oldThreshold(shape->textRunAroundThreshold())
    , oldContour(shape->textRunAroundContour())
    {}

    KoShape *shape;
    KoShape::TextRunAroundSide newSide;
    int newRunThrough;
    qreal newDistanceLeft;
    qreal newDistanceTop;
    qreal newDistanceRight;
    qreal newDistanceBottom;
    qreal newThreshold;
    KoShape::TextRunAroundContour newContour;
    KoShape::TextRunAroundSide oldSide;
    int oldRunThrough;
    qreal oldDistanceLeft;
    qreal oldDistanceTop;
    qreal oldDistanceRight;
    qreal oldDistanceBottom;
    qreal oldThreshold;
    KoShape::TextRunAroundContour oldContour;
};

KoShapeRunAroundCommand::KoShapeRunAroundCommand(KoShape *shape, KoShape::TextRunAroundSide side, int runThrough, qreal distanceLeft, qreal distanceTop, qreal distanceRight, qreal distanceBottom, qreal threshold, KoShape::TextRunAroundContour contour, KUndo2Command *parent)
: KUndo2Command(parent)
, d(new Private(shape, side, runThrough, distanceLeft, distanceTop, distanceRight, distanceBottom, threshold, contour))
{
    setText(kundo2_i18n("Change Shape RunAround"));
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
    d->shape->setTextRunAroundDistanceLeft(d->newDistanceLeft);
    d->shape->setTextRunAroundDistanceTop(d->newDistanceTop);
    d->shape->setTextRunAroundDistanceRight(d->newDistanceRight);
    d->shape->setTextRunAroundDistanceBottom(d->newDistanceBottom);
    d->shape->setTextRunAroundThreshold(d->newThreshold);
    d->shape->setTextRunAroundContour(d->newContour);
    d->shape->notifyChanged();
}

void KoShapeRunAroundCommand::undo()
{
    KUndo2Command::undo();
    d->shape->setTextRunAroundSide(d->oldSide, KoShape::Background);
    d->shape->setRunThrough(d->oldRunThrough);
    d->shape->setTextRunAroundDistanceLeft(d->oldDistanceLeft);
    d->shape->setTextRunAroundDistanceTop(d->oldDistanceTop);
    d->shape->setTextRunAroundDistanceRight(d->oldDistanceRight);
    d->shape->setTextRunAroundDistanceBottom(d->oldDistanceBottom);
    d->shape->setTextRunAroundThreshold(d->oldThreshold);
    d->shape->setTextRunAroundContour(d->oldContour);
    d->shape->notifyChanged();
}
