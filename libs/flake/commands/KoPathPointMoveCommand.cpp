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
#include <klocalizedstring.h>
#include "kis_command_ids.h"
#include "krita_container_utils.h"

class KoPathPointMoveCommandPrivate
{
public:
    KoPathPointMoveCommandPrivate() { }
    void applyOffset(qreal factor);

    QMap<KoPathPointData, QPointF > points;
    QSet<KoPathShape*> paths;
};


KoPathPointMoveCommand::KoPathPointMoveCommand(const QList<KoPathPointData> &pointData, const QPointF &offset, KUndo2Command *parent)
    : KUndo2Command(parent),
    d(new KoPathPointMoveCommandPrivate())
{
    setText(kundo2_i18n("Move points"));

    foreach (const KoPathPointData &data, pointData) {
        if (!d->points.contains(data)) {
            d->points[data] = offset;
            d->paths.insert(data.pathShape);
        }
    }
}

KoPathPointMoveCommand::KoPathPointMoveCommand(const QList<KoPathPointData> &pointData, const QList<QPointF> &offsets, KUndo2Command *parent)
    : KUndo2Command(parent),
    d(new KoPathPointMoveCommandPrivate())
{
    Q_ASSERT(pointData.count() == offsets.count());

    setText(kundo2_i18n("Move points"));

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
    KUndo2Command::redo();
    d->applyOffset(1.0);
}

void KoPathPointMoveCommand::undo()
{
    KUndo2Command::undo();
    d->applyOffset(-1.0);
}

int KoPathPointMoveCommand::id() const
{
    return KisCommandUtils::ChangePathShapePointId;
}

bool KoPathPointMoveCommand::mergeWith(const KUndo2Command *command)
{
    const KoPathPointMoveCommand *other = dynamic_cast<const KoPathPointMoveCommand*>(command);

    if (!other ||
        other->d->paths != d->paths ||
        !KritaUtils::compareListsUnordered(other->d->points.keys(), d->points.keys())) {

        return false;
    }

    auto it = d->points.begin();
    while (it != d->points.end()) {
        it.value() += other->d->points[it.key()];
        ++it;
    }

    return true;
}

void KoPathPointMoveCommandPrivate::applyOffset(qreal factor)
{
    QMap<KoShape*, QRectF> oldDirtyRects;

    foreach (KoPathShape *path, paths) {
        oldDirtyRects[path] = path->boundingRect();
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
            p->map(matrix);
    }

    foreach (KoPathShape *path, paths) {
        path->normalize();
        // repaint new bounding rect
        path->updateAbsolute(oldDirtyRects[path] | path->boundingRect());
    }
}
