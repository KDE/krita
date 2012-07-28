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

#include "kis_color_selector_simple.h"
#include <QImage>
#include <QPainter>
#include <QColor>
#include <cmath>

KisColorSelectorSimple::KisColorSelectorSimple(KisColorSelector *parent) :
    KisColorSelectorComponent(parent),
    m_lastClickPos(-1,-1)
{
}

void KisColorSelectorSimple::setColor(const QColor &c)
{
    switch (m_parameter) {
    case KisColorSelector::SL:
        m_lastClickPos.setX(c.hslSaturationF());
        m_lastClickPos.setY(1.-c.lightnessF());
        emit paramChanged(-1, -1, -1, c.hslSaturationF(), c.lightnessF());
        break;
    case KisColorSelector::LH:
        m_lastClickPos.setX(qBound<qreal>(0., c.hueF(), 1.));
        m_lastClickPos.setY(1.-c.lightnessF());
        emit paramChanged(c.hueF(), -1, -1, -1, c.lightnessF());
        break;
    case KisColorSelector::SV:
        m_lastClickPos.setX(c.saturationF());
        m_lastClickPos.setY(1-c.valueF());
        emit paramChanged(-1, c.saturationF(), c.valueF(), -1, -1);
        break;
    case KisColorSelector::SV2: {
        qreal xRel = c.hsvSaturationF();
        qreal yRel = 0.5;
        
        if(xRel != 1.0)
            yRel = 1.0 - qBound<qreal>(0.0, (c.valueF() - xRel) / (1.0 - xRel), 1.0);
        
        m_lastClickPos.setX(xRel);
        m_lastClickPos.setY(yRel);
        emit paramChanged(-1, -1, -1, xRel, yRel);
        break;
    }
    case KisColorSelector::VH:
        m_lastClickPos.setX(qBound<qreal>(0., c.hueF(), 1.));
        m_lastClickPos.setY(c.valueF());
        emit paramChanged(c.hueF(), -1, c.valueF(), -1, -1);
        break;
    case KisColorSelector::hsvSH:
        m_lastClickPos.setX(qBound<qreal>(0., c.hueF(), 1.));
        m_lastClickPos.setY(1-c.saturationF());
        emit paramChanged(c.hueF(), c.saturationF(), -1, -1, -1);
        break;
    case KisColorSelector::hslSH:
        m_lastClickPos.setX(qBound<qreal>(0., c.hueF(), 1.));
        m_lastClickPos.setY(1-c.hslSaturationF());
        emit paramChanged(c.hueF(), -1, -1, c.hslSaturationF(), -1);
        break;
    case KisColorSelector::L:
        m_lastClickPos.setX(c.lightnessF());
        emit paramChanged(-1, -1, -1, -1, c.lightnessF());
        break;
    case KisColorSelector::V:
        m_lastClickPos.setX(c.valueF());
        emit paramChanged(-1, -1, c.valueF(), -1, -1);
        break;
    case KisColorSelector::hsvS:
        m_lastClickPos.setX(c.saturationF());
        emit paramChanged(-1, c.saturationF(), -1, -1, -1);
        break;
    case KisColorSelector::hslS:
        m_lastClickPos.setX(c.hslSaturationF());
        emit paramChanged(-1, -1, -1, c.hslSaturationF(), -1);
        break;
    case KisColorSelector::H:
        m_lastClickPos.setX(qBound<qreal>(0., c.hueF(), 1.));
        emit paramChanged(c.hueF(), -1, -1, -1, -1);
        break;
    default:
        Q_ASSERT(false);
        break;
    }
    emit update();
}

QColor KisColorSelectorSimple::selectColor(int x, int y)
{
    m_kocolor.convertTo(colorSpace());

    m_lastClickPos.setX(x/qreal(width()));
    m_lastClickPos.setY(y/qreal(height()));

    qreal xRel = x/qreal(width());
    qreal yRel = 1.-y/qreal(height());
    qreal relPos;
    if(height()>width())
        relPos = 1.-y/qreal(height());
    else
        relPos = x/qreal(width());

    switch (m_parameter) {
    case KisColorSelector::H:
        emit paramChanged(relPos, -1, -1, -1, -1);
        break;
    case KisColorSelector::hsvS:
        emit paramChanged(-1, relPos, -1, -1, -1);
        break;
    case KisColorSelector::hslS:
        emit paramChanged(-1, -1, -1, relPos, -1);
        break;
    case KisColorSelector::V:
        emit paramChanged(-1, -1, relPos, -1, -1);
        break;
    case KisColorSelector::L:
        emit paramChanged(-1, -1, -1, -1, relPos);
        break;
    case KisColorSelector::SL:
        emit paramChanged(-1, -1, -1, xRel, yRel);
        break;
    case KisColorSelector::SV2:
    case KisColorSelector::SV:
        emit paramChanged(-1, xRel, yRel, -1, -1);
        break;
    case KisColorSelector::hsvSH:
        emit paramChanged(xRel, yRel, -1, -1, -1);
        break;
    case KisColorSelector::hslSH:
        emit paramChanged(xRel, -1, -1, yRel, -1);
        break;
    case KisColorSelector::VH:
        emit paramChanged(xRel, -1, yRel, -1, -1);
        break;
    case KisColorSelector::LH:
        emit paramChanged(xRel, -1, -1, -1, yRel);
        break;
    }

    emit update();

//    kDebug()<<"selectColor(x/y) y rel="<<yRel<<"  value="<<QColor::fromRgb(colorAt(x, y)).valueF();
    return colorAt(x, y);
}

