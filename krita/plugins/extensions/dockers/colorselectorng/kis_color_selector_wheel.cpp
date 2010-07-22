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

#include <KDebug>

KisColorSelectorWheel::KisColorSelectorWheel(KisColorSelectorBase *parent) :
    KisColorSelectorComponent(parent),
    m_lastClickPos(-1,-1)
{
}

void KisColorSelectorWheel::setColor(const QColor &c)
{
    switch (m_parameter) {
    case KisColorSelector::LH:
//        m_hue=c.hueF();
//        m_lightness=c.lightnessF();
        emit paramChanged(c.hueF(), -1, -1, -1, c.lightnessF());
        break;
    case KisColorSelector::VH:
//        m_hue=c.hueF();
//        m_value=c.valueF();
        emit paramChanged(c.hueF(), -1, c.valueF(), -1, -1);
        break;
    case KisColorSelector::hsvSH:
//        m_hue=c.hueF();
//        m_saturation=c.saturationF();
        emit paramChanged(c.hueF(), c.saturationF(), -1, -1, -1);
        break;
    case KisColorSelector::hslSH:
//        m_hue=c.hueF();
//        m_saturation=c.saturationF();
        emit paramChanged(c.hueF(), -1, -1, c.saturationF(), -1);
        break;
    default:
        Q_ASSERT(false);
        break;
    }
}

QColor KisColorSelectorWheel::selectColor(qreal x, qreal y)
{
    m_kocolor.convertTo(colorSpace());
    qreal xRel = x-0.5;
    qreal yRel = y-0.5;

    qreal radius = sqrt(xRel*xRel+yRel*yRel);
    if(radius>0.5)
        return QColor();

    m_lastClickPos.setX(x);
    m_lastClickPos.setY(y);

    radius*=2.;

    qreal angle = std::atan2(yRel, xRel);
    angle+=M_PI;
    angle/=2*M_PI;

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

    return QColor::fromRgb(colorAt(x*width(), y*height()));
}

void KisColorSelectorWheel::paint(QPainter* painter)
{
    if(isDirty()) {
        m_kocolor.convertTo(colorSpace());

        m_pixelCache=QImage(width(), height(), QImage::Format_ARGB32_Premultiplied);

        for(int x=0; x<width(); x++) {
            for(int y=0; y<height(); y++) {
                m_pixelCache.setPixel(x, y, colorAt(x, y));
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

    if(m_lastClickPos!=QPoint(-1,-1)) {
        painter->setPen(QColor(0,0,0));
        painter->drawEllipse(m_lastClickPos.x()*width()-5, m_lastClickPos.y()*height()-5, 10, 10);
        painter->setPen(QColor(255,255,255));
        painter->drawEllipse(m_lastClickPos.x()*width()-4, m_lastClickPos.y()*height()-4, 8, 8);
    }
}

QRgb KisColorSelectorWheel::colorAt(int x, int y)
{
    Q_ASSERT(x>=0 && x<=width());
    Q_ASSERT(y>=0 && y<=height());

    qreal xRel = x-width()/2.;
    qreal yRel = y-height()/2.;

    qreal radius = sqrt(xRel*xRel+yRel*yRel);
    if(radius>qMin(width(), height())/2)
        return qRgba(0,0,0,0);
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
        return qRgb(255,0,0);
    }

    m_kocolor.fromQColor(m_qcolor);
    m_kocolor.toQColor(&m_qcolor);
    return m_qcolor.rgb();
}
