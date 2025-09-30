/*
 * SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KoSvgTextAddRemoveShapeCommands.h"

#include <KoSvgTextShape.h>
#include <KoShapeContainer.h>

struct KoSvgTextAddRemoveShapeCommandImpl::Private {
    Private(KoSvgTextShape *_text, KoShape *_shape)
        : textShape(_text)
        , shape(_shape)
        , memento(textShape->getMemento())
    {

    }

    ~Private() {}
    KoSvgTextShape *textShape;
    KoShape* shape;
    KoSvgTextShapeMementoSP memento;
    KoSvgTextAddRemoveShapeCommandImpl::ContourType type;
    bool removeCommand;
};

KoSvgTextAddRemoveShapeCommandImpl::KoSvgTextAddRemoveShapeCommandImpl(KoSvgTextShape *textShape, KoShape* shape, ContourType type, State state, KUndo2Command *parent)
    : KisCommandUtils::FlipFlopCommand(state, parent)
    , d(new Private(textShape, shape))
{
    if (type == Unknown) {
        if (d->textShape->shapesInside().contains(shape)) {
            d->type = Inside;
        } else if (d->textShape->shapesSubtract().contains(shape)) {
            d->type = Subtract;
        } else if (d->textShape->shapeInContours(shape)){
            d->type = TextPath;
        } else {
            d->type = Unknown;
        }
    } else {
        d->type = type;
    }
}

KoSvgTextAddRemoveShapeCommandImpl::~KoSvgTextAddRemoveShapeCommandImpl()
{

}

void KoSvgTextAddRemoveShapeCommandImpl::partB()
{
    QRectF updateRect = d->textShape->boundingRect();
    updateRect |= d->shape->boundingRect();
    d->textShape->removeShapesFromContours({d->shape}, true);
    if (!d->removeCommand) {
        d->textShape->setMemento(d->memento);
    } else {
        d->textShape->relayout();
    }
    updateRect |= d->textShape->boundingRect();
    updateRect |= d->shape->boundingRect();
    d->textShape->updateAbsolute(updateRect);
    d->shape->updateAbsolute(updateRect);
}

void KoSvgTextAddRemoveShapeCommandImpl::partA()
{
    QRectF updateRect = d->textShape->boundingRect();
    updateRect |= d->shape->boundingRect();
    if (d->type == Inside) {
        d->textShape->addShapeContours({d->shape}, true);
    } else if (d->type == Subtract) {
        d->textShape->addShapeContours({d->shape}, false);
    }
    if (d->removeCommand) {
        d->textShape->setMemento(d->memento);
    } else {
        d->textShape->relayout();
    }
    updateRect |= d->textShape->boundingRect();
    updateRect |= d->shape->boundingRect();
    d->shape->updateAbsolute(updateRect);
    d->textShape->updateAbsolute(updateRect);
}

KoSvgTextAddShapeCommand::KoSvgTextAddShapeCommand(KoSvgTextShape *textShape, KoShape *shape, bool inside, KUndo2Command *parentCommand)
    : KoSvgTextAddRemoveShapeCommandImpl(textShape, shape, (inside? Inside: Subtract), INITIALIZING, parentCommand)
{

}

KoSvgTextAddShapeCommand::~KoSvgTextAddShapeCommand()
{

}

KoSvgTextRemoveShapeCommand::KoSvgTextRemoveShapeCommand(KoSvgTextShape *textShape, KoShape *shape, KUndo2Command *parentCommand)
: KoSvgTextAddRemoveShapeCommandImpl(textShape, shape, Unknown, FINALIZING, parentCommand)
{

}

KoSvgTextRemoveShapeCommand::~KoSvgTextRemoveShapeCommand()
{

}
