/* This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2006 Thomas Zander <zander@kde.org>
 * SPDX-FileCopyrightText: 2006-2008 Jan Hambrecht <jaham@gmx.net>
 * SPDX-FileCopyrightText: 2012 Inge Wallin <inge@lysator.liu.se>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KoShapeStrokeCommand.h"
#include "KoShape.h"
#include "KoShapeStrokeModel.h"

#include <klocalizedstring.h>

#include "kis_command_ids.h"


class Q_DECL_HIDDEN KoShapeStrokeCommand::Private
{
public:
    Private() {}
    ~Private()
    {
    }

    void addOldStroke(KoShapeStrokeModelSP oldStroke)
    {
        oldStrokes.append(oldStroke);
    }

    void addNewStroke(KoShapeStrokeModelSP newStroke)
    {
        newStrokes.append(newStroke);
    }

    QList<KoShape*> shapes;                ///< the shapes to set stroke for
    QList<KoShapeStrokeModelSP> oldStrokes; ///< the old strokes, one for each shape
    QList<KoShapeStrokeModelSP> newStrokes; ///< the new strokes to set
};

KoShapeStrokeCommand::KoShapeStrokeCommand(const QList<KoShape*> &shapes, KoShapeStrokeModelSP stroke, KUndo2Command *parent)
    : KUndo2Command(parent)
    , d(new Private())
{
    d->shapes = shapes;

    // save old strokes
    Q_FOREACH (KoShape *shape, d->shapes) {
        d->addOldStroke(shape->stroke());
        d->addNewStroke(stroke);
    }

    setText(kundo2_i18n("Set stroke"));
}

KoShapeStrokeCommand::KoShapeStrokeCommand(const QList<KoShape*> &shapes,
        const QList<KoShapeStrokeModelSP> &strokes,
        KUndo2Command *parent)
        : KUndo2Command(parent)
        , d(new Private())
{
    Q_ASSERT(shapes.count() == strokes.count());

    d->shapes = shapes;

    // save old strokes
    Q_FOREACH (KoShape *shape, shapes)
        d->addOldStroke(shape->stroke());
    foreach (KoShapeStrokeModelSP stroke, strokes)
        d->addNewStroke(stroke);

    setText(kundo2_i18n("Set stroke"));
}

KoShapeStrokeCommand::KoShapeStrokeCommand(KoShape* shape, KoShapeStrokeModelSP stroke, KUndo2Command *parent)
        : KUndo2Command(parent)
        , d(new Private())
{
    d->shapes.append(shape);
    d->addNewStroke(stroke);
    d->addOldStroke(shape->stroke());

    setText(kundo2_i18n("Set stroke"));
}

KoShapeStrokeCommand::~KoShapeStrokeCommand()
{
    delete d;
}

void KoShapeStrokeCommand::redo()
{
    KUndo2Command::redo();
    QList<KoShapeStrokeModelSP>::iterator strokeIt = d->newStrokes.begin();
    Q_FOREACH (KoShape *shape, d->shapes) {
        const QRectF oldDirtyRect = shape->boundingRect();
        shape->setStroke(*strokeIt);
        shape->updateAbsolute(oldDirtyRect | shape->boundingRect());
        ++strokeIt;
    }
}

void KoShapeStrokeCommand::undo()
{
    KUndo2Command::undo();
    QList<KoShapeStrokeModelSP>::iterator strokeIt = d->oldStrokes.begin();
    Q_FOREACH (KoShape *shape, d->shapes) {
        const QRectF oldDirtyRect = shape->boundingRect();
        shape->setStroke(*strokeIt);
        shape->updateAbsolute(oldDirtyRect | shape->boundingRect());
        ++strokeIt;
    }
}

int KoShapeStrokeCommand::id() const
{
    return KisCommandUtils::ChangeShapeStrokeId;
}

bool KoShapeStrokeCommand::mergeWith(const KUndo2Command *command)
{
    const KoShapeStrokeCommand *other = dynamic_cast<const KoShapeStrokeCommand*>(command);

    if (!other ||
        other->d->shapes != d->shapes) {

        return false;
    }

    d->newStrokes = other->d->newStrokes;
    return true;
}
