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

#include "kis_animation_curves_value_ruler.h"

#include <QPaintEvent>
#include <QPainter>
#include <QApplication>

struct KisAnimationCurvesValueRuler::Private
{
    Private()
        : offset(-300)
        , scale(1.0)
    {}

    float offset;
    float scale;
};

KisAnimationCurvesValueRuler::KisAnimationCurvesValueRuler(QWidget *parent)
    : QWidget(parent)
    , m_d(new Private())
{}

KisAnimationCurvesValueRuler::~KisAnimationCurvesValueRuler()
{}

float KisAnimationCurvesValueRuler::scaleFactor() const
{
    return -m_d->scale;
}

float KisAnimationCurvesValueRuler::mapValueToView(float value) const
{
    return -m_d->offset - m_d->scale * value;
}

float KisAnimationCurvesValueRuler::mapViewToValue(float y) const
{
    return (-m_d->offset - y) / m_d->scale;
}

void KisAnimationCurvesValueRuler::setOffset(float offset)
{
    m_d->offset = offset;
}

float KisAnimationCurvesValueRuler::offset() const
{
    return m_d->offset;
}

QSize KisAnimationCurvesValueRuler::sizeHint() const
{
    return QSize(16, 0);
}

void KisAnimationCurvesValueRuler::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);

    float zeroLine = mapValueToView(0.0);
    QColor color = qApp->palette().color(QPalette::ButtonText);
    painter.setPen(QPen(color, 1));
    painter.drawLine(e->rect().left(), zeroLine, e->rect().right(), zeroLine);
}
