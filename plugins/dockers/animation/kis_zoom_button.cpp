/*
 *  Copyright (c) 2016 Jouni Pentikäinen <joupent@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_zoom_button.h"

#include <cmath>
#include <QMouseEvent>


KisZoomButton::KisZoomButton(QWidget *parent)
    : KisDraggableToolButton(parent)
{
    connect(this, &KisZoomButton::valueChanged,
            this, &KisZoomButton::slotValueChanged);
}

KisZoomButton::~KisZoomButton()
{}

qreal KisZoomButton::zoomLevel() const
{
    return m_zoomLevel;
}

void KisZoomButton::setZoomLevel(qreal level)
{
    m_zoomLevel = level;
}

void KisZoomButton::beginZoom(QPoint mousePos, qreal staticPoint)
{
    m_initialDragZoomLevel = m_zoomLevel;
    beginDrag(mousePos);
    emit zoomStarted(staticPoint);
}

void KisZoomButton::continueZoom(QPoint mousePos)
{
    int delta = continueDrag(mousePos);
    slotValueChanged(delta);
}

void KisZoomButton::mousePressEvent(QMouseEvent *e)
{
    beginZoom(e->pos(), qQNaN());
}

void KisZoomButton::slotValueChanged(int value)
{
    qreal zoomCoeff = std::pow(2.0, qreal(value) / unitRadius());
    m_zoomLevel = m_initialDragZoomLevel * zoomCoeff;

    emit zoomLevelChanged(m_zoomLevel);
}
