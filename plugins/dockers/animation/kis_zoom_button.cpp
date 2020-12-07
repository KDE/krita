/*
 *  Copyright (c) 2016 Jouni Pentikäinen <joupent@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
