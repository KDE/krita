/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <KoTextTool.h>
#include <KoCanvasBase.h>
#include <KoSelection.h>
#include <KoShapeManager.h>

#include <kdebug.h>
#include <QKeyEvent>

KoTextTool::KoTextTool(KoCanvasBase *canvas)
: KoTool(canvas)
, m_textShape(0)
{
}

KoTextTool::~KoTextTool() {
}

void KoTextTool::paint( QPainter &painter, KoViewConverter &converter) {
}

void KoTextTool::mousePressEvent( KoPointerEvent *event )  {
}

void KoTextTool::mouseDoubleClickEvent( KoPointerEvent *event ) {
}

void KoTextTool::mouseMoveEvent( KoPointerEvent *event ) {
}

void KoTextTool::mouseReleaseEvent( KoPointerEvent *event ) {
}

void KoTextTool::keyPressEvent(QKeyEvent *event) {
    QTextCursor::MoveOperation moveOperation = QTextCursor::NoMove;
    switch(event->key()) { // map input to moveOperation
        case Qt::Key_Backspace:
            m_caret.deletePreviousChar();
            break;
        case Qt::Key_Tab:
            kDebug() << "Tab key pressed";
            break;
        case Qt::Key_Delete:
            m_caret.deleteChar();
            break;
        case Qt::Key_Left:
            if(event->modifiers() & Qt::ControlModifier)
                moveOperation = QTextCursor::WordLeft;
            else
                moveOperation = QTextCursor::Left;
            break;
        case Qt::Key_Up:
            moveOperation = QTextCursor::Up;
            break;
        case Qt::Key_Right:
            if(event->modifiers() & Qt::ControlModifier)
                moveOperation = QTextCursor::NextWord;
            else
                moveOperation = QTextCursor::Right;
            break;
        case Qt::Key_Down:
            moveOperation = QTextCursor::Down;
            break;
        case Qt::Key_PageUp:
            //moveOperation = QTextCursor::Left;
            break;
        case Qt::Key_PageDown:
            //moveOperation = QTextCursor::Left;
            break;
        case Qt::Key_End:
            moveOperation = QTextCursor::EndOfLine;
            break;
        case Qt::Key_Home:
            moveOperation = QTextCursor::StartOfLine;
            break;
        default:
            m_caret.insertText(event->text());
    }
    m_caret.movePosition(moveOperation,
        (event->modifiers() & Qt::ShiftModifier)?QTextCursor::KeepAnchor:QTextCursor::MoveAnchor);
    event->accept();
    m_textShape->repaint();// TODO more fine grained repainting..
}

void KoTextTool::keyReleaseEvent(QKeyEvent *event) {
    event->accept();
}

void KoTextTool::activate (bool temporary) {
    Q_UNUSED(temporary);
    KoShape *shape = m_canvas->shapeManager()->selection()->firstSelectedObject();
    m_textShape = dynamic_cast<KoTextShape*> (shape);
    if(m_textShape == 0) {
        emit sigDone();
        return;
    }
    KoTextShapeData *data = static_cast<KoTextShapeData*> (shape->userData());
    m_caret = QTextCursor(data->document());
    useCursor(Qt::IBeamCursor, true);
}

void KoTextTool::deactivate() {
kDebug(32500) << "  deactivate" << endl;
    m_textShape = 0;
}