void KisColorSelectorSimple::paint(QPainter* painter)
{
    if(isDirty()) {
        m_kocolor.convertTo(colorSpace());

        m_pixelCache=QImage(width(), height(), QImage::Format_ARGB32_Premultiplied);

        for(int x=0; x<width(); x++) {
            for(int y=0; y<height(); y++) {
                m_kocolor.fromQColor(colorAt(x, y));
                m_kocolor.toQColor(&m_qcolor);
                m_pixelCache.setPixel(x, y, m_qcolor.rgb());
            }
        }
    }

    painter->drawImage(0,0, m_pixelCache);

    // draw blip
    if(m_lastClickPos!=QPointF(-1,-1) && m_parent->displayBlip()) {
        switch (m_parameter) {
        case KisColorSelector::H:
        case KisColorSelector::hsvS:
        case KisColorSelector::hslS:
        case KisColorSelector::V:
        case KisColorSelector::L:
            if(width()>height()) {
                painter->setPen(QColor(0,0,0));
                painter->drawLine(m_lastClickPos.x()*width()-1, 0, m_lastClickPos.x()*width()-1, height());
                painter->setPen(QColor(255,255,255));
                painter->drawLine(m_lastClickPos.x()*width()+1, 0, m_lastClickPos.x()*width()+1, height());
            }
            else {
                painter->setPen(QColor(0,0,0));
                painter->drawLine(0, m_lastClickPos.x()*height()-1, width(), m_lastClickPos.x()*height()-1);
                painter->setPen(QColor(255,255,255));
                painter->drawLine(0, m_lastClickPos.x()*height()+1, width(), m_lastClickPos.x()*height()+1);
            }
            break;
        case KisColorSelector::SL:
        case KisColorSelector::SV:
        case KisColorSelector::SV2:
        case KisColorSelector::hslSH:
        case KisColorSelector::hsvSH:
        case KisColorSelector::VH:
        case KisColorSelector::LH:
            painter->setPen(QColor(0,0,0));
            painter->drawEllipse(m_lastClickPos.x()*width()-5, m_lastClickPos.y()*height()-5, 10, 10);
            painter->setPen(QColor(255,255,255));
            painter->drawEllipse(m_lastClickPos.x()*width()-4, m_lastClickPos.y()*height()-4, 8, 8);
            break;
        }

    }
}

const QColor& KisColorSelectorSimple::colorAt(int x, int y)
{
    if (x < 0) x = 0;
    if (x > width()) x = width();
    if (y < 0) y = 0;
    if (y > width()) y = height();

    qreal xRel = x/qreal(width());
    qreal yRel = 1.-y/qreal(height());
    qreal relPos;
    if(height()>width())
        relPos = 1.-y/qreal(height());
    else
        relPos = x/qreal(width());

    QColor color;
    color.setHsvF(m_hue, 1.0, 1.0);
    
    switch(m_parameter) {
    case KisColorSelector::SL:
        m_qcolor.setHslF(m_hue, xRel, yRel);
        break;
    case KisColorSelector::SV:
        m_qcolor.setHsvF(m_hue, xRel, yRel);
        break;
    case KisColorSelector::SV2:
        m_qcolor.setHsvF(m_hue, xRel, xRel + (1.0-xRel)*yRel);
        break;
    case KisColorSelector::hsvSH:
        m_qcolor.setHsvF(xRel, yRel, m_value);
        break;
    case KisColorSelector::hslSH:
        m_qcolor.setHslF(xRel, yRel, m_lightness);
        break;
    case KisColorSelector::VH:
        m_qcolor.setHsvF(xRel, m_hsvSaturation, yRel);
        break;
    case KisColorSelector::LH:
        m_qcolor.setHslF(xRel, m_hslSaturation, yRel);
        break;
    case KisColorSelector::H:
        m_qcolor.setHsvF(relPos, 1, 1);
        break;
    case KisColorSelector::hsvS:
        m_qcolor.setHsvF(m_hue, relPos, m_value);
        break;
    case KisColorSelector::hslS:
        m_qcolor.setHslF(m_hue, relPos, m_lightness);
        break;
    case KisColorSelector::V:
        m_qcolor.setHsvF(m_hue, m_hsvSaturation, relPos);
        break;
    case KisColorSelector::L:
        m_qcolor.setHslF(m_hue, m_hslSaturation, relPos);
        break;
    default:
        Q_ASSERT(false);
        m_qcolor = QColor();
        return m_qcolor;
    }

    return m_qcolor;
}
