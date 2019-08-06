/*
 *  Copyright (c) 2019 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
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

#include "kis_pressure_callibration_helper.h"

#include <QPainter>
#include <QPaintEvent>
#include <QApplication>

KisPressureCallibrationHelper::KisPressureCallibrationHelper(QWidget *parent) :
    QWidget(parent),
    m_highestValue(0.0),
    m_lowestValue(1.0),
    m_progress(WAITING)
{
    setMinimumHeight(256);
}

KisPressureCallibrationHelper::~KisPressureCallibrationHelper()
{

}

QList<QPointF> KisPressureCallibrationHelper::callibrationInfo()
{
    if (!m_callibrationInfo.isEmpty()) {
        return m_callibrationInfo;
    }
    QList<QPointF> dummy;
    dummy.append(QPointF(0.0, 0.0));
    dummy.append(QPointF(0.5, 0.5));
    dummy.append(QPointF(1.0, 1.0));
    return dummy;
}

void KisPressureCallibrationHelper::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e);
    const int w = width();
    const int h = height();
    QPainter p(this);
    p.fillRect(0, 0, w, h, QApplication::palette().background());
    p.setBrush(QApplication::palette().foreground());
    QString text = QString();
    switch (m_progress) {
    case WAITING:
        text = "Please click to start callibrating tablet pressure.";
    break;
    case LIGHT_STROKE:
        text = "Press as lightly as you can.";
    break;
    case MEDIUM_STROKE:
        text = "Draw a medium stroke.";
    break;
    case HEAVY_STROKE:
        text = "Press as hard as you hard.";
    break;
    case DONE:
        text = "Done! Click me to start again.";
    }
    int center = (w/2) - (this->fontMetrics().width(text)/2);
    QPoint o = QPoint(center, h/2);

    p.drawText(o, text);
    p.setBrush(Qt::transparent);
    p.setPen(QPen(QApplication::palette().foreground(), 2));
    p.drawPolyline(m_currentPath);
    p.drawRect(0, 0, w, h);
}

void KisPressureCallibrationHelper::tabletEvent(QTabletEvent *e)
{
    if(e->type() == QEvent::TabletRelease) {
        if (m_progress == LIGHT_STROKE) {
            m_callibrationInfo.append(QPointF(m_lowestValue, 0.0));
        } else if (m_progress == MEDIUM_STROKE) {
            m_callibrationInfo.append(QPointF((m_lowestValue+m_highestValue)/2, 0.5));
        } else if (m_progress == HEAVY_STROKE) {
            m_callibrationInfo.append(QPointF(m_highestValue, 1.0));
        } else {
            m_callibrationInfo.clear();
        }
        m_highestValue = 0.0;
        m_lowestValue = 1.0;
        nextSection();
        return;
    }
    if(e->type() == QEvent::TabletPress || e->type() == QEvent::TabletMove) {
        if (e->pressure() > m_highestValue) {
            m_highestValue = e->pressure();
        }
        if (e->pressure() < m_lowestValue) {
            m_lowestValue = e->pressure();
        }
        m_currentPath.append(e->pos());
        repaint();
    }


}

void KisPressureCallibrationHelper::nextSection()
{
    m_currentPath.clear();
    switch (m_progress) {
    case WAITING:
        m_progress = LIGHT_STROKE;
    break;
    case LIGHT_STROKE:
        m_progress = MEDIUM_STROKE;
    break;
    case MEDIUM_STROKE:
        m_progress = HEAVY_STROKE;
    break;
    case HEAVY_STROKE:
        m_progress = DONE;
        emit(callibrationDone());
    break;
    case DONE:
        m_progress = LIGHT_STROKE;
    break;
    }
    repaint();
}
