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
    KoSvgTextShape *textShape = nullptr;
    KoShape* shape = nullptr;
    KoShapeContainer *originalShapeParent = nullptr;
    KoSvgTextShapeMementoSP memento;
    KoSvgTextAddRemoveShapeCommandImpl::ContourType type;
    bool removeCommand;
};

KoSvgTextAddRemoveShapeCommandImpl::KoSvgTextAddRemoveShapeCommandImpl(KoSvgTextShape *textShape, KoShape* shape, ContourType type, State state, KUndo2Command *parent)
    : KisCommandUtils::FlipFlopCommand(state, parent)
    , d(new Private(textShape, shape))
{
    d->removeCommand = state == FINALIZING;
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

// Remove shape from text.
void KoSvgTextAddRemoveShapeCommandImpl::partB()
{
    QRectF updateRectText = d->textShape->boundingRect();
    QRectF updateRectShape = d->shape->boundingRect();

    // Because "applyAbsoluteTransformation" applies ontop of the local transform, we want to get the minimal transfor difference.
    QTransform removeFromAbsolute = d->originalShapeParent? d->originalShapeParent->absoluteTransformation() * d->shape->transformation(): d->shape->transformation();
    QTransform absoluteTf = (d->shape->absoluteTransformation()*removeFromAbsolute.inverted());

    d->textShape->removeShapesFromContours({d->shape}, true);

    if (!d->removeCommand) {
        d->textShape->setMemento(d->memento);
        d->shape->setParent(d->originalShapeParent);
    } else {
        d->textShape->relayout();
    }
    d->shape->applyAbsoluteTransformation(absoluteTf);
    updateRectText |= d->textShape->boundingRect();
    updateRectShape |= d->shape->boundingRect();
    d->textShape->updateAbsolute(updateRectText);
    d->shape->updateAbsolute(updateRectShape);
}

// Add shape to text.
void KoSvgTextAddRemoveShapeCommandImpl::partA()
{
    QRectF updateRect = d->textShape->boundingRect();
    updateRect |= d->shape->boundingRect();
    if (!d->removeCommand) {
        d->originalShapeParent = d->shape->parent();
    }
    QTransform absoluteTf = (d->shape->absoluteTransformation()*d->shape->transformation().inverted()) * d->textShape->absoluteTransformation().inverted();

    if (d->type == Inside) {
        d->textShape->addShapeContours({d->shape}, true);
    } else if (d->type == Subtract) {
        d->textShape->addShapeContours({d->shape}, false);
    } // TextPath is handled by setMemento.

    if (d->removeCommand) {
        d->textShape->setMemento(d->memento);
    } else {
        d->textShape->relayout();
    }
    d->shape->applyAbsoluteTransformation(absoluteTf);
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
