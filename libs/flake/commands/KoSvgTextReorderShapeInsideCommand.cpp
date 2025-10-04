/*
 * SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KoSvgTextReorderShapeInsideCommand.h"
#include <KoSvgTextShape.h>


struct KoSvgTextReorderShapeInsideCommand::Private {
    Private(KoSvgTextShape *_text, QList<KoShape *> _shapes, KoSvgTextReorderShapeInsideCommand::MoveShapeType _type)
        : textShape(_text)
        , memento(textShape->getMemento())
        , shapes(_shapes)
        , type(_type)
    {
        std::sort(shapes.begin(), shapes.end(), [this](KoShape *a, KoShape *b)
        {
            return this->textShape->shapesInside().indexOf(a) < this->textShape->shapesInside().indexOf(b);
        });
        Q_FOREACH(KoShape *shape, shapes) {
            oldIndices.append(textShape->shapesInside().indexOf(shape));
        }
    }

    KoSvgTextShape *textShape;
    KoSvgTextShapeMementoSP memento;
    QList<KoShape *>  shapes;
    QList<int> oldIndices;
    KoSvgTextReorderShapeInsideCommand::MoveShapeType type;
};

KoSvgTextReorderShapeInsideCommand::KoSvgTextReorderShapeInsideCommand(KoSvgTextShape *textShape, QList<KoShape *> shapes, MoveShapeType type, KUndo2Command *parent)
    : KUndo2Command(parent)
    , d(new Private(textShape, shapes, type))
{

}

KoSvgTextReorderShapeInsideCommand::~KoSvgTextReorderShapeInsideCommand()
{

}

void KoSvgTextReorderShapeInsideCommand::redo()
{
    QRectF updateRect = d->textShape->boundingRect();

    int newIndex = d->textShape->shapesInside().indexOf(d->shapes.first());
    const int max = (d->textShape->shapesInside().size() -1);
    if (d->type == MoveEarlier || d->type == BringToFront) {
        if (d->type == MoveEarlier) {
            newIndex = qMax(0, newIndex - 1);
        } else {
            newIndex = 0;
        }
        Q_FOREACH(KoShape *shape, d->shapes) {
            const int index = d->textShape->shapesInside().indexOf(shape);
            if (index == newIndex) continue;

            d->textShape->moveShapeInsideToIndex(shape, newIndex);
            newIndex += 1;
        }
    } else {
        if (d->type == MoveLater) {
            newIndex = qMin(max, newIndex + d->shapes.size());
        } else {
            newIndex = max;
        }
        auto end = std::make_reverse_iterator(d->shapes.begin());
        auto begin = std::make_reverse_iterator(d->shapes.end());
        for (auto it = begin; it != end; it++) {
            KoShape *shape = *it;

            const int index = d->textShape->shapesInside().indexOf(shape);
            if (index == newIndex) continue;
            if (newIndex < 0) continue;

            d->textShape->moveShapeInsideToIndex(shape, newIndex);
            newIndex -= 1;
        }
    }

    updateRect |= d->textShape->boundingRect();
    d->textShape->updateAbsolute(updateRect);
}

void KoSvgTextReorderShapeInsideCommand::undo()
{
    QRectF updateRect = d->textShape->boundingRect();
    if (d->type == MoveEarlier || d->type == BringToFront) {
        for (int i = d->oldIndices.size() -1 ; i>= 0; i--) {
            d->textShape->moveShapeInsideToIndex(d->shapes.at(i), d->oldIndices.at(i));
        }
    } else {
        for (int i = 0; i< d->oldIndices.size(); i++) {
            d->textShape->moveShapeInsideToIndex(d->shapes.at(i), d->oldIndices.at(i));
        }
    }
    d->textShape->setMemento(d->memento);
    updateRect |= d->textShape->boundingRect();
    d->textShape->updateAbsolute(updateRect);
}
