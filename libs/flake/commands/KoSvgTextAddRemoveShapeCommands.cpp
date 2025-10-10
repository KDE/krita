/*
 * SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KoSvgTextAddRemoveShapeCommands.h"
#include "kis_assert.h"
#include <optional>
#include "kis_debug.h"

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
    std::optional<KoShapeContainer*> originalShapeParent;
    KoSvgTextShapeMementoSP memento;
    KoSvgTextAddRemoveShapeCommandImpl::ContourType type;
    bool removeCommand;
};

KoSvgTextAddRemoveShapeCommandImpl::KoSvgTextAddRemoveShapeCommandImpl(KoSvgTextShape *textShape, KoShape* shape, ContourType type, State state, KUndo2Command *parent)
    : KisCommandUtils::FlipFlopCommand(state, parent)
    , d(new Private(textShape, shape))
{
    d->removeCommand = state == FINALIZING;

    if (!d->removeCommand) {
        d->originalShapeParent = shape->parent();
    }

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

    KoShapeContainer *newParent = d->originalShapeParent.has_value() ? d->originalShapeParent.value() : d->textShape->parent();
    const QTransform newParentTransform = newParent ? newParent->absoluteTransformation() : QTransform();
    const QTransform absoluteTf = newParentTransform.inverted() * d->textShape->absoluteTransformation();

    d->textShape->removeShapesFromContours({d->shape}, true);
    // by this time d->shape->parent() is set to nullptr

    if (newParent) {
        // set new parent if exists
        d->shape->setParent(newParent);
    }

    if (!d->removeCommand) {
        d->textShape->setMemento(d->memento);
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

    KoShapeContainer *oldParent = d->shape->parent();
    const QTransform oldParentTransform = oldParent ? oldParent->absoluteTransformation() : QTransform();
    const QTransform absoluteTf = d->textShape->absoluteTransformation().inverted() * oldParentTransform;

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
    /**
     * 1) The shapes should belong to the same parent or have no parent at all.
     * 2) The parent of the text shape is always preserved (null or non-null)
     * 3) The parent of the contour shape is dropped, since it is put into
     *    a virtual group created by the text shape
     */
    KIS_SAFE_ASSERT_RECOVER_NOOP(!textShape->parent() || !shape->parent() || textShape->parent() == shape->parent());
}

KoSvgTextAddShapeCommand::~KoSvgTextAddShapeCommand()
{

}

KoSvgTextRemoveShapeCommand::KoSvgTextRemoveShapeCommand(KoSvgTextShape *textShape, KoShape *shape, KUndo2Command *parentCommand)
: KoSvgTextAddRemoveShapeCommandImpl(textShape, shape, Unknown, FINALIZING, parentCommand)
{
    // the \p shape will be ungrouped into the parent of \p textShape
    KIS_SAFE_ASSERT_RECOVER_NOOP(textShape->shapeInContours(shape));
}

KoSvgTextRemoveShapeCommand::~KoSvgTextRemoveShapeCommand()
{

}
