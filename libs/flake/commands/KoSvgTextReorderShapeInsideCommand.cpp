/*
 * SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KoSvgTextReorderShapeInsideCommand.h"
#include <KoSvgTextShape.h>

struct KoSvgTextReorderShapeInsideCommand::Private {
    Private(KoSvgTextShape *_text, KoShape *_shape, KoSvgTextReorderShapeInsideCommand::MoveShapeType _type)
        : textShape(_text)
        , memento(textShape->getMemento())
        , shape(_shape)
        , type(_type)
    {
    }

    KoSvgTextShape *textShape;
    KoSvgTextShapeMementoSP memento;
    KoShape *shape;
    int oldIndex = 0;
    KoSvgTextReorderShapeInsideCommand::MoveShapeType type;
};

KoSvgTextReorderShapeInsideCommand::KoSvgTextReorderShapeInsideCommand(KoSvgTextShape *textShape, KoShape *shape, MoveShapeType type, KUndo2Command *parent)
    : KUndo2Command(parent)
    , d(new Private(textShape, shape, type))
{

}

KoSvgTextReorderShapeInsideCommand::~KoSvgTextReorderShapeInsideCommand()
{

}

void KoSvgTextReorderShapeInsideCommand::redo()
{
    QRectF updateRect = d->textShape->boundingRect();

    d->oldIndex = d->textShape->shapesInside().indexOf(d->shape);
    int newIndex = d->oldIndex;
    const int max = d->textShape->shapesInside().size();
    if (d->type == MoveEarlier) {
        newIndex = qMax(0, d->oldIndex - 1);
    } else if (d->type == MoveLater) {
        newIndex = qMin(max, d->oldIndex + 1);
    } else if (d->type == BringToFront) {
        newIndex = 0;
    } else /*if (type == SendToBack)*/ {
        newIndex = max;
    }
    d->textShape->moveShapeInsideToIndex(d->shape, newIndex);
    updateRect |= d->textShape->boundingRect();
    d->textShape->updateAbsolute(updateRect);
}

void KoSvgTextReorderShapeInsideCommand::undo()
{
    QRectF updateRect = d->textShape->boundingRect();
    d->textShape->moveShapeInsideToIndex(d->shape, d->oldIndex);
    d->textShape->setMemento(d->memento);
    updateRect |= d->textShape->boundingRect();
    d->textShape->updateAbsolute(updateRect);
}
