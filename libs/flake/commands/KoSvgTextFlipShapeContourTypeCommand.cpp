/*
 * SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KoSvgTextFlipShapeContourTypeCommand.h"

#include <KoSvgTextShape.h>
#include <KoShapeBulkActionLock.h>


struct KoSvgTextFlipShapeContourTypeCommand::Private {
    Private(KoSvgTextShape *_text, KoShape *_shape)
        : textShape(_text)
        , shape(_shape)
        , isInside(textShape->shapesInside().contains(_shape))
    {
    }

    KoSvgTextShape *textShape;
    KoShape *shape;
    bool isInside = false;
};

KoSvgTextFlipShapeContourTypeCommand::KoSvgTextFlipShapeContourTypeCommand(KoSvgTextShape *textShape, KoShape *shape, KUndo2Command *parent)
    : KUndo2Command(parent)
    , d(new Private(textShape, shape))
{

}

KoSvgTextFlipShapeContourTypeCommand::~KoSvgTextFlipShapeContourTypeCommand()
{

}

void KoSvgTextFlipShapeContourTypeCommand::redo()
{
    KoShapeBulkActionLock lock(d->textShape);

    d->textShape->addShapeContours({d->shape}, !(d->isInside));

    KoShapeBulkActionLock::bulkShapesUpdate(lock.unlock());
}

void KoSvgTextFlipShapeContourTypeCommand::undo()
{
    KoShapeBulkActionLock lock(d->textShape);

    d->textShape->addShapeContours({d->shape}, d->isInside);

    KoShapeBulkActionLock::bulkShapesUpdate(lock.unlock());
}
