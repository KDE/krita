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
#include <QRandomGenerator>

KisPressureCallibrationHelper::KisPressureCallibrationHelper(QWidget *parent) :
    QWidget(parent),
    m_callibrationTime(new QTimer(this))
{
    setMinimumHeight(256);
    m_callibrationTime->setInterval(10000);
    m_caption = "Please click to start callibrating tablet pressure.";
    connect(m_callibrationTime, &QTimer::timeout, this, SIGNAL(callibrationDone()));
}

KisPressureCallibrationHelper::~KisPressureCallibrationHelper()
{

}

QList<QPointF> KisPressureCallibrationHelper::callibrationInfo()
{
    QList<QPointF> callibrationInfo;

    if (!m_callibrationInfo.isEmpty()) {
        //return m_callibrationInfo;
    } else {
        //Dummyvalues to avoid crashes.
        callibrationInfo.append(QPointF(0.0, 0.0));
        callibrationInfo.append(QPointF(0.5, 0.5));
        callibrationInfo.append(QPointF(1.0, 1.0));
    }
    return callibrationInfo;
}

void KisPressureCallibrationHelper::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e);
    const int w = width();
    const int h = height();
    QPainter p(this);
    p.fillRect(0, 0, w, h, QApplication::palette().background());
    p.setBrush(QApplication::palette().foreground());
    QString text = m_caption;
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
    m_callibrationInfo.append(e->pressure());
    if(e->type() == QEvent::TabletLeaveProximity) {
        if (!m_callibrationTime->isActive()) {
            m_caption = "Please click to start callibrating tablet pressure.";
        }
    }
    if(e->type() == QEvent::TabletRelease) {
        m_currentPath.clear();
        if (m_callibrationTime->isActive()) {
            UpdateCaption();
        } else {
            m_caption = "Done! Click again to start over.";
        }
        return;
    }

    if(e->type() == QEvent::TabletPress || e->type() == QEvent::TabletMove) {
        if (!m_callibrationTime->isActive()) {
            m_callibrationInfo.clear();
            m_callibrationTime->start();
        }
        m_currentPath.append(e->pos());
        repaint();
    }
}

void KisPressureCallibrationHelper::UpdateCaption()
{
    int random = QRandomGenerator::global()->bounded(0, 2);
    while (random == m_oldCaption) {
        random = QRandomGenerator::global()->bounded(0, 2);
    }

    switch(random) {
    case 1: {
        m_caption = "Press as hard as you can.";
        break;
    }
    case 2: {
        m_caption = "Draw a medium stroke.";
        break;
    }
    default: {
        m_caption = "Press as lightly as you can.";
    }
    }

    m_oldCaption = random;
}
