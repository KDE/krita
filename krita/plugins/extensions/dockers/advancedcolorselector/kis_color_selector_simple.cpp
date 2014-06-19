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
#include "kis_display_color_converter.h"
#include "kis_acs_pixel_cache_renderer.h"


KisColorSelectorSimple::KisColorSelectorSimple(KisColorSelector *parent) :
    KisColorSelectorComponent(parent),
    m_lastClickPos(-1,-1)
{
}

KoColor KisColorSelectorSimple::selectColor(int x, int y)
{
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
    return colorAt(x, y);
}

void KisColorSelectorSimple::setColor(const KoColor &color)
{
    qreal hsvH, hsvS, hsvV;
    qreal hslH, hslS, hslL;
    m_parent->converter()->getHsvF(color, &hsvH, &hsvS, &hsvV);
    m_parent->converter()->getHslF(color, &hslH, &hslS, &hslL);

    switch (m_parameter) {
    case KisColorSelector::SL:
        m_lastClickPos.setX(hslS);
        m_lastClickPos.setY(1 - hslL);
        emit paramChanged(-1, -1, -1, hslS, hslL);
        break;
    case KisColorSelector::LH:
        m_lastClickPos.setX(qBound<qreal>(0., hslH, 1.));
        m_lastClickPos.setY(1 - hslL);
        emit paramChanged(hslH, -1, -1, -1, hslL);
        break;
    case KisColorSelector::SV:
        m_lastClickPos.setX(hsvS);
        m_lastClickPos.setY(1 - hsvV);
        emit paramChanged(-1, hsvS, hsvV, -1, -1);
        break;
    case KisColorSelector::SV2: {
        qreal xRel = hsvS;
        qreal yRel = 0.5;

        if(xRel != 1.0)
            yRel = 1.0 - qBound<qreal>(0.0, (hsvV - xRel) / (1.0 - xRel), 1.0);

        m_lastClickPos.setX(xRel);
        m_lastClickPos.setY(yRel);
        emit paramChanged(-1, -1, -1, xRel, yRel);
        break;
    }
    case KisColorSelector::VH:
        m_lastClickPos.setX(qBound<qreal>(0., hsvH, 1.));
        m_lastClickPos.setY(1 - hsvV);
        emit paramChanged(hsvH, -1, hsvV, -1, -1);
        break;
    case KisColorSelector::hsvSH:
        m_lastClickPos.setX(qBound<qreal>(0., hsvH, 1.));
        m_lastClickPos.setY(1 - hsvS);
        emit paramChanged(hsvH, hsvS, -1, -1, -1);
        break;
    case KisColorSelector::hslSH:
        m_lastClickPos.setX(qBound<qreal>(0., hslH, 1.));
        m_lastClickPos.setY(1 - hslS);
        emit paramChanged(hslH, -1, -1, hslS, -1);
        break;
    case KisColorSelector::L:
        m_lastClickPos.setX(qBound<qreal>(0., hslL, 1.));
        emit paramChanged(-1, -1, -1, -1, hslL);
        break;
    case KisColorSelector::V:
        m_lastClickPos.setX(hsvV);
        emit paramChanged(-1, -1, hsvV, -1, -1);
        break;
    case KisColorSelector::hsvS:
        m_lastClickPos.setX( hsvS );
        emit paramChanged(-1, hsvS, -1, -1, -1);
        break;
    case KisColorSelector::hslS:
        m_lastClickPos.setX( hslS );
        emit paramChanged(-1, -1, -1, hslS, -1);
        break;
    case KisColorSelector::H:
        m_lastClickPos.setX(qBound<qreal>(0., hsvH, 1.));
        emit paramChanged(hsvH, -1, -1, -1, -1);
        break;
    default:
        Q_ASSERT(false);
        break;
    }
    emit update();
    //Workaround for bug 317648
    setLastMousePosition((m_lastClickPos.x()*width()), (m_lastClickPos.y()*height()));
}

void KisColorSelectorSimple::paint(QPainter* painter)
{
    if(isDirty()) {
        KisPaintDeviceSP realPixelCache;
        QPoint pixelCacheOffset;
        Acs::PixelCacheRenderer::render(this,
                                        m_parent->converter(),
                                        QRect(0, 0, width(), height()),
                                        realPixelCache,
                                        m_pixelCache,
                                        pixelCacheOffset);

        if (!pixelCacheOffset.isNull()) {
            qWarning() << "WARNING: offset of the rectangle selector is not null!";
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

KoColor KisColorSelectorSimple::colorAt(int x, int y)
{
    qreal xRel = x/qreal(width());
    qreal yRel = 1.-y/qreal(height());
    qreal relPos;
    if(height()>width())
        relPos = 1.-y/qreal(height());
    else
        relPos = x/qreal(width());

    KoColor color(Qt::transparent, m_parent->colorSpace());

    switch(m_parameter) {
    case KisColorSelector::SL:
        color = m_parent->converter()->fromHslF(m_hue, xRel, yRel);
        break;
    case KisColorSelector::SV:
        color = m_parent->converter()->fromHsvF(m_hue, xRel, yRel);
        break;
    case KisColorSelector::SV2:
        color = m_parent->converter()->fromHsvF(m_hue, xRel, xRel + (1.0-xRel)*yRel);
        break;
    case KisColorSelector::hsvSH:
        color = m_parent->converter()->fromHsvF(xRel, yRel, m_value);
        break;
    case KisColorSelector::hslSH:
        color = m_parent->converter()->fromHslF(xRel, yRel, m_lightness);
        break;
    case KisColorSelector::VH:
        color = m_parent->converter()->fromHsvF(xRel, m_hsvSaturation, yRel);
        break;
    case KisColorSelector::LH:
        color = m_parent->converter()->fromHslF(xRel, m_hslSaturation, yRel);
        break;
    case KisColorSelector::H:
        color = m_parent->converter()->fromHsvF(relPos, 1, 1);
        break;
    case KisColorSelector::hsvS:
        color = m_parent->converter()->fromHsvF(m_hue, relPos, m_value);
        break;
    case KisColorSelector::hslS:
        color = m_parent->converter()->fromHslF(m_hue, relPos, m_lightness);
        break;
    case KisColorSelector::V:
        color = m_parent->converter()->fromHsvF(m_hue, m_hsvSaturation, relPos);
        break;
    case KisColorSelector::L:
        color = m_parent->converter()->fromHslF(m_hue, m_hslSaturation, relPos);
        break;
    default:
        Q_ASSERT(false);

        return color;
    }

    return color;
}
