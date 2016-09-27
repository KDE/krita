/*
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
 *                2016 Sven Langkamp <sven.langkamp@gmail.com>
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

#include "widgets/kis_stopgradient_slider_widget.h"
#include <QPainter>
#include <QPixmap>
#include <QMouseEvent>
#include <QPolygon>
#include <QPaintEvent>

#define MARGIN 10
#define HANDLE_SIZE 20

KisStopGradientSliderWidget::KisStopGradientSliderWidget(QWidget *parent, Qt::WFlags f)
    : QWidget(parent, f)
    , m_selectedStop(0)
    , m_drag(0)
{
    setMinimumHeight(30);
}

void KisStopGradientSliderWidget::setGradientResource(KoStopGradient* gradient)
{
    m_gradient = gradient;

    emit sigSelectedStop(m_selectedStop);
}

void KisStopGradientSliderWidget::paintEvent(QPaintEvent* pe)
{
    QWidget::paintEvent(pe);
    QPainter painter(this);
    painter.fillRect(rect(), palette().background());
    painter.setPen(Qt::black);
    painter.drawRect(MARGIN, MARGIN, width() - 2 * MARGIN, height() - 2 * MARGIN - HANDLE_SIZE);
    if (m_gradient) {
        QImage image = m_gradient->generatePreview(width() - 2 * MARGIN - 2, height() - 2 * MARGIN - HANDLE_SIZE - 2);
        QPixmap pixmap(image.width(), image.height());
        if (!image.isNull()) {
            painter.drawImage(MARGIN + 1, MARGIN + 1, image);
        }

        QPolygon triangle(3);
        QList<KoGradientStop> handlePositions = m_gradient->stops();
        int position;
        for (int i = 0; i < handlePositions.count(); i++) {
            position = qRound(handlePositions[i].first * (double)(width() - 2*MARGIN)) + MARGIN;
            triangle[0] = QPoint(position, height() - HANDLE_SIZE - MARGIN);
            triangle[1] = QPoint(position + (HANDLE_SIZE / 2 - 1), height() - MARGIN);
            triangle[2] = QPoint(position - (HANDLE_SIZE / 2 - 1), height() - MARGIN);

            if(i != m_selectedStop)
                painter.setPen(QPen(Qt::black, 2.0));
            else
                painter.setPen(QPen(palette().highlight(), 2.0));
            painter.setBrush(QBrush(handlePositions[i].second.toQColor()));
            painter.setRenderHint(QPainter::Antialiasing);
            painter.drawPolygon(triangle);
        }
    }
}

void KisStopGradientSliderWidget::mousePressEvent(QMouseEvent * e)
{
    if (e->x() < MARGIN || e->x() > width() - MARGIN) {
        QWidget::mousePressEvent(e);
        return;
    }

    double t = (double)(e->x() - MARGIN) / (double)(width() - 2 * MARGIN);
    if(e->y() < height() - HANDLE_SIZE - MARGIN) {
        if(e->button() == Qt::LeftButton)
            insertStop(t);
    }
    else {
        QPolygon triangle(3);
        QList<KoGradientStop> stops = m_gradient->stops();
        int position;
        for (int i = 0; i < stops.count(); i++) {
            position = qRound(stops[i].first * (double)(width() - 2*MARGIN)) + MARGIN;
            triangle[0] = QPoint(position, height() - HANDLE_SIZE - MARGIN);
            triangle[1] = QPoint(position + (HANDLE_SIZE / 2 - 1), height() - MARGIN);
            triangle[2] = QPoint(position - (HANDLE_SIZE / 2 - 1), height() - MARGIN);

            if(triangle.containsPoint(e->pos(), Qt::WindingFill))
            {
                if(e->button() == Qt::LeftButton) {
                    m_selectedStop = i;
                    emit sigSelectedStop(m_selectedStop);
                    if(m_selectedStop > 0 && m_selectedStop < stops.size()-1)
                        m_drag = true;
                }
                else if (e->button() == Qt::RightButton && (i > 0 && i < stops.size()-1)) {
                    QList<KoGradientStop> stops = m_gradient->stops();
                    stops.removeAt(i);
                    m_gradient->setStops(stops);
                    if(m_selectedStop == i)
                        m_selectedStop = i-1;
                    else if (m_selectedStop > i)
                        m_selectedStop--;
                }
                break;
            }
        }
    }
    update();
}

void KisStopGradientSliderWidget::mouseReleaseEvent(QMouseEvent * e)
{
     Q_UNUSED(e);
     m_drag = false;
}

void KisStopGradientSliderWidget::mouseMoveEvent(QMouseEvent * e)
{
    if ((e->y() < MARGIN || e->y() > height() - MARGIN) || (e->x() < MARGIN || e->x() > width() - MARGIN)) {
        QWidget::mouseMoveEvent(e);
        return;
    }

    double t = (double)(e->x() - MARGIN) / (double)(width() - 2 * MARGIN);
    if (m_drag) {
        QList<KoGradientStop> stops = m_gradient->stops();

        KoGradientStop dragedStop = stops[m_selectedStop];

        t = qBound(stops[m_selectedStop-1].first, t, stops[m_selectedStop+1].first);
        dragedStop.first = t;

        stops.removeAt(m_selectedStop);
        stops.insert(m_selectedStop, dragedStop);

        m_gradient->setStops(stops);
    }

    update();
}

void KisStopGradientSliderWidget::insertStop(double t)
{
    QList<KoGradientStop> stopPositions = m_gradient->stops();
    int i = 0;
    while(stopPositions[i].first < t)
        i++;

    KoColor color;
    m_gradient->colorAt(color, t);
    stopPositions.insert(i, KoGradientStop(t, color));
    m_gradient->setStops(stopPositions);

    m_selectedStop = i;
    emit sigSelectedStop(m_selectedStop);
}

int KisStopGradientSliderWidget::selectedStop()
{
    return m_selectedStop;
}

void KisStopGradientSliderWidget::setSeletectStop(int selected)
{
    m_selectedStop = selected;
    emit sigSelectedStop(m_selectedStop);
}



