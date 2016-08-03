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
#include <QStyle>

const int MIN_LABEL_SEPARATION = 24;

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
    : QHeaderView(Qt::Vertical, parent)
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
    viewport()->update();
}

float KisAnimationCurvesValueRuler::offset() const
{
    return m_d->offset;
}

void KisAnimationCurvesValueRuler::setScale(float scale)
{
    m_d->scale = scale;
    viewport()->update();
}

QSize KisAnimationCurvesValueRuler::sizeHint() const
{
    return QSize(32, 0);
}

void KisAnimationCurvesValueRuler::paintEvent(QPaintEvent *e)
{
    QPalette palette = qApp->palette();
    QPainter painter(viewport());

    painter.fillRect(e->rect(), palette.color(QPalette::Button));

    QColor textColor = qApp->palette().color(QPalette::ButtonText);
    const QPen labelPen = QPen(textColor);

    QStyleOptionViewItemV4 option = viewOptions();
    const int gridHint = style()->styleHint(QStyle::SH_Table_GridLineColor, &option, this);
    const QColor gridColor = static_cast<QRgb>(gridHint);
    const QPen gridPen = QPen(gridColor);

    qreal minStep = MIN_LABEL_SEPARATION / m_d->scale;
    int minExp = ceil(log10(minStep));
    qreal majorStep = pow(10, minExp);
    qreal minorStep = 0.1 * majorStep;

    if (0.2 * majorStep * m_d->scale > MIN_LABEL_SEPARATION) {
        majorStep *= 0.2;
        minExp--;
    } else if (0.5 * majorStep * m_d->scale > MIN_LABEL_SEPARATION) {
        majorStep *= 0.5;
        minExp--;
    }

    qreal min = mapViewToValue(e->rect().bottom());
    qreal max = mapViewToValue(e->rect().top());
    qreal value = majorStep * floor(min/majorStep);

    while (value < max) {
        painter.setPen(gridPen);
        int y = mapValueToView(value);
        painter.drawLine(24, y, 32, y);

        qreal nextMajor = value + majorStep;
        while (value < nextMajor) {
            value += minorStep;
            int y = mapValueToView(value);
            painter.drawLine(30, y, 32, y);
        }

        painter.setPen(labelPen);
        const QString label = QString::number(value, 'f', qMax(0,-minExp));
        const QRect textRect = QRect(0, y, 30, MIN_LABEL_SEPARATION);
        painter.drawText(textRect, label, QTextOption(Qt::AlignRight));

        value = nextMajor;
    }
}
