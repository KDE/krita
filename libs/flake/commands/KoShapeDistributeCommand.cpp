/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
 * Copyright (C) 2006 Jan Hambrecht <jaham@gmx.net>
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

#include "KoShapeDistributeCommand.h"

#include <QMap>

#include <klocale.h>

class KoShapeDistributeCommand::Private
{
public:
    Private() : command(0) {}
    ~Private() {
        delete command;
    }

    qreal getAvailableSpace(KoShape *first, KoShape *last, qreal extent, const QRectF &boundingRect);

    Distribute distribute;
    KoShapeMoveCommand *command;
};

KoShapeDistributeCommand::KoShapeDistributeCommand(const QList<KoShape*> &shapes, Distribute distribute, const QRectF &boundingRect, QUndoCommand *parent)
        : QUndoCommand(parent),
        d(new Private())
{
    d->distribute = distribute;
    QMap<qreal, KoShape*> sortedPos;
    QRectF bRect;
    qreal extent = 0.0;
    // sort by position and calculate sum of objects widht/height
    foreach(KoShape *shape, shapes) {
        bRect = shape->boundingRect();
        switch (d->distribute) {
        case HorizontalCenterDistribution:
            sortedPos[bRect.center().x()] = shape;
            break;
        case HorizontalGapsDistribution:
        case HorizontalLeftDistribution:
            sortedPos[bRect.left()] = shape;
            extent += bRect.width();
            break;
        case HorizontalRightDistribution:
            sortedPos[bRect.right()] = shape;
            break;
        case VerticalCenterDistribution:
            sortedPos[bRect.center().y()] = shape;
            break;
        case VerticalGapsDistribution:
        case VerticalBottomDistribution:
            sortedPos[bRect.bottom()] = shape;
            extent += bRect.height();
            break;
        case VerticalTopDistribution:
            sortedPos[bRect.top()] = shape;
            break;
        }
    }
    KoShape* first = sortedPos.begin().value();
    KoShape* last = (--sortedPos.end()).value();

    // determine the available space to distribute
    qreal space = d->getAvailableSpace(first, last, extent, boundingRect);
    qreal pos = 0.0, step = space / qreal(shapes.count() - 1);

    QList<QPointF> previousPositions;
    QList<QPointF> newPositions;
    QPointF position;
    QPointF delta;
    QMapIterator<qreal, KoShape*> it(sortedPos);
    while (it.hasNext()) {
        it.next();
        position = it.value()->position();
        previousPositions  << position;

        bRect = it.value()->boundingRect();
        switch (d->distribute)        {
        case HorizontalCenterDistribution:
            delta = QPointF(boundingRect.x() + first->boundingRect().width() / 2 + pos - bRect.width() / 2, bRect.y()) - bRect.topLeft();
            break;
        case HorizontalGapsDistribution:
            delta = QPointF(boundingRect.left() + pos, bRect.y()) - bRect.topLeft();
            pos += bRect.width();
            break;
        case HorizontalLeftDistribution:
            delta = QPointF(boundingRect.left() + pos, bRect.y()) - bRect.topLeft();
            break;
        case HorizontalRightDistribution:
            delta = QPointF(boundingRect.left() + first->boundingRect().width() + pos - bRect.width(), bRect.y()) - bRect.topLeft();
            break;
        case VerticalCenterDistribution:
            delta = QPointF(bRect.x(), boundingRect.y() + first->boundingRect().height() / 2 + pos - bRect.height() / 2) - bRect.topLeft();
            break;
        case VerticalGapsDistribution:
            delta = QPointF(bRect.x(), boundingRect.top() + pos) - bRect.topLeft();
            pos += bRect.height();
            break;
        case VerticalBottomDistribution:
            delta = QPointF(bRect.x(), boundingRect.top() + first->boundingRect().height() + pos - bRect.height()) - bRect.topLeft();
            break;
        case VerticalTopDistribution:
            delta = QPointF(bRect.x(), boundingRect.top() + pos) - bRect.topLeft();
            break;
        };
        newPositions  << position + delta;
        pos += step;
    }
    d->command = new KoShapeMoveCommand(sortedPos.values(), previousPositions, newPositions);

    setText(i18n("Distribute shapes"));
}

KoShapeDistributeCommand::~KoShapeDistributeCommand()
{
    delete d;
}

void KoShapeDistributeCommand::redo()
{
    QUndoCommand::redo();
    d->command->redo();
}

void KoShapeDistributeCommand::undo()
{
    QUndoCommand::undo();
    d->command->undo();
}

qreal KoShapeDistributeCommand::Private::getAvailableSpace(KoShape *first, KoShape *last, qreal extent, const QRectF &boundingRect)
{
    switch (distribute) {
    case HorizontalCenterDistribution:
        return boundingRect.width() - last->boundingRect().width() / 2 - first->boundingRect().width() / 2;
        break;
    case HorizontalGapsDistribution:
        return boundingRect.width() - extent;
        break;
    case HorizontalLeftDistribution:
        return boundingRect.width() - last->boundingRect().width();
        break;
    case HorizontalRightDistribution:
        return boundingRect.width() - first->boundingRect().width();
        break;
    case VerticalCenterDistribution:
        return boundingRect.height() - last->boundingRect().height() / 2 - first->boundingRect().height() / 2;
        break;
    case VerticalGapsDistribution:
        return boundingRect.height() - extent;
        break;
    case VerticalBottomDistribution:
        return boundingRect.height() - first->boundingRect().height();
        break;
    case VerticalTopDistribution:
        return boundingRect.height() - last->boundingRect().height();
        break;
    }
    return 0.0;
}
