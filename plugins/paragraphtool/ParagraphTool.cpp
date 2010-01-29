/* This file is part of the KDE project
 * Copyright (C) 2008 Florian Merz <florianmerz@gmx.de>
 * Copyright (C) 2009 Carlos Licea <carlos.licea@kdemail.net>
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

#include "ParagraphTool.h"
#include "dialogs/OptionWidget.h"

#include <KoCanvasBase.h>
#include <KoPointerEvent.h>

#include <KDebug>

#include <QKeyEvent>

class KoViewConverter;

class QPainter;

ParagraphTool::ParagraphTool(KoCanvasBase *canvas)
        : KoToolBase(canvas),
        m_paragraphEditor(this, canvas),
        m_paragraphHighlighter(this, canvas)
{}

ParagraphTool::~ParagraphTool()
{}

QWidget *ParagraphTool::createOptionWidget()
{
    OptionWidget *optionWidget = new OptionWidget();

    connect(&m_paragraphEditor, SIGNAL(styleNameChanged(const QString&)), optionWidget, SLOT(setStyleName(const QString &)));

    connect(&m_paragraphEditor, SIGNAL(smoothMovementChanged(bool)), optionWidget, SLOT(setSmoothMovement(bool)));
    connect(optionWidget, SIGNAL(smoothMovementChanged(bool)), &m_paragraphEditor, SLOT(setSmoothMovement(bool)));

    return optionWidget;
}

void ParagraphTool::paint(QPainter &painter, const KoViewConverter &converter)
{
    m_paragraphEditor.paint(painter, converter);

    if (m_paragraphHighlighter.textBlock() != m_paragraphEditor.textBlock()) {
        m_paragraphHighlighter.paint(painter, converter);
    }
}

void ParagraphTool::repaintDecorations()
{
    QRectF repaintRectangle;
    repaintRectangle |= m_paragraphEditor.dirtyRectangle(true); //true: update the whole region not just the dirty part
    repaintRectangle |= m_paragraphHighlighter.dirtyRectangle();

    canvas()->updateCanvas(repaintRectangle);
}

void ParagraphTool::repaintDecorationsInternal()
{
    QRectF repaintRectangle;
    if (m_paragraphEditor.needsRepaint()) {
        repaintRectangle |= m_paragraphEditor.dirtyRectangle();
    }
    if (m_paragraphHighlighter.needsRepaint()) {
        repaintRectangle |= m_paragraphHighlighter.dirtyRectangle();
    }

    if (repaintRectangle != QRectF()) {
        canvas()->updateCanvas(repaintRectangle);
    }
}

void ParagraphTool::mousePressEvent(KoPointerEvent *event)
{
    m_mousePosition = event->point;

    if (event->button() == Qt::LeftButton) {

        bool activated = m_paragraphEditor.activateRulerAt(m_mousePosition);

        if (!activated) {
            if (m_paragraphEditor.hasFocusedRuler()) {
                m_paragraphEditor.defocusRuler();
            } else {
                m_paragraphEditor.deactivateTextBlock();
                m_paragraphEditor.activateTextBlockAt(m_mousePosition);
            }
        }
    } else {
        if (event->button() == Qt::RightButton) {
            m_paragraphEditor.resetActiveRuler();
        } else if (event->button() == Qt::MidButton) {
            m_paragraphEditor.applyParentStyleToActiveRuler();
        }

        m_paragraphEditor.deactivateRuler();
        m_paragraphEditor.highlightRulerAt(m_mousePosition);
    }

    repaintDecorationsInternal();
}

void ParagraphTool::mouseReleaseEvent(KoPointerEvent *event)
{
    m_mousePosition = event->point;

    if (m_paragraphEditor.hasActiveTextBlock() &&
            m_paragraphEditor.hasActiveRuler()) {
        m_paragraphEditor.deactivateRuler();
    }

    repaintDecorationsInternal();
}

void ParagraphTool::mouseMoveEvent(KoPointerEvent *event)
{
    m_mousePosition = event->point;

    m_paragraphHighlighter.activateTextBlockAt(m_mousePosition);

    if (m_paragraphEditor.hasActiveTextBlock()) {
        if (m_paragraphEditor.hasActiveRuler()) {
            m_paragraphEditor.moveActiveRulerTo(m_mousePosition);
        } else {
            m_paragraphEditor.highlightRulerAt(m_mousePosition);
        }

    }

    repaintDecorationsInternal();
}

void ParagraphTool::keyPressEvent(QKeyEvent *event)
{
    if (m_paragraphEditor.hasActiveRuler()) {
        switch (event->key()) {
        case Qt::Key_Shift:
            if (!event->isAutoRepeat()) {
                m_paragraphEditor.toggleSmoothMovement();
            }
            break;
        case Qt::Key_Escape:
            m_paragraphEditor.resetActiveRuler();
            m_paragraphEditor.deactivateRuler();
            m_paragraphEditor.highlightRulerAt(m_mousePosition);
            break;
        case Qt::Key_Delete:
        case Qt::Key_Backspace:
            m_paragraphEditor.applyParentStyleToActiveRuler();
            break;
        default:
            break;
        }
    } else if (m_paragraphEditor.hasFocusedRuler()) {
        switch (event->key()) {
        case Qt::Key_Plus:
            m_paragraphEditor.focusedRuler().increaseByStep();
            break;
        case Qt::Key_Minus:
            m_paragraphEditor.focusedRuler().decreaseByStep();
            break;
        case Qt::Key_PageUp:
            break;
        case Qt::Key_PageDown:
            break;
        case Qt::Key_Tab:
            if (!m_paragraphEditor.focusNextRuler()) {
                if (m_paragraphEditor.activateNextTextBlock()) {
                    m_paragraphEditor.focusFirstRuler();
                }
            }
            break;
        default:
            break;
        }
    } else {
        switch (event->key()) {
        case Qt::Key_Tab:
            m_paragraphEditor.focusRuler(topMarginRuler);
            break;
        default:
            break;
        }
    }

    repaintDecorationsInternal();
}

void ParagraphTool::keyReleaseEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Shift)
        m_paragraphEditor.toggleSmoothMovement();
}

void  ParagraphTool::activate(bool)
{
    useCursor(Qt::ArrowCursor);
}

void ParagraphTool::deactivate()
{
    // the document might have changed, so we have to deactivate the textblock
    m_paragraphEditor.deactivateTextBlock();
}

