/* This file is part of the KDE project
 * Copyright (C) 2010 KO GmbH <boud@kogmbh.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (  at your option ) any later version.
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
#include "KoCanvasControllerGraphicsWidget.h"

#include <QCursor>
#include <QRect>
#include <QRectF>
#include <QSize>
#include <QSizeF>
#include <QPoint>
#include <QPointF>

class KoCanvasControllerGraphicsWidget::Private
{
public:

    Private(KoCanvasControllerGraphicsWidget *qq)
        : q(qq),
        canvas(0),
        ignoreScrollSignals(false),
        zoomWithWheel(false),
        vastScrollingFactor(0)
    {
    }

    /**
     * Gets called by the tool manager if this canvas controller is the current active canvas controller.
     */
    void setDocumentOffset();

    void resetScrollBars();
    void emitPointerPositionChangedSignals(QEvent *event);

    void activate();

    KoCanvasControllerGraphicsWidget *q;
    KoCanvasBase * canvas;
    bool ignoreScrollSignals;
    bool zoomWithWheel;
    qreal vastScrollingFactor;
    QPointF m_oldPosition;
    int m_margin;
    bool m_ignoreScrollSignals;
};

KoCanvasControllerGraphicsWidget::KoCanvasControllerGraphicsWidget(QGraphicsItem * parent, Qt::WindowFlags wFlags)
    : QGraphicsWidget(parent, wFlags)
    , d(new Private(this))
{
}

KoCanvasControllerGraphicsWidget::~KoCanvasControllerGraphicsWidget()
{
}


void KoCanvasControllerGraphicsWidget::scrollContentsBy(int dx, int dy)
{
}

QSize KoCanvasControllerGraphicsWidget::viewportSize() const
{
}

void KoCanvasControllerGraphicsWidget::setDrawShadow(bool drawShadow)
{
}

void KoCanvasControllerGraphicsWidget::setCanvas(KoCanvasBase *canvas)
{
}

KoCanvasBase *KoCanvasControllerGraphicsWidget::canvas() const
{
}

int KoCanvasControllerGraphicsWidget::visibleHeight() const
{
}

int KoCanvasControllerGraphicsWidget::visibleWidth() const
{
}

int KoCanvasControllerGraphicsWidget::canvasOffsetX() const
{
}

int KoCanvasControllerGraphicsWidget::canvasOffsetY() const
{
}

void KoCanvasControllerGraphicsWidget::ensureVisible(const QRectF &rect, bool smooth)
{
}

void KoCanvasControllerGraphicsWidget::ensureVisible(KoShape *shape)
{
}

void KoCanvasControllerGraphicsWidget::zoomIn(const QPoint &center)
{
}

void KoCanvasControllerGraphicsWidget::zoomOut(const QPoint &center)
{
}

void KoCanvasControllerGraphicsWidget::zoomBy(const QPoint &center, qreal zoom)
{
}

void KoCanvasControllerGraphicsWidget::zoomTo(const QRect &rect)
{
}

void KoCanvasControllerGraphicsWidget::recenterPreferred()
{
}

void KoCanvasControllerGraphicsWidget::setPreferredCenter(const QPoint &viewPoint)
{
}

QPoint KoCanvasControllerGraphicsWidget::preferredCenter() const
{
}

void KoCanvasControllerGraphicsWidget::pan(const QPoint &distance)
{
}

void KoCanvasControllerGraphicsWidget::setMargin(int margin)
{
}

QPoint KoCanvasControllerGraphicsWidget::scrollBarValue() const
{
}

void KoCanvasControllerGraphicsWidget::setScrollBarValue(const QPoint &value)
{
}

void KoCanvasControllerGraphicsWidget::updateDocumentSize(const QSize &sz, bool recalculateCenter)
{
}

QCursor KoCanvasControllerGraphicsWidget::setCursor(const QCursor &cursor)
{
}

void KoCanvasControllerGraphicsWidget::setZoomWithWheel(bool)
{
}

void KoCanvasControllerGraphicsWidget::setVastScrolling(qreal)
{
}

