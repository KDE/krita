/*
 *  SPDX-FileCopyrightText: 2023 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KoSvgTextCursor.h"
#include <QTimer>
#include <QDebug>

struct Q_DECL_HIDDEN KoSvgTextCursor::Private {
    int pos = 0;
    int anchor = 0;
    KoSvgTextShape *shape;

    QTimer cursorFlash;
    QTimer cursorFlashLimit;
    bool cursorVisible = false;

    QPainterPath cursorShape;
    int cursorWidth = 1;
    QPainterPath selection;
};

KoSvgTextCursor::KoSvgTextCursor(QObject *parent, KoSvgTextShape *textShape, int cursorWidth, int cursorFlash, int cursorFlashLimit) :
    QObject(parent)
    , d(new Private)
{
    d->shape = textShape;
    d->cursorFlash.setInterval(cursorFlash/2);
    d->cursorFlashLimit.setInterval(cursorFlashLimit);
    d->cursorWidth = cursorWidth;
    connect(&d->cursorFlash, SIGNAL(timeout()), this, SLOT(blinkCursor()));
    connect(&d->cursorFlashLimit, SIGNAL(timeout()), this, SLOT(stopBlinkCursor()));
    if (textShape) {
        d->cursorFlash.start(500);
    }
}

KoSvgTextCursor::~KoSvgTextCursor()
{
    d->cursorFlash.stop();
    d->shape = nullptr;
}

int KoSvgTextCursor::getPos()
{
    return d->pos;
}

int KoSvgTextCursor::getAnchor()
{
    return d->anchor;
}

void KoSvgTextCursor::setPosToPoint(QPointF point)
{
    if (d->shape) {
        d->pos = d->shape->posForPoint(d->shape->documentToShape(point));
        d->anchor = d->pos;
        updateCursor();
    }
}

void KoSvgTextCursor::moveCursor(bool forward, bool moveAnchor)
{
    if (forward) {
        d->pos = d->shape->nextPos(d->pos);
    } else {
        d->pos = d->shape->previousPos(d->pos);
    }
    if (moveAnchor) {
        d->anchor = d->pos;
    } else {
        updateSelection();
    }
    updateCursor();
}

void KoSvgTextCursor::insertText(QString text)
{
    if (d->shape) {
        //TODO: don't break when the position is zero...
        QRectF updateRect = d->shape->boundingRect();
        if (d->anchor != d->pos) {
            int start = qMin(d->anchor, d->pos);
            int length = qMax(d->anchor, d->pos) - start;
            if (d->shape->removeText(start, length)) {
                d->pos = start;
                d->anchor = d->pos;
            }
        }
        int index = d->shape->indexForPos(d->pos);
        if (d->shape->insertText(d->pos, text)) {
            d->pos = d->shape->posForIndex(index+text.size(), true);
            d->anchor = d->pos;
            updateCursor();
        }
        d->shape->updateAbsolute( updateRect| d->shape->boundingRect());
    }
}

void KoSvgTextCursor::removeLast()
{
    if (d->shape) {
        QRectF updateRect = d->shape->boundingRect();
        if (d->anchor != d->pos) {
            int start = qMin(d->anchor, d->pos);
            int length = qMax(d->anchor, d->pos) - start;
            if (d->shape->removeText(start, length)) {
                d->pos = start;
            }
        }
        int oldIndex = d->shape->indexForPos(d->pos);

        // TODO: When there's 'pre-anchor' positions, these need to be skipped, not yet sure how.
        int newPos = d->shape->posForIndex(oldIndex-1, true, true);
        int newIndex = d->shape->indexForPos(newPos);
        if (d->shape->removeText(newPos, oldIndex-newIndex)) {
            d->pos = newPos;
            d->anchor = d->pos;
            updateCursor();
        }
        d->shape->updateAbsolute( updateRect| d->shape->boundingRect());
    }
}

void KoSvgTextCursor::removeNext()
{
    if (d->shape) {
        QRectF updateRect = d->shape->boundingRect();
        if (d->anchor != d->pos) {
            int start = qMin(d->anchor, d->pos);
            int length = qMax(d->anchor, d->pos) - start;
            if (d->shape->removeText(start, length)) {
                d->pos = start;
            }
        }
        d->shape->removeText(d->pos, 1);
        d->anchor = d->pos;
        d->shape->updateAbsolute( updateRect| d->shape->boundingRect());
        updateCursor();
    }
}

void KoSvgTextCursor::paintDecorations(QPainter &gc, QColor selectionColor)
{
    gc.save();
    gc.setTransform(d->shape->absoluteTransformation(), true);
    if (d->pos != d->anchor) {
        gc.save();
        gc.setOpacity(0.5);
        QBrush brush(selectionColor);
        gc.fillPath(d->selection, brush);
        gc.restore();
    } else {
        if (d->cursorVisible) {
            QPen pen;
            pen.setCosmetic(true);
            pen.setColor(Qt::black);
            pen.setWidth(d->cursorWidth);
            gc.setPen(pen);
            gc.drawPath(d->cursorShape);

        }
    }
    gc.restore();
}

void KoSvgTextCursor::blinkCursor()
{
    if (d->shape) {
        emit(decorationsChanged(d->shape->shapeToDocument(d->cursorShape.boundingRect())));
        d->cursorVisible = !d->cursorVisible;
    }
}

void KoSvgTextCursor::stopBlinkCursor()
{
    d->cursorFlash.stop();
    d->cursorFlashLimit.stop();
    d->cursorVisible = true;
    emit(decorationsChanged(d->shape->shapeToDocument(d->cursorShape.boundingRect())));
}

void KoSvgTextCursor::updateCursor()
{
    d->cursorShape = d->shape->cursorForPos(d->pos);
    d->cursorFlash.start();
    d->cursorFlashLimit.start();
    d->cursorVisible = false;
    blinkCursor();
}

void KoSvgTextCursor::updateSelection()
{
    if (d->shape) {
        d->selection = d->shape->selectionBoxes(d->pos, d->anchor);
        emit(decorationsChanged(d->shape->shapeToDocument(d->selection.boundingRect())));
    }
}
