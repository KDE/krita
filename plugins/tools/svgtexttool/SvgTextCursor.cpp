/*
 *  SPDX-FileCopyrightText: 2023 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "SvgTextCursor.h"
#include "KoCanvasBase.h"
#include "KoSvgTextProperties.h"
#include "SvgTextInsertCommand.h"
#include "SvgTextRemoveCommand.h"
#include "kundo2command.h"
#include <QTimer>
#include <QDebug>
#include <QClipboard>
#include <QMimeData>
#include <QApplication>
#include <QKeyEvent>
#include <QKeySequence>
#include <QAction>
#include <kis_assert.h>

struct Q_DECL_HIDDEN SvgTextCursor::Private {
    KoCanvasBase *canvas;
    bool isAddingCommand = false;
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

    // This is used to adjust cursorpositions better on text-shape relayouts.
    int posIndex = 0;
    int anchorIndex = 0;

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
    if (d->shape) {
        d->shape->removeShapeChangeListener(this);
    }
    d->shape = textShape;
    if (d->shape) {
        d->shape->addShapeChangeListener(this);
    }
    d->pos = 0;
    d->anchor = 0;
    updateCursor();
    updateSelection();
}

void SvgTextCursor::setCaretSetting(int cursorWidth, int cursorFlash, int cursorFlashLimit)
{
    d->cursorFlash.setInterval(cursorFlash/2);
    d->cursorFlashLimit.setInterval(cursorFlashLimit);
    d->cursorWidth = cursorWidth;
    connect(&d->cursorFlash, SIGNAL(timeout()), this, SLOT(blinkCursor()));
    connect(&d->cursorFlashLimit, SIGNAL(timeout()), this, SLOT(stopBlinkCursor()));
}

void SvgTextCursor::setVisualMode(bool visualMode)
{
    d->visualNavigation = visualMode;
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
    updateSelection();
}

void SvgTextCursor::setPosToPoint(QPointF point, bool moveAnchor)
{
    if (d->shape) {
        d->pos = d->shape->posForPoint(d->shape->documentToShape(point));
        if (moveAnchor) {
            d->anchor = d->pos;
        }
        updateSelection();
        updateCursor();
        updateSelection();
    }
}

void SvgTextCursor::moveCursor(MoveMode mode, bool moveAnchor)
{
    if (d->shape) {
        if (mode == MoveWordLeft || mode == MoveWordRight) {
            int pos = moveModeResult(mode, d->pos, d->visualNavigation);
            if (pos == d->pos) {
                MoveMode shift = mode == MoveWordLeft? MoveLeft: MoveRight;
                pos = moveModeResult(shift, pos, false);
                d->pos = moveModeResult(mode, pos, d->visualNavigation);
            } else {
                d->pos = pos;
            }
        } else if (mode == MoveWordStart || mode == MoveWordEnd) {
            int pos = moveModeResult(mode, d->pos, d->visualNavigation);
            if (pos == d->pos) {
                MoveMode shift = mode == MoveWordStart? MovePreviousChar: MoveNextChar;
                pos = moveModeResult(shift, pos, false);
                d->pos = moveModeResult(mode, pos, d->visualNavigation);
            } else {
                d->pos = pos;
            }
        } else {
            d->pos = moveModeResult(mode, d->pos, d->visualNavigation);
        }
        if (moveAnchor) {
            d->anchor = d->pos;
        }
        updateSelection();
        updateCursor();
    }
}

void SvgTextCursor::insertText(QString text)
{

    if (d->shape) {
        //KUndo2Command *parentCmd = new KUndo2Command;
        if (hasSelection()) {
            SvgTextRemoveCommand *removeCmd = removeSelectionImpl();
            addCommandToUndoAdapter(removeCmd);
        }

        SvgTextInsertCommand *insertCmd = new SvgTextInsertCommand(d->shape, d->pos, d->anchor, text);
        addCommandToUndoAdapter(insertCmd);

    }
}

void SvgTextCursor::removeText(SvgTextCursor::MoveMode first, SvgTextCursor::MoveMode second)
{
    if (d->shape) {
        SvgTextRemoveCommand *removeCmd;
        if (hasSelection()) {
            removeCmd = removeSelectionImpl();
            addCommandToUndoAdapter(removeCmd);
        } else {
            int posA = moveModeResult(first, d->pos, d->visualNavigation);
            int posB = moveModeResult(second, d->pos, d->visualNavigation);

            int posStart = qMin(posA, posB);
            int posEnd = qMax(posA, posB);
            int length = d->shape->indexForPos(posEnd) - d->shape->indexForPos(posStart);

            removeCmd = new SvgTextRemoveCommand(d->shape, posStart, d->pos, d->anchor, length);
            addCommandToUndoAdapter(removeCmd);
        }
    }
}

void SvgTextCursor::removeSelection()
{
    KUndo2Command *removeCmd = removeSelectionImpl();
    addCommandToUndoAdapter(removeCmd);
}

SvgTextRemoveCommand *SvgTextCursor::removeSelectionImpl(KUndo2Command *parent)
{
    SvgTextRemoveCommand *removeCmd = nullptr;
    if (d->shape) {
        if (d->anchor != d->pos) {
            int start = qMin(d->anchor, d->pos);
            int length = d->shape->indexForPos(qMax(d->anchor, d->pos)) - d->shape->indexForPos(start);
            removeCmd = new SvgTextRemoveCommand(d->shape, start, d->pos, d->anchor, length, parent);
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

void SvgTextCursor::deselectText()
{
    setPos(d->pos, d->pos);
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

void SvgTextCursor::notifyShapeChanged(KoShape::ChangeType type, KoShape *shape)
{
    Q_UNUSED(type);
    Q_UNUSED(shape);
    if (d->shape->indexForPos(d->pos) != d->posIndex) {
        d->pos = d->shape->posForIndex(d->posIndex);
    }
    if (d->shape->indexForPos(d->anchor) != d->anchorIndex) {
        d->anchor = d->shape->posForIndex(d->anchorIndex);
    }
    updateCursor();
    updateSelection();
}

void SvgTextCursor::notifyCursorPosChanged(int pos, int anchor)
{
    d->pos = pos;
    d->anchor = anchor;
    updateCursor();
    updateSelection();
}

void SvgTextCursor::keyPressEvent(QKeyEvent *event)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(d->shape);

    bool select = event->modifiers().testFlag(Qt::ShiftModifier);

    if (!((Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier) & event->modifiers())) {

        switch (event->key()) {
        case Qt::Key_Right:
            moveCursor(SvgTextCursor::MoveRight, !select);
            event->accept();
            break;
        case Qt::Key_Left:
            moveCursor(SvgTextCursor::MoveLeft, !select);
            event->accept();
            break;
        case Qt::Key_Up:
            moveCursor(SvgTextCursor::MoveUp, !select);
            event->accept();
            break;
        case Qt::Key_Down:
            moveCursor(SvgTextCursor::MoveDown, !select);
            event->accept();
            break;
        case Qt::Key_Delete:
            removeText(MoveNone, MoveNextChar);
            event->accept();
            break;
        case Qt::Key_Backspace:
            removeText(MovePreviousChar, MoveNone);
            event->accept();
            break;
        case Qt::Key_Return:
        case Qt::Key_Enter:
            insertText("\n");
            event->accept();
            break;
        default:
            event->ignore();
        }

        if (event->isAccepted()) {
            return;
        }
    }
    if (acceptableInput(event)) {
        insertText(event->text());
        event->accept();
        return;
    }

    KoSvgTextProperties props = d->shape->textProperties();

    KoSvgText::WritingMode mode = KoSvgText::WritingMode(props.propertyOrDefault(KoSvgTextProperties::WritingModeId).toInt());
    KoSvgText::Direction direction = KoSvgText::Direction(props.propertyOrDefault(KoSvgTextProperties::DirectionId).toInt());

    // Qt's keysequence stuff doesn't handle vertical, so to test all the standard keyboard shortcuts as if it did,
    // we reinterpret the direction keys according to direction and writing mode, and test against that.

    int newKey = event->key();

    if (direction == KoSvgText::DirectionRightToLeft) {
        switch (newKey) {
        case Qt::Key_Left:
            newKey = Qt::Key_Right;
            break;
        case Qt::Key_Right:
            newKey = Qt::Key_Left;
            break;
        default:
            break;
        }
    }

    if (mode == KoSvgText::VerticalRL) {
        switch (newKey) {
        case Qt::Key_Left:
            newKey = Qt::Key_Down;
            break;
        case Qt::Key_Right:
            newKey = Qt::Key_Up;
            break;
        case Qt::Key_Up:
            newKey = Qt::Key_Left;
            break;
        case Qt::Key_Down:
            newKey = Qt::Key_Right;
            break;
        default:
            break;
        }
    } else if (mode == KoSvgText::VerticalRL) {
        switch (newKey) {
        case Qt::Key_Left:
            newKey = Qt::Key_Up;
            break;
        case Qt::Key_Right:
            newKey = Qt::Key_Down;
            break;
        case Qt::Key_Up:
            newKey = Qt::Key_Left;
            break;
        case Qt::Key_Down:
            newKey = Qt::Key_Right;
            break;
        default:
            break;
        }
    }

    QKeySequence testSequence(event->modifiers() | newKey);


    // Note for future, when we have format changing actions:
    // We'll need to test format change actions before the standard
    // keys, as one of the standard keys for deleting a line is ctrl+u
    // which would probably be expected to do underline before deleting.

    // This first set is already tested above, however, if they still
    // match, then it's one of the extra sequences for MacOs, which
    // seem to be purely logical, instead of the visual set we tested
    // above.
    if (testSequence == QKeySequence::MoveToNextChar) {
        moveCursor(SvgTextCursor::MoveNextChar, true);
        event->accept();
    } else if (testSequence == QKeySequence::SelectNextChar) {
        moveCursor(SvgTextCursor::MoveNextChar, false);
        event->accept();
    } else if (testSequence == QKeySequence::MoveToPreviousChar) {
        moveCursor(SvgTextCursor::MovePreviousChar, true);
        event->accept();
    } else if (testSequence == QKeySequence::SelectPreviousChar) {
        moveCursor(SvgTextCursor::MovePreviousChar, false);
        event->accept();
    } else if (testSequence == QKeySequence::MoveToNextLine) {
        moveCursor(SvgTextCursor::MoveNextLine, true);
        event->accept();
    } else if (testSequence == QKeySequence::SelectNextLine) {
        moveCursor(SvgTextCursor::MoveNextLine, false);
        event->accept();
    } else if (testSequence == QKeySequence::MoveToPreviousLine) {
        moveCursor(SvgTextCursor::MovePreviousLine, true);
        event->accept();
    } else if (testSequence == QKeySequence::SelectPreviousLine) {
        moveCursor(SvgTextCursor::MovePreviousLine, false);
        event->accept();

    } else if (testSequence == QKeySequence::MoveToNextWord) {
        moveCursor(SvgTextCursor::MoveWordEnd, true);
        event->accept();
    } else if (testSequence == QKeySequence::SelectNextWord) {
        moveCursor(SvgTextCursor::MoveWordEnd, false);
        event->accept();
    } else if (testSequence == QKeySequence::MoveToPreviousWord) {
        moveCursor(SvgTextCursor::MoveWordStart, true);
        event->accept();
    } else if (testSequence == QKeySequence::SelectPreviousWord) {
        moveCursor(SvgTextCursor::MoveWordStart, false);
        event->accept();

    } else if (testSequence == QKeySequence::MoveToStartOfLine) {
        moveCursor(SvgTextCursor::MoveLineStart, true);
        event->accept();
    } else if (testSequence == QKeySequence::SelectStartOfLine) {
        moveCursor(SvgTextCursor::MoveLineStart, false);
        event->accept();
    } else if (testSequence == QKeySequence::MoveToEndOfLine) {
        moveCursor(SvgTextCursor::MoveLineEnd, true);
        event->accept();
    } else if (testSequence == QKeySequence::SelectEndOfLine) {
        moveCursor(SvgTextCursor::MoveLineEnd, false);
        event->accept();

    } else if (testSequence == QKeySequence::MoveToStartOfBlock
               || testSequence == QKeySequence::MoveToStartOfDocument) {
        moveCursor(SvgTextCursor::ParagraphStart, true);
        event->accept();
    } else if (testSequence == QKeySequence::SelectStartOfBlock
               || testSequence == QKeySequence::SelectStartOfDocument) {
        moveCursor(SvgTextCursor::ParagraphStart, false);
        event->accept();

    } else if (testSequence == QKeySequence::MoveToEndOfBlock
               || testSequence == QKeySequence::MoveToEndOfDocument) {
        moveCursor(SvgTextCursor::ParagraphEnd, true);
        event->accept();
    } else if (testSequence == QKeySequence::SelectEndOfBlock
               || testSequence == QKeySequence::SelectEndOfDocument) {
        moveCursor(SvgTextCursor::ParagraphEnd, false);
        event->accept();

    }else if (testSequence == QKeySequence::DeleteStartOfWord) {
        removeText(MoveWordStart, MoveNone);
        event->accept();
    } else if (testSequence == QKeySequence::DeleteEndOfWord) {
        removeText(MoveNone, MoveWordEnd);
        event->accept();
    } else if (testSequence == QKeySequence::DeleteEndOfLine) {
        removeText(MoveNone, MoveLineEnd);
        event->accept();
    } else if (testSequence == QKeySequence::DeleteCompleteLine) {
        removeText(MoveLineStart, MoveLineEnd);
        event->accept();
    } else if (testSequence == QKeySequence::Backspace) {
        removeText(MovePreviousChar, MoveNone);
        event->accept();
    } else if (testSequence == QKeySequence::Delete) {
        removeText(MoveNone, MoveNextChar);
        event->accept();

    } else if (testSequence == QKeySequence::InsertLineSeparator
               || testSequence == QKeySequence::InsertParagraphSeparator) {
        insertText("\n");
        event->accept();
    } else {
        event->ignore();
    }
}

bool SvgTextCursor::isAddingCommand() const
{
    return d->isAddingCommand;
}

void SvgTextCursor::updateCursor()
{
    if (d->shape) {
        d->oldCursorRect = d->shape->shapeToDocument(d->cursorShape.boundingRect());
        d->posIndex = d->shape->indexForPos(d->pos);
        d->anchorIndex = d->shape->indexForPos(d->anchor);
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
            d->isAddingCommand = true;
            d->canvas->addCommand(cmd);
            d->isAddingCommand = false;
        }
    }
}

int SvgTextCursor::moveModeResult(SvgTextCursor::MoveMode &mode, int &pos, bool visual) const
{
    int newPos = pos;
    switch (mode) {
        case MoveNone:
        break;
    case MoveLeft:
        newPos = d->shape->posLeft(pos, visual);
        break;
    case MoveRight:
        newPos = d->shape->posRight(pos, visual);
        break;
    case MoveUp:
        newPos = d->shape->posUp(pos, visual);
        break;
    case MoveDown:
        newPos = d->shape->posDown(pos, visual);
        break;
    case MovePreviousChar:
        newPos = d->shape->previousCluster(pos);
        break;
    case MoveNextChar:
        newPos = d->shape->nextCluster(pos);
        break;
    case MovePreviousLine:
        newPos = d->shape->previousLine(pos);
        break;
    case MoveNextLine:
        newPos = d->shape->nextLine(pos);
        break;
    case MoveWordLeft:
        newPos = d->shape->wordLeft(pos, visual);
        break;
    case MoveWordRight:
        newPos = d->shape->wordRight(pos, visual);
        break;
    case MoveWordStart:
        newPos = d->shape->wordStart(pos);
        break;
    case MoveWordEnd:
        newPos = d->shape->wordEnd(pos);
        break;
    case MoveLineStart:
        newPos = d->shape->lineStart(pos);
        break;
    case MoveLineEnd:
        newPos = d->shape->lineEnd(pos);
        break;
    case ParagraphStart:
        newPos = 0;
        break;
    case ParagraphEnd:
        newPos = d->shape->posForIndex(d->shape->plainText().size());
        break;
    }
    return newPos;
}
/// More or less copied from bool QInputControl::isAcceptableInput(const QKeyEvent *event) const
bool SvgTextCursor::acceptableInput(const QKeyEvent *event) const
{
    const QString text = event->text();
    if (text.isEmpty())
        return false;
    const QChar c = text.at(0);
    // Formatting characters such as ZWNJ, ZWJ, RLM, etc. This needs to go before the
    // next test, since CTRL+SHIFT is sometimes used to input it on Windows.
    if (c.category() == QChar::Other_Format)
        return true;
    // QTBUG-35734: ignore Ctrl/Ctrl+Shift; accept only AltGr (Alt+Ctrl) on German keyboards
    if (event->modifiers() == Qt::ControlModifier
            || event->modifiers() == (Qt::ShiftModifier | Qt::ControlModifier)) {
        return false;
    }
    if (c.isPrint())
        return true;
    if (c.category() == QChar::Other_PrivateUse)
        return true;
    if (c == QLatin1Char('\t'))
        return true;
    return false;
}
