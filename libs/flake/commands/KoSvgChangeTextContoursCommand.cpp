/*
 * SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <KoShape.h>
#include <KoSvgTextShape.h>

#include "KoSvgChangeTextContoursCommand.h"

struct KoSvgChangeTextContoursCommand::Private {
    Private(KoSvgTextShape *_text, QList<KoShape*> _shapes, bool _inside)
        : textShape(_text)
        , newShapes(_shapes)
        , memento(textShape->getMemento())
        , inside(_inside)
    {
        oldShapes = inside? textShape->shapesInside(): textShape->shapesSubtract();
    }
    ~Private()
    {

    }
    KoSvgTextShape *textShape;
    QList<KoShape*> oldShapes;
    QList<KoShape*> newShapes;
    KoSvgTextShapeMementoSP memento;
    bool inside;
};

KoSvgChangeTextContoursCommand::KoSvgChangeTextContoursCommand(KoSvgTextShape *textShape, QList<KoShape*> shapes, bool inside, KUndo2Command *parent)
    : KUndo2Command(parent)
    , d(new Private(textShape, shapes, inside))
{
    setText(kundo2_i18n("Change Text Contours"));
}

KoSvgChangeTextContoursCommand::~KoSvgChangeTextContoursCommand()
{

}

void KoSvgChangeTextContoursCommand::redo()
{
    QRectF updateRect = d->textShape->boundingRect();
    if (d->inside) {
        d->textShape->setShapesInside(d->newShapes);
    } else {
        d->textShape->setShapesSubtract(d->newShapes);
    }
    updateRect |= d->textShape->boundingRect();
    d->textShape->updateAbsolute(updateRect);
}

void KoSvgChangeTextContoursCommand::undo()
{
    QRectF updateRect = d->textShape->boundingRect();
    if (d->inside) {
        d->textShape->setShapesInside(d->oldShapes);
    } else {
        d->textShape->setShapesSubtract(d->oldShapes);
    }
    d->textShape->setMemento(d->memento);
    updateRect |= d->textShape->boundingRect();
    d->textShape->updateAbsolute(updateRect);
}
