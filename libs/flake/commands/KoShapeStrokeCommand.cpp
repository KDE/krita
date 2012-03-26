/* This file is part of the KDE project
 *
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
 * Copyright (C) 2006-2008 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2012 Inge Wallin <inge@lysator.liu.se>
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

#include "KoShapeStrokeCommand.h"
#include "KoShape.h"
#include "KoShapeStrokeModel.h"

#include <klocale.h>

class KoShapeStrokeCommand::Private
{
public:
    Private() {}
    ~Private()
    {
        foreach(KoShapeStrokeModel* stroke, oldStrokes) {
            if (stroke && !stroke->deref())
                delete stroke;
        }
    }

    void addOldStroke(KoShapeStrokeModel * oldStroke)
    {
        if (oldStroke)
            oldStroke->ref();
        oldStrokes.append(oldStroke);
    }

    void addNewStroke(KoShapeStrokeModel * newStroke)
    {
        if (newStroke)
            newStroke->ref();
        newStrokes.append(newStroke);
    }

    QList<KoShape*> shapes;                ///< the shapes to set stroke for
    QList<KoShapeStrokeModel*> oldStrokes; ///< the old strokes, one for each shape
    QList<KoShapeStrokeModel*> newStrokes; ///< the new strokes to set
};

KoShapeStrokeCommand::KoShapeStrokeCommand(const QList<KoShape*> &shapes, KoShapeStrokeModel *stroke, KUndo2Command *parent)
    : KUndo2Command(parent)
    , d(new Private())
{
    d->shapes = shapes;

    // save old strokes
    foreach(KoShape *shape, d->shapes) {
        d->addOldStroke(shape->stroke());
        d->addNewStroke(stroke);
    }

    setText(i18nc("(qtundo-format)", "Set stroke"));
}

KoShapeStrokeCommand::KoShapeStrokeCommand(const QList<KoShape*> &shapes,
        const QList<KoShapeStrokeModel*> &strokes,
        KUndo2Command *parent)
        : KUndo2Command(parent)
        , d(new Private())
{
    Q_ASSERT(shapes.count() == strokes.count());

    d->shapes = shapes;

    // save old strokes
    foreach(KoShape *shape, shapes)
        d->addOldStroke(shape->stroke());
    foreach (KoShapeStrokeModel * stroke, strokes)
        d->addNewStroke(stroke);

    setText(i18nc("(qtundo-format)", "Set stroke"));
}

KoShapeStrokeCommand::KoShapeStrokeCommand(KoShape* shape, KoShapeStrokeModel *stroke, KUndo2Command *parent)
        : KUndo2Command(parent)
        , d(new Private())
{
    d->shapes.append(shape);
    d->addNewStroke(stroke);
    d->addOldStroke(shape->stroke());

    setText(i18nc("(qtundo-format)", "Set stroke"));
}

KoShapeStrokeCommand::~KoShapeStrokeCommand()
{
    delete d;
}

void KoShapeStrokeCommand::redo()
{
    KUndo2Command::redo();
    QList<KoShapeStrokeModel*>::iterator strokeIt = d->newStrokes.begin();
    foreach(KoShape *shape, d->shapes) {
        shape->update();
        shape->setStroke(*strokeIt);
        shape->update();
        ++strokeIt;
    }
}

void KoShapeStrokeCommand::undo()
{
    KUndo2Command::undo();
    QList<KoShapeStrokeModel*>::iterator strokeIt = d->oldStrokes.begin();
    foreach(KoShape *shape, d->shapes) {
        shape->update();
        shape->setStroke(*strokeIt);
        shape->update();
        ++strokeIt;
    }
}
