/*
 *  SPDX-FileCopyrightText: 2016 Jouni Pentikäinen <joupent@gmail.com>
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

void KisZoomButton::slotValueChanged(int value)
{
    const int unitRadius = 200;

    emit zoom(qreal(value) / unitRadius);
}
