/* This file is part of the KDE project
 * Copyright (C) 2006,2008-2009 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2006,2007 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
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

#include "KoPathPointMoveCommand.h"
#include "KoPathPoint.h"
#include <klocale.h>

class KoPathPointMoveCommandPrivate
{
public:
    KoPathPointMoveCommandPrivate() : undoCalled(true) { }
    void applyOffset(qreal factor);

    bool undoCalled; // this command stores diffs; so calling undo twice will give wrong results. Guard against that.
    QMap<KoPathPointData, QPointF > points;
    QSet<KoPathShape*> paths;
};


KoPathPointMoveCommand::KoPathPointMoveCommand(const QList<KoPathPointData> &pointData, const QPointF &offset, QUndoCommand *parent)
    : QUndoCommand(parent),
    d(new KoPathPointMoveCommandPrivate())
{
    setText(i18n("Move points"));

    foreach (const KoPathPointData &data, pointData) {
        if (!d->points.contains(data)) {
            d->points[data] = offset;
            d->paths.insert(data.pathShape);
        }
    }
}

KoPathPointMoveCommand::KoPathPointMoveCommand(const QList<KoPathPointData> &pointData, const QList<QPointF> &offsets, QUndoCommand *parent)
    : QUndoCommand(parent),
    d(new KoPathPointMoveCommandPrivate())
{
    Q_ASSERT(pointData.count() == offsets.count());

    setText(i18n("Move points"));

    uint dataCount = pointData.count();
    for (uint i = 0; i < dataCount; ++i) {
        const KoPathPointData & data = pointData[i];
        if (!d->points.contains(data)) {
            d->points[data] = offsets[i];
            d->paths.insert(data.pathShape);
        }
    }
}

KoPathPointMoveCommand::~KoPathPointMoveCommand()
{
    delete d;
}

void KoPathPointMoveCommand::redo()
{
    QUndoCommand::redo();
    if (! d->undoCalled)
        return;

    d->applyOffset(1.0);
    d->undoCalled = false;
}

void KoPathPointMoveCommand::undo()
{
    QUndoCommand::undo();
    if (d->undoCalled)
        return;

    d->applyOffset(-1.0);
    d->undoCalled = true;
}

void KoPathPointMoveCommandPrivate::applyOffset(qreal factor)
{
    foreach (KoPathShape *path, paths) {
        // repaint old bounding rect
        path->update();
    }

    QMap<KoPathPointData, QPointF>::iterator it(points.begin());
    for (; it != points.end(); ++it) {
        KoPathShape *path = it.key().pathShape;
        // transform offset from document to shape coordinate system
        QPointF shapeOffset = path->documentToShape(factor*it.value()) - path->documentToShape(QPointF());
        QTransform matrix;
        matrix.translate(shapeOffset.x(), shapeOffset.y());

        KoPathPoint *p = path->pointByIndex(it.key().pointIndex);
        if (p)
            p->map(matrix, true);
    }

    foreach (KoPathShape *path, paths) {
        path->normalize();
        // repaint new bounding rect
        path->update();
    }
}
