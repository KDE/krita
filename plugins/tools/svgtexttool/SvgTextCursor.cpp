/*
 *  SPDX-FileCopyrightText: 2023 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "SvgTextCursor.h"
#include "KoCanvasBase.h"
#include "SvgTextInsertCommand.h"
#include "SvgTextRemoveCommand.h"
#include "kundo2command.h"
#include <QTimer>
#include <QDebug>
#include <QClipboard>
#include <QMimeData>
#include <QApplication>

struct Q_DECL_HIDDEN SvgTextCursor::Private {
    KoCanvasBase *canvas;
    int pos = 0;
    int anchor = 0;
    KoSvgTextShape *shape {nullptr};

    QTimer cursorFlash;
    QTimer cursorFlashLimit;
    bool cursorVisible = false;

    QPainterPath cursorShape;
    QRectF oldCursorRect;
    int cursorWidth = 1;
    QPainterPath selection;
    QRectF oldSelectionRect;

    bool visualNavigation = true;
};

SvgTextCursor::SvgTextCursor(KoCanvasBase *canvas) :
    d(new Private)
{
    d->canvas = canvas;
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

void SvgTextCursor::setPos(int pos, int anchor)
{
    d->pos = pos;
    d->anchor = anchor;
    updateCursor();
}

void SvgTextCursor::setPosToPoint(QPointF point)
{
    if (d->shape) {
        d->pos = d->shape->posForPoint(d->shape->documentToShape(point));
        d->anchor = d->pos;
        updateCursor();
    }
}

void SvgTextCursor::moveCursor(MoveMode mode, bool moveAnchor)
{
    if (d->shape) {
        if (mode == MoveLeft) {
            d->pos = d->shape->posLeft(d->pos, d->visualNavigation);
        } else if (mode == MoveRight) {
            d->pos = d->shape->posRight(d->pos, d->visualNavigation);
        } else if (mode == MoveUp) {
            d->pos = d->shape->posUp(d->pos, d->visualNavigation);
        } else if (mode == MoveDown) {
            d->pos = d->shape->posDown(d->pos, d->visualNavigation);
        } else if (mode == MoveWordLeft) {
            int pos = d->shape->wordLeft(d->pos, d->visualNavigation);
            if (pos == d->pos) {
                pos = d->shape->posLeft(d->pos, false);
                d->pos = d->shape->wordLeft(pos, d->visualNavigation);
            } else {
                d->pos = pos;
            }
        } else if (mode == MoveWordRight) {
            int pos = d->shape->wordRight(d->pos, d->visualNavigation);
            if (pos == d->pos) {
                pos = d->shape->posRight(d->pos, false);
                d->pos = d->shape->wordRight(pos, d->visualNavigation);
            } else {
                d->pos = pos;
            }
        }  else if (mode == MoveLineStart) {
            d->pos = d->shape->lineStart(d->pos);
        } else if (mode == MoveLineEnd) {
            d->pos = d->shape->lineEnd(d->pos);
        } else if (mode == ParagraphStart) {
            d->pos = 0;
        } else if (mode == ParagraphEnd) {
            d->pos = d->shape->posForIndex(d->shape->plainText().size());
        }
        if (moveAnchor) {
            d->anchor = d->pos;
        } else {
            updateSelection();
        }
        updateCursor();
    }
}

void SvgTextCursor::insertText(QString text)
{

    if (d->shape) {
        //KUndo2Command *parentCmd = new KUndo2Command;
        if (hasSelection()) {
            SvgTextRemoveCommand *removeCmd = removeSelection();
            addCommandToUndoAdapter(removeCmd);
        }

        SvgTextInsertCommand *insertCmd = new SvgTextInsertCommand(d->shape, d->pos, d->anchor, text, this);
        addCommandToUndoAdapter(insertCmd);

    }
}

void SvgTextCursor::removeLast()
{
    if (d->shape) {
        SvgTextRemoveCommand *removeCmd;
        if (hasSelection()) {
            removeCmd = removeSelection();
            addCommandToUndoAdapter(removeCmd);
            updateCursor();
        } else {
            int oldIndex = d->shape->indexForPos(d->pos);
            int newPos = d->shape->posForIndex(oldIndex-1, false, false);
            int newIndex = d->shape->indexForPos(newPos);
            // using old-new index allows us to remove the whole grapheme.
            removeCmd = new SvgTextRemoveCommand(d->shape, newPos, d->pos, d->anchor, oldIndex-newIndex, this);
            addCommandToUndoAdapter(removeCmd);
        }
    }
}

void SvgTextCursor::removeNext()
{
    if (d->shape) {
        SvgTextRemoveCommand *removeCmd;
        if (hasSelection()) {
            removeCmd = removeSelection();
            addCommandToUndoAdapter(removeCmd);
            updateCursor();
        } else {
            int oldIndex = d->shape->indexForPos(d->pos);
            int newIndex = d->shape->indexForPos(d->pos+1);
            if (newIndex == oldIndex) {
                newIndex = d->shape->indexForPos(d->pos+2);
            }
            removeCmd = new SvgTextRemoveCommand(d->shape, d->pos, d->pos, d->anchor, newIndex-oldIndex, this);
            addCommandToUndoAdapter(removeCmd);
        }
    }
}

SvgTextRemoveCommand *SvgTextCursor::removeSelection(KUndo2Command *parent)
{
    SvgTextRemoveCommand *removeCmd = nullptr;
    if (d->shape) {
        if (d->anchor != d->pos) {
            int start = qMin(d->anchor, d->pos);
            int length = d->shape->indexForPos(qMax(d->anchor, d->pos)) - d->shape->indexForPos(start);
            d->pos = start;
            d->anchor = d->pos;
            removeCmd = new SvgTextRemoveCommand(d->shape, start, d->pos, d->anchor, length, this, parent);
        }
    }
    return removeCmd;
}

void SvgTextCursor::copy() const
{
    if (d->shape) {
        int start = d->shape->indexForPos(qMin(d->anchor, d->pos));
        int length = d->shape->indexForPos(qMax(d->anchor, d->pos)) - start;
        QString copied = d->shape->plainText().mid(start, length);
        QClipboard *cb = QApplication::clipboard();
        cb->setText(copied);
    }
}

bool SvgTextCursor::paste()
{
    bool success = false;
    if (d->shape) {
        QClipboard *cb = QApplication::clipboard();
        const QMimeData *mimeData = cb->mimeData();
        if (mimeData->hasText()) {
            insertText(mimeData->text());
            success = true;
        }
    }
    return success;
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
        emit updateCursorDecoration(d->shape->shapeToDocument(d->cursorShape.boundingRect()) | d->oldCursorRect);
        d->cursorVisible = !d->cursorVisible;
    }
}

void SvgTextCursor::stopBlinkCursor()
{
    d->cursorFlash.stop();
    d->cursorFlashLimit.stop();
    d->cursorVisible = true;
    if (d->shape) {
        emit updateCursorDecoration(d->shape->shapeToDocument(d->cursorShape.boundingRect()) | d->oldCursorRect);
    }
}

bool SvgTextCursor::hasSelection()
{
    return d->pos != d->anchor;
}

void SvgTextCursor::updateCursor()
{
    if (d->shape) {
        d->oldCursorRect = d->shape->shapeToDocument(d->cursorShape.boundingRect());
    }
    d->cursorShape = d->shape? d->shape->cursorForPos(d->pos): QPainterPath();
    d->cursorFlash.start();
    d->cursorFlashLimit.start();
    d->cursorVisible = false;
    blinkCursor();
}

void SvgTextCursor::updateSelection()
{
    if (d->shape) {
        d->oldSelectionRect = d->shape->shapeToDocument(d->selection.boundingRect());
        d->selection = d->shape->selectionBoxes(d->pos, d->anchor);
        emit updateCursorDecoration(d->shape->shapeToDocument(d->selection.boundingRect()) | d->oldSelectionRect);
    }
}

void SvgTextCursor::addCommandToUndoAdapter(KUndo2Command *cmd)
{
    if (d->canvas) {
        if (cmd) {
            d->canvas->addCommand(cmd);
        }
    }
}
