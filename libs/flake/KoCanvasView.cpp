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

#include "KoCanvasView.h"

#include <QGridLayout>
#include <QDebug>


KoCanvasView::KoCanvasView(QWidget *parent)
: QScrollArea(parent)
, m_canvas(0)
{
    m_viewport = new Viewport();
    setWidget(m_viewport);
    setWidgetResizable(true);
    setAutoFillBackground(false);
}

void KoCanvasView::setCanvas(KoCanvasBase *canvas) {
    Q_ASSERT(canvas); // param is not null
    if(m_canvas) {
        emit canvasRemoved(this);
        m_viewport->removeCanvas(m_canvas->canvasWidget());
    }
    m_viewport->setCanvas(canvas->canvasWidget());
    m_canvas = canvas;
    emit canvasSet(this);
}

KoCanvasBase* KoCanvasView::canvas() const {
    return m_canvas;
}

int KoCanvasView::visibleHeight() const {
    return qMin(m_viewport->width(), m_canvasWidget->width());
}

int KoCanvasView::visibleWidth() const {
    return qMin(m_viewport->height(), m_canvasWidget->height());
}

void KoCanvasView::centerCanvas(bool centered) {
    m_centerCanvas = centered;
    m_viewport->centerCanvas(centered);
}

bool KoCanvasView::isCanvasCentered() const {
    return m_centerCanvas;
}

int KoCanvasView::canvasOffsetX() const {
    return 0; // TODO
}

int KoCanvasView::canvasOffsetY() const {
    return 0; // TODO
}

// ********** Viewport **********
KoCanvasView::Viewport::Viewport()
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

void KoCanvasView::Viewport::setCanvas(QWidget *canvas) {
    canvas->setAutoFillBackground(true);
    m_layout->addWidget(canvas, 0, 1, Qt::AlignHCenter);
}

void KoCanvasView::Viewport::removeCanvas(QWidget *canvas) {
    m_layout->removeWidget(canvas);
}

void KoCanvasView::Viewport::centerCanvas(bool centered) {
    m_layout->setColumnStretch(0,centered?1:0);
    m_layout->setColumnStretch(1,1);
    m_layout->setColumnStretch(2,centered?1:2);
}

#include "KoCanvasView.moc"

// TODO add a paintEvent here and paint a nice shadow to the bottom/right of the canvas
