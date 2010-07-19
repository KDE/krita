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

#include <KDebug>

KisColorSelectorSimple::KisColorSelectorSimple(KisColorSelectorBase *parent) :
    KisColorSelectorComponent(parent),
    m_parameter(KisColorSelector::SL),
    m_lastClickPos(-1,-1)
{
}

void KisColorSelectorSimple::setConfiguration(Parameter param, Type type)
{
    m_parameter = param;
    m_type = type;
}

QColor KisColorSelectorSimple::selectColor(int x, int y)
{
    m_kocolor.convertTo(colorSpace());

    m_lastClickPos.setX(x);
    m_lastClickPos.setY(y);

    switch (m_parameter) {
    case KisColorSelector::H:
    case KisColorSelector::S:
    case KisColorSelector::V:
    case KisColorSelector::L:
        qreal relPos;
        if(height()>width())
            relPos = 1.-y/qreal(height());
        else
            relPos = 1.-x/qreal(width());

        emit paramChanged(relPos);

        break;
    case KisColorSelector::SL:
    case KisColorSelector::SV:
    case KisColorSelector::SH:
    case KisColorSelector::VH:
    case KisColorSelector::LH:
        if(m_type==KisColorSelector::Square) {
            qreal xRel = x/qreal(width());
            qreal yRel = 1.-y/qreal(height());

            emit paramChanged(xRel, yRel);
        }
        else {
            //wheel
            qreal xRel = x-width()/2.;
            qreal yRel = y-height()/2.;

            qreal radius = sqrt(xRel*xRel+yRel*yRel);
            if(radius>qMin(width(), height())/2)
                return qRgba(0,0,0,0);
            radius/=qMin(width(), height())/2.;

            qreal angle = std::atan2(yRel, xRel);
            angle+=M_PI;
            angle/=2*M_PI;

            emit paramChanged(angle, radius);
        }
    }

    emit update();

    return QColor::fromRgb(colorAt(x, y));
}

void KisColorSelectorSimple::paint(QPainter* painter)
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
        if(m_type==KisColorSelector::Wheel) {
            QPainter tmpPainter(&m_pixelCache);
            tmpPainter.setRenderHint(QPainter::Antialiasing);
            tmpPainter.setPen(QPen(QColor(0,0,0,0), 2.5));
            tmpPainter.setCompositionMode(QPainter::CompositionMode_Clear);
            int size=qMin(width(), height());
            tmpPainter.drawEllipse(width()/2-size/2, height()/2-size/2, size, size);
        }
    }

    painter->drawImage(0,0, m_pixelCache);

    if(m_lastClickPos!=QPoint(-1,-1)) {
        painter->setPen(QColor(0,0,0));
        painter->drawEllipse(m_lastClickPos, 5, 5);
        painter->setPen(QColor(255,255,255));
        painter->drawEllipse(m_lastClickPos, 4, 4);
    }
}

QRgb KisColorSelectorSimple::colorAt(int x, int y)
{
    Q_ASSERT(x>=0 && x<width());
    Q_ASSERT(y>=0 && y<height());
    if(m_type==KisColorSelector::Square || m_type==KisColorSelector::Slider) {
        qreal xRel = x/qreal(width());
        qreal yRel = 1.-y/qreal(height());
        qreal relPos;
        if(height()>width())
            relPos = 1.-y/qreal(height());
        else
            relPos = 1.-x/qreal(width());

        switch(m_parameter) {
        case KisColorSelector::SL:
            m_qcolor.setHslF(parameter1(), xRel, yRel);
            break;
        case KisColorSelector::SV:
            m_qcolor.setHsvF(parameter1(), xRel, yRel);
            break;
        case KisColorSelector::SH:
            m_qcolor.setHsvF(xRel, yRel, parameter1());
            break;
        case KisColorSelector::VH:
            m_qcolor.setHsvF(xRel, parameter1(), yRel);
            break;
        case KisColorSelector::LH:
            m_qcolor.setHslF(xRel, parameter1(), yRel);
            break;
        case KisColorSelector::H:
            m_qcolor.setHsvF(relPos, 1, 1);
            break;
        case KisColorSelector::S:
            m_qcolor.setHsvF(parameter1(), relPos, parameter2());
            break;
        case KisColorSelector::V:
            m_qcolor.setHsvF(parameter1(), parameter2(), relPos);
            break;
        case KisColorSelector::L:
            m_qcolor.setHslF(parameter1(), parameter2(), relPos);
            break;
        default:
            return qRgb(255,0,0);
        }
    }
    else {
        //wheel
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
        case KisColorSelector::SH:
            m_qcolor.setHsvF(angle, radius, parameter1());
            break;
        case KisColorSelector::VH:
            m_qcolor.setHsvF(angle, parameter1(), radius);
            break;
        case KisColorSelector::LH:
            m_qcolor.setHslF(angle, parameter1(), radius);
            break;
        default:
            return qRgb(255,0,0);
        }
    }

    m_kocolor.fromQColor(m_qcolor);
    m_kocolor.toQColor(&m_qcolor);
    return m_qcolor.rgb();
}
