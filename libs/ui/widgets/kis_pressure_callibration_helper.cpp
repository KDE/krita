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
#include <QTimer>

#include <klocalizedstring.h>

KisPressureCallibrationHelper::KisPressureCallibrationHelper(QWidget *parent) :
    QWidget(parent)
    , m_callibrationTime(new QTimer(this))
    , m_oldCaption(0)
    , m_callibrating(false)
{
    setMinimumHeight(256);
    m_callibrationTime->setInterval(10000);
    m_callibrationTime->setSingleShot(true);
    connect(m_callibrationTime, SIGNAL(timeout()), this, SLOT(finalizeCallibration()));

    QTimer *refresh = new QTimer(this);
    refresh->setInterval(1000);
    connect(refresh, SIGNAL(timeout()), this, SLOT(repaint()));
    QTimer *refreshCaption = new QTimer(this);
    connect(refreshCaption, SIGNAL(timeout()), this, SLOT(updateCaption()));
    refresh->start();
    refreshCaption->start(2000);
}

KisPressureCallibrationHelper::~KisPressureCallibrationHelper()
{

}

QList<QPointF> KisPressureCallibrationHelper::callibrationInfo()
{
    QList<QPointF> callibrationInfo;

    qreal lowest = 0.0;
    qreal average = 0.5;
    qreal highest = 1.0;

    if (!m_callibrationInfo.isEmpty()) {
        lowest = 1.0;
        highest = 0.0;

        for (int i = 0; i < m_callibrationInfo.size(); i++) {
            qreal pressure = m_callibrationInfo.at(i);
            if (pressure > highest) {
                highest = pressure;
            }
            if (pressure < lowest) {
                lowest = pressure;
            }
            average = (average + pressure) * 0.5;
        }


    }
    callibrationInfo.append(QPointF(lowest, 0.0));
    callibrationInfo.append(QPointF(average, 0.5));
    callibrationInfo.append(QPointF(highest, 1.0));
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

    if (m_oldCaption == 0) {
        m_caption = i18n("Please click to start callibrating tablet pressure.");
    }
    QString text = m_caption;
    int center = (w/2) - (this->fontMetrics().width(text)/2);
    QPoint o = QPoint(center, h/2);

    p.drawText(o, text);
    p.setRenderHint(QPainter::Antialiasing);


    if (m_callibrationTime->isActive()){
        p.setOpacity(0.3);
        qreal progress = int(m_callibrationTime->remainingTime()/1000);
        progress /= qreal(m_callibrationTime->interval()/1000);
        p.drawPie(QRect((w/2)-50, (h/2)+10, 100, 100), 0, int(5760*progress));
        p.setOpacity(1.0);
    }

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
            m_oldCaption = 0;
        }
    }
    if(e->type() == QEvent::TabletRelease) {
        m_currentPath.clear();
        return;
    }

    if(e->type() == QEvent::TabletPress || e->type() == QEvent::TabletMove) {
        if (!m_callibrationTime->isActive() && !m_callibrating) {
            m_callibrationInfo.clear();
            m_callibrationTime->start();
            m_callibrating = true;
            updateCaption();
        }
        if (m_callibrationTime->isActive()) {
            m_currentPath.append(e->pos());
        }
        repaint();
    }
}

void KisPressureCallibrationHelper::updateCaption()
{
    if (!m_callibrating || !m_callibrationTime->isActive()) return;

    int random = m_oldCaption;
    while (random == m_oldCaption) {
        random = QRandomGenerator::global()->bounded(1, 4);
    }

    switch(random) {
    case 1: {
        m_caption = i18n("Press as hard as you can.");
        break;
    }
    case 2: {
        m_caption = i18n("Draw a medium stroke.");
        break;
    }
    default: {
        m_caption = i18n("Press as lightly as you can.");
    }
    }

    m_oldCaption = random;
}

void KisPressureCallibrationHelper::finalizeCallibration()
{
    emit(callibrationDone());
    m_caption = "Done! Click again to start over.";
    QTimer::singleShot(1000, this, SLOT(resetEverything()));
}

void KisPressureCallibrationHelper::resetEverything()
{
    m_callibrationInfo.clear();
    m_currentPath.clear();
    m_oldCaption = 0;
    m_callibrating = false;
}
