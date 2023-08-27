/*
 *  SPDX-FileCopyrightText: 2023 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "SvgTextCursor.h"
#include "SvgTextTool.h"
#include <QTimer>
#include <QDebug>

struct Q_DECL_HIDDEN SvgTextCursor::Private {
    SvgTextTool *tool;
    int pos = 0;
    int anchor = 0;
    KoSvgTextShape *shape {nullptr};

    QTimer cursorFlash;
    QTimer cursorFlashLimit;
    bool cursorVisible = false;

    QPainterPath cursorShape;
    int cursorWidth = 1;
    QPainterPath selection;
};

SvgTextCursor::SvgTextCursor(SvgTextTool *tool) :
    d(new Private)
{
    d->tool = tool;
}

SvgTextCursor::~SvgTextCursor()
{
    d->cursorFlash.stop();
    d->shape = nullptr;
}

void SvgTextCursor::setShape(KoSvgTextShape *textShape)
{
    d->shape = textShape;
    d->pos = 0;
    d->anchor = 0;
    updateCursor();
}

void SvgTextCursor::setCaretSetting(int cursorWidth, int cursorFlash, int cursorFlashLimit)
{
    d->cursorFlash.setInterval(cursorFlash/2);
    d->cursorFlashLimit.setInterval(cursorFlashLimit);
    d->cursorWidth = cursorWidth;
    connect(&d->cursorFlash, SIGNAL(timeout()), this, SLOT(blinkCursor()));
    connect(&d->cursorFlashLimit, SIGNAL(timeout()), this, SLOT(stopBlinkCursor()));
}

int SvgTextCursor::getPos()
{
    return d->pos;
}

int SvgTextCursor::getAnchor()
{
    return d->anchor;
}

void SvgTextCursor::setPosToPoint(QPointF point)
{
    if (d->shape) {
        d->pos = d->shape->posForPoint(d->shape->documentToShape(point));
        d->anchor = d->pos;
        updateCursor();
    }
}

void SvgTextCursor::moveCursor(bool forward, bool moveAnchor)
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

void SvgTextCursor::insertText(QString text)
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

void SvgTextCursor::removeLast()
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

void SvgTextCursor::removeNext()
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

void SvgTextCursor::paintDecorations(QPainter &gc, QColor selectionColor)
{
    if (d->shape) {
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
}

void SvgTextCursor::blinkCursor()
{
    if (d->shape) {
        d->tool->updateCursor(d->shape->shapeToDocument(d->cursorShape.boundingRect()));
        d->cursorVisible = !d->cursorVisible;
    }
}

void SvgTextCursor::stopBlinkCursor()
{
    d->cursorFlash.stop();
    d->cursorFlashLimit.stop();
    d->cursorVisible = true;
    if (d->shape) {
        d->tool->updateCursor(d->shape->shapeToDocument(d->cursorShape.boundingRect()));
    }
}

bool SvgTextCursor::hasSelection()
{
    return d->pos != d->anchor;
}

void SvgTextCursor::updateCursor()
{
    d->cursorShape = d->shape->cursorForPos(d->pos);
    d->cursorFlash.start();
    d->cursorFlashLimit.start();
    d->cursorVisible = false;
    blinkCursor();
}

void SvgTextCursor::updateSelection()
{
    if (d->shape) {
        d->selection = d->shape->selectionBoxes(d->pos, d->anchor);
        d->tool->updateCursor(d->shape->shapeToDocument(d->cursorShape.boundingRect()));
    }
}
