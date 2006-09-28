/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
 * Copyright (C) 2006 Peter Simonsson <peter.simonsson@gmail.com>
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

#include "KoCanvasController.h"

#include <kdebug.h>

#include <QGridLayout>
#include <QScrollBar>
#include <QEvent>

KoCanvasController::KoCanvasController(QWidget *parent)
: QScrollArea(parent)
, m_canvas(0)
, m_canvasWidget(0)
{
    m_viewport = new Viewport();
    setWidget(m_viewport);
    setWidgetResizable(true);
    setAutoFillBackground(false);

    connect(horizontalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(updateCanvasOffsetX()));
    connect(verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(updateCanvasOffsetY()));
}

void KoCanvasController::setCanvas(KoCanvasBase *canvas) {
    Q_ASSERT(canvas); // param is not null
    if(m_canvas) {
        emit canvasRemoved(this);
        m_viewport->removeCanvas(m_canvas->canvasWidget());
    }
    m_viewport->setCanvas(canvas->canvasWidget());
    m_canvas = canvas;
    m_canvas->canvasWidget()->installEventFilter(this);
    emit canvasSet(this);
}

KoCanvasBase* KoCanvasController::canvas() const {
    return m_canvas;
}

int KoCanvasController::visibleHeight() const {
    int height1;
    if(m_canvasWidget == 0)
        height1 = m_viewport->height();
    else
        height1 = qMin(m_viewport->height(), m_canvasWidget->height());
    int height2 = height();
    if(horizontalScrollBar() && horizontalScrollBar()->isVisible())
        height2 -= horizontalScrollBar()->height();
    return qMin(height1, height2);
}

int KoCanvasController::visibleWidth() const {
    int width1;
    if(m_canvasWidget == 0)
        width1 = m_viewport->width();
    else
        width1 = qMin(m_viewport->width(), m_canvasWidget->width());
    int width2 = width();
    if(verticalScrollBar() && verticalScrollBar()->isVisible())
        width2 -= verticalScrollBar()->width();
    return qMin(width1, width2);
}

void KoCanvasController::centerCanvas(bool centered) {
    m_centerCanvas = centered;
    m_viewport->centerCanvas(centered);
}

bool KoCanvasController::isCanvasCentered() const {
    return m_centerCanvas;
}

int KoCanvasController::canvasOffsetX() const {
    int offset = 0;

    if(m_canvas) {
        offset = m_canvas->canvasWidget()->x() + frameWidth();
    }

    if(horizontalScrollBar()) {
        offset -= horizontalScrollBar()->value();
    }

    return offset;
}

int KoCanvasController::canvasOffsetY() const {
    int offset = 0;

    if(m_canvas) {
        offset = m_canvas->canvasWidget()->y() + frameWidth();
    }

    if(verticalScrollBar()) {
        offset -= verticalScrollBar()->value();
    }

    return offset;
}

void KoCanvasController::updateCanvasOffsetX() {
    emit canvasOffsetXChanged(canvasOffsetX());
}

void KoCanvasController::updateCanvasOffsetY() {
    emit canvasOffsetYChanged(canvasOffsetY());
}

bool KoCanvasController::eventFilter(QObject* watched, QEvent* event) {
    if(m_canvas && m_canvas->canvasWidget() && (watched == m_canvas->canvasWidget())) {
        if((event->type() == QEvent::Resize) || event->type() == QEvent::Move) {
            updateCanvasOffsetX();
            updateCanvasOffsetY();
        }
    }

    return false;
}


// ********** Viewport **********
KoCanvasController::Viewport::Viewport()
: QWidget()
{
    setBackgroundRole(QPalette::Dark);
    setAutoFillBackground(true);
    m_layout = new QGridLayout(this);
    m_layout->setSpacing(0);
    m_layout->setMargin(0);
    m_layout->setRowStretch(1,1);
    centerCanvas(true);
}

void KoCanvasController::Viewport::setCanvas(QWidget *canvas) {
    canvas->setAutoFillBackground(true);
    m_layout->addWidget(canvas, 0, 1, Qt::AlignHCenter);
}

void KoCanvasController::Viewport::removeCanvas(QWidget *canvas) {
    m_layout->removeWidget(canvas);
}

void KoCanvasController::Viewport::centerCanvas(bool centered) {
    m_layout->setColumnStretch(0,centered?1:0);
    m_layout->setColumnStretch(1,1);
    m_layout->setColumnStretch(2,centered?1:2);
}

#include "KoCanvasController.moc"

// TODO add a paintEvent here and paint a nice shadow to the
// bottom/right of the canvas. Also paint an optional gray border
// around the image and make it possible to show the canvas widget
// rotated.

