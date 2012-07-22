/*
 *  Copyright (c) 2010 Adam Celarek <kdedev at xibo dot at>
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
 */

#include "kis_color_selector_wheel.h"

#include <QImage>
#include <QPainter>
#include <QColor>
#include <cmath>

KisColorSelectorWheel::KisColorSelectorWheel(KisColorSelector *parent) :
    KisColorSelectorComponent(parent),
    m_lastClickPos(-1,-1)
{
}

void KisColorSelectorWheel::setColor(const QColor &c)
{
    qreal angle = 0.0, radius = 0.0;
    angle = c.hueF();
    angle *= 2. * M_PI;
    angle -= M_PI;
    switch (m_parameter) {
    case KisColorSelector::LH:
        emit paramChanged(c.hueF(), -1, -1, -1, c.lightnessF());
        radius = c.lightnessF();
        break;
    case KisColorSelector::VH:
        emit paramChanged(c.hueF(), -1, c.valueF(), -1, -1);
        radius=c.valueF();
        break;
    case KisColorSelector::hsvSH:
        emit paramChanged(c.hueF(), c.saturationF(), -1, -1, -1);
        radius = c.saturationF();
        break;
    case KisColorSelector::hslSH:
        emit paramChanged(c.hueF(), -1, -1, c.hslSaturationF(), -1);
        radius = c.hslSaturationF();
        break;
    default:
        Q_ASSERT(false);
        break;
    }
    radius*=0.5;

    m_lastClickPos.setX(cos(angle)*radius+0.5);
    m_lastClickPos.setY(sin(angle)*radius+0.5);

    //workaround for bug 279500
    setLastMousePosition(m_lastClickPos.x()*width(), m_lastClickPos.y()*height());
}

QColor KisColorSelectorWheel::selectColor(int x, int y)
{
    m_kocolor.convertTo(colorSpace());

    int xWheel = x-width()/2;
    int yWheel = y-height()/2;

    qreal radius = sqrt((double)xWheel*xWheel+yWheel*yWheel);
    radius/=qMin(width(), height());
    if(radius>0.5)
        radius=0.5;

    radius*=2.;

    qreal angle = std::atan2((qreal)yWheel, (qreal)xWheel);
    angle+=M_PI;
    angle/=2*M_PI;

//    kDebug()<<"angele="<<angle << "   radius="<<radius;

    switch (m_parameter) {
    case KisColorSelector::hsvSH:
        emit paramChanged(angle, radius, -1, -1, -1);
        break;
    case KisColorSelector::hslSH:
        emit paramChanged(angle, -1, -1, radius, -1);
        break;
    case KisColorSelector::VH:
        emit paramChanged(angle, -1, radius, -1, -1);
        break;
    case KisColorSelector::LH:
        emit paramChanged(angle, -1, -1, -1, radius);
        break;
    default:
        Q_ASSERT(false);
        break;
    }

    emit update();

    angle *= 2. * M_PI;
    angle -= M_PI;
    radius*=0.5;
    m_lastClickPos.setX(cos(angle)*radius+0.5);
    m_lastClickPos.setY(sin(angle)*radius+0.5);

    return colorAt(x, y, true);
}

void KisColorSelectorWheel::paint(QPainter* painter)
{
    if(isDirty()) {
        m_kocolor.convertTo(colorSpace());

        m_pixelCache=QImage(width(), height(), QImage::Format_ARGB32_Premultiplied);

        for(int x=0; x<width(); x++) {
            for(int y=0; y<height(); y++) {
                m_qcolor=colorAt(x, y);
                if(m_qcolor.isValid()) {
                    m_kocolor.fromQColor(m_qcolor);
                    m_kocolor.toQColor(&m_qcolor);
                    m_pixelCache.setPixel(x, y, m_qcolor.rgb());
                }
                else {
                    m_pixelCache.setPixel(x, y, qRgba(0,0,0,0));
                }
            }
        }

        //antialiasing for wheel
        QPainter tmpPainter(&m_pixelCache);
        tmpPainter.setRenderHint(QPainter::Antialiasing);
        tmpPainter.setPen(QPen(QColor(0,0,0,0), 2.5));
        tmpPainter.setCompositionMode(QPainter::CompositionMode_Clear);
        int size=qMin(width(), height());
        tmpPainter.drawEllipse(width()/2-size/2, height()/2-size/2, size, size);
    }

    painter->drawImage(0,0, m_pixelCache);

    // draw blips
    if(m_lastClickPos!=QPoint(-1,-1) && m_parent->displayBlip()) {
        QPoint pos = (m_lastClickPos*qMin(width(), height())).toPoint();
        if(width()<height())
            pos.setY(pos.y()+height()/2-width()/2);
        else
            pos.setX(pos.x()+width()/2-height()/2);

        painter->setPen(QColor(0,0,0));
        painter->drawEllipse(pos, 5, 5);
        painter->setPen(QColor(255,255,255));
        painter->drawEllipse(pos, 4, 4);
    }
}

const QColor& KisColorSelectorWheel::colorAt(int x, int y, bool forceValid)
{
    Q_ASSERT(x>=0 && x<=width());
    Q_ASSERT(y>=0 && y<=height());

    qreal xRel = x-width()/2.;
    qreal yRel = y-height()/2.;

    qreal radius = sqrt(xRel*xRel+yRel*yRel);
    if(radius>qMin(width(), height())/2) {
        if (!forceValid) {
            m_qcolor = QColor();
            return m_qcolor;
        } else {
            radius = qMin(width(), height())/2;
        }
    }
    radius/=qMin(width(), height())/2.;

    qreal angle = std::atan2(yRel, xRel);
    angle+=M_PI;
    angle/=2*M_PI;


    switch(m_parameter) {
    case KisColorSelector::hsvSH:
        m_qcolor.setHsvF(angle, radius, m_value);
        break;
    case KisColorSelector::hslSH:
        m_qcolor.setHslF(angle, radius, m_lightness);
        break;
    case KisColorSelector::VH:
        m_qcolor.setHsvF(angle, m_hsvSaturation, radius);
        break;
    case KisColorSelector::LH:
        m_qcolor.setHslF(angle, m_hslSaturation, radius);
        break;
    default:
        Q_ASSERT(false);
        m_qcolor = QColor();
        return m_qcolor;
    }

    return m_qcolor;
}
