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
#include "KoColor.h"
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

            emit paramChanged(radius, angle);
        }
    }

    emit update();

    return QColor::fromRgb(colorAt(x, y));
}

void KisColorSelectorSimple::paint(QPainter* painter)
{
    QImage tmpDev(width(), height(), QImage::Format_ARGB32_Premultiplied);

    for(int x=0; x<width(); x++) {
        for(int y=0; y<height(); y++) {
//            tmpDev.setPixel(x, y, QColor::fromHslF(0,x/qreal(width()), 1-y/qreal(height())).rgb());
            tmpDev.setPixel(x, y, colorAt(x, y));
        }
    }

    //antialiasing for wheel
    if(m_type==KisColorSelector::Wheel) {
        QPainter tmpPainter(&tmpDev);
        tmpPainter.setRenderHint(QPainter::Antialiasing);
        tmpPainter.setPen(QPen(QColor(0,0,0,0), 2.5));
        tmpPainter.setCompositionMode(QPainter::CompositionMode_Clear);
        int size=qMin(width(), height());
        tmpPainter.drawEllipse(width()/2-size/2, height()/2-size/2, size, size);
    }

    painter->drawImage(0,0, tmpDev);

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
            return KoColor(QColor::fromHslF(parameter1(), xRel, yRel).rgb(), colorSpace()).toQColor().rgb();
            break;
        case KisColorSelector::SV:
            return KoColor(QColor::fromHsvF(parameter1(), xRel, yRel).rgb(), colorSpace()).toQColor().rgb();
            break;
        case KisColorSelector::SH:
            return KoColor(QColor::fromHsvF(xRel, yRel, parameter1()).rgb(), colorSpace()).toQColor().rgb();
            break;
        case KisColorSelector::VH:
            return KoColor(QColor::fromHsvF(xRel, parameter1(), yRel).rgb(), colorSpace()).toQColor().rgb();
            break;
        case KisColorSelector::LH:
            return KoColor(QColor::fromHslF(xRel, parameter1(), yRel).rgb(), colorSpace()).toQColor().rgb();
            break;
        case KisColorSelector::H:
            return KoColor(QColor::fromHsvF(relPos, 1, 1).rgb(), colorSpace()).toQColor().rgb();
            break;
        case KisColorSelector::S:
            return KoColor(QColor::fromHsvF(parameter1(), relPos, parameter2()).rgb(), colorSpace()).toQColor().rgb();
            break;
        case KisColorSelector::V:
            return KoColor(QColor::fromHsvF(parameter1(), parameter2(), relPos).rgb(), colorSpace()).toQColor().rgb();
            break;
        case KisColorSelector::L:
            return KoColor(QColor::fromHslF(parameter1(), parameter2(), relPos).rgb(), colorSpace()).toQColor().rgb();
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
            return KoColor(QColor::fromHsvF(angle, radius, parameter1()).rgb(), colorSpace()).toQColor().rgb();
            break;
        case KisColorSelector::VH:
            return KoColor(QColor::fromHsvF(angle, parameter1(), radius).rgb(), colorSpace()).toQColor().rgb();
            break;
        case KisColorSelector::LH:
            return KoColor(QColor::fromHslF(angle, parameter1(), radius).rgb(), colorSpace()).toQColor().rgb();
            break;
        default:
            return qRgb(255,0,0);
        }
    }
}
