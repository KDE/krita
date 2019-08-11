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
#include <kconfig.h>
#include <kconfiggroup.h>
#include <ksharedconfig.h>

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
    case KisColorSelectorConfiguration::H:
        emit paramChanged(relPos, -1, -1, -1, -1, -1, -1, -1, -1);
        break;
    case KisColorSelectorConfiguration::hsvS:
        emit paramChanged(-1, relPos, -1, -1, -1, -1, -1, -1, -1);
        break;
    case KisColorSelectorConfiguration::hslS:
        emit paramChanged(-1, -1, -1, relPos, -1, -1, -1, -1, -1);
        break;
    case KisColorSelectorConfiguration::hsiS:
        emit paramChanged(-1, -1, -1, -1, -1, relPos, -1, -1, -1);
        break;
    case KisColorSelectorConfiguration::hsyS:
        emit paramChanged(-1, -1, -1, -1, -1, -1, -1, relPos, -1);
        break;
    case KisColorSelectorConfiguration::V:
        emit paramChanged(-1, -1, relPos, -1, -1, -1, -1, -1, -1);
        break;
    case KisColorSelectorConfiguration::L:
        emit paramChanged(-1, -1, -1, -1, relPos, -1, -1, -1, -1);
        break;
    case KisColorSelectorConfiguration::I:
        emit paramChanged(-1, -1, -1, -1, -1, -1, relPos, -1, -1);
        break;
    case KisColorSelectorConfiguration::Y:
        emit paramChanged(-1, -1, -1, -1, -1, -1, -1, -1, relPos);
        break;
    case KisColorSelectorConfiguration::SL:
        emit paramChanged(-1, -1, -1, xRel, yRel, -1, -1, -1, -1);
        break;
    case KisColorSelectorConfiguration::SI:
        emit paramChanged(-1, -1, -1, -1, -1, xRel, yRel, -1, -1);
        break;
    case KisColorSelectorConfiguration::SY:
        emit paramChanged(-1, -1, -1, -1, -1, -1, -1, xRel, yRel);
        break;
    case KisColorSelectorConfiguration::SV2:
    case KisColorSelectorConfiguration::SV:
        emit paramChanged(-1, xRel, yRel, -1, -1, -1, -1, -1, -1);
        break;
    case KisColorSelectorConfiguration::hsvSH:
        emit paramChanged(xRel, yRel, -1, -1, -1, -1, -1, -1, -1);
        break;
    case KisColorSelectorConfiguration::hslSH:
        emit paramChanged(xRel, -1, -1, yRel, -1, -1, -1, -1, -1);
        break;
    case KisColorSelectorConfiguration::hsiSH:
        emit paramChanged(xRel, -1, -1, -1, -1, yRel, -1, -1, -1);
        break;
    case KisColorSelectorConfiguration::hsySH:
        emit paramChanged(xRel, -1, -1, -1, -1, -1, -1, yRel, -1);
        break;
    case KisColorSelectorConfiguration::VH:
        emit paramChanged(xRel, -1, yRel, -1, -1, -1, -1, -1, -1);
        break;
    case KisColorSelectorConfiguration::LH:
        emit paramChanged(xRel, -1, -1, -1, yRel, -1, -1, -1, -1);
        break;
    case KisColorSelectorConfiguration::IH:
        emit paramChanged(xRel, -1, -1, -1, -1, -1, yRel, -1, -1);
        break;
    case KisColorSelectorConfiguration::YH:
        emit paramChanged(xRel, -1, -1, -1, -1, -1, -1, -1, yRel);
        break;
    }

    emit update();
    return colorAt(x, y);
}

void KisColorSelectorSimple::setColor(const KoColor &color)
{
    qreal hsvH, hsvS, hsvV;
    qreal hslH, hslS, hslL;
    qreal hsiH, hsiS, hsiI;
    qreal hsyH, hsyS, hsyY;
    KConfigGroup cfg =  KSharedConfig::openConfig()->group("advancedColorSelector");
    R = cfg.readEntry("lumaR", 0.2126);
    G = cfg.readEntry("lumaG", 0.7152);
    B = cfg.readEntry("lumaB", 0.0722);
    Gamma = cfg.readEntry("gamma", 2.2);
    m_parent->converter()->getHsvF(color, &hsvH, &hsvS, &hsvV);
    m_parent->converter()->getHslF(color, &hslH, &hslS, &hslL);
    //here we add our converter options
    m_parent->converter()->getHsiF(color, &hsiH, &hsiS, &hsiI);
    m_parent->converter()->getHsyF(color, &hsyH, &hsyS, &hsyY, R, G, B, Gamma);

    //workaround, for some reason the HSI and HSY algorithms are fine, but they don't seem to update the selectors properly.
    hsiH=hslH;
    hsyH=hslH;

    switch (m_parameter) {
    case KisColorSelectorConfiguration::SL:
        m_lastClickPos.setX(hslS);
        m_lastClickPos.setY(1 - hslL);
        emit paramChanged(-1, -1, -1, hslS, hslL, -1, -1, -1, -1);
        break;
    case KisColorSelectorConfiguration::SI:
        m_lastClickPos.setX(hsiS);
        m_lastClickPos.setY(1 - hsiI);
        emit paramChanged(-1, -1, -1, -1, -1, hsiS, hsiI, -1, -1);
        break;
    case KisColorSelectorConfiguration::SY:
        m_lastClickPos.setX(hsyS);
        m_lastClickPos.setY(1 - hsyY);
        emit paramChanged(-1, -1, -1, -1, -1, -1, -1, hsyS, hsyY);
        break;
    case KisColorSelectorConfiguration::LH:
        m_lastClickPos.setX(qBound<qreal>(0., hslH, 1.));
        m_lastClickPos.setY(1 - hslL);
        emit paramChanged(hslH, -1, -1, -1, hslL, -1, -1, -1, -1);
        break;
    case KisColorSelectorConfiguration::SV:
        m_lastClickPos.setX(hsvS);
        m_lastClickPos.setY(1 - hsvV);
        emit paramChanged(-1, hsvS, hsvV, -1, -1, -1, -1, -1, -1);
        break;
    case KisColorSelectorConfiguration::SV2: {
        qreal xRel = hsvS;
        qreal yRel = 0.5;

        if(xRel != 1.0)
            yRel = 1.0 - qBound<qreal>(0.0, (hsvV - xRel) / (1.0 - xRel), 1.0);

        m_lastClickPos.setX(xRel);
        m_lastClickPos.setY(yRel);
        emit paramChanged(-1, -1, -1, xRel, yRel, -1, -1, -1, -1);
        break;
    }
    case KisColorSelectorConfiguration::VH:
        m_lastClickPos.setX(qBound<qreal>(0., hsvH, 1.));
        m_lastClickPos.setY(1 - hsvV);
        emit paramChanged(hsvH, -1, hsvV, -1, -1, -1, -1, -1, -1);
        break;
    case KisColorSelectorConfiguration::IH:
        m_lastClickPos.setX(qBound<qreal>(0., hsiH, 1.));
        m_lastClickPos.setY(1 - hsiI);
        emit paramChanged(hsiH, -1, -1, -1, -1, -1, hsiI, -1, -1);
        break;
    case KisColorSelectorConfiguration::YH:
        m_lastClickPos.setX(qBound<qreal>(0., hsyH, 1.));
        m_lastClickPos.setY(1 - hsyY);
        emit paramChanged(hsyH, -1, -1, -1, -1, -1, -1, -1, hsyY);
        break;
    case KisColorSelectorConfiguration::hsvSH:
        m_lastClickPos.setX(qBound<qreal>(0., hsvH, 1.));
        m_lastClickPos.setY(1 - hsvS);
        emit paramChanged(hsvH, hsvS, -1, -1, -1, -1, -1, -1, -1);
        break;
    case KisColorSelectorConfiguration::hslSH:
        m_lastClickPos.setX(qBound<qreal>(0., hslH, 1.));
        m_lastClickPos.setY(1 - hslS);
        emit paramChanged(hslH, -1, -1, hslS, -1, -1, -1, -1, -1);
        break;

    case KisColorSelectorConfiguration::hsiSH:
        m_lastClickPos.setX(qBound<qreal>(0., hsiH, 1.));
        m_lastClickPos.setY(1 - hsiS);
        emit paramChanged(hsiH, -1, -1, hsiS, -1, -1, -1, -1, -1);
        break;

    case KisColorSelectorConfiguration::hsySH:
        m_lastClickPos.setX(qBound<qreal>(0., hsyH, 1.));
        m_lastClickPos.setY(1 - hsyS);
        emit paramChanged(hsyH, -1, -1, hsyS, -1, -1, -1, -1, -1);
        break;
    case KisColorSelectorConfiguration::L:
        m_lastClickPos.setX(qBound<qreal>(0., hslL, 1.));
        emit paramChanged(-1, -1, -1, -1, hslL, -1, -1, -1, -1);
        break;
    case KisColorSelectorConfiguration::I:
        m_lastClickPos.setX(qBound<qreal>(0., hsiI, 1.));
        emit paramChanged(-1, -1, -1, -1, -1, -1, hsiI, -1, -1);
        break;
    case KisColorSelectorConfiguration::V:
        m_lastClickPos.setX(hsvV);
        emit paramChanged(-1, -1, hsvV, -1, -1, -1, -1, -1, -1);
        break;
    case KisColorSelectorConfiguration::Y:
        m_lastClickPos.setX(qBound<qreal>(0., hsyY, 1.));
        emit paramChanged(-1, -1, -1, -1, -1, -1, -1, -1, hsyY);
        break;
    case KisColorSelectorConfiguration::hsvS:
        m_lastClickPos.setX( hsvS );
        emit paramChanged(-1, hsvS, -1, -1, -1, -1, -1, -1, -1);
        break;
    case KisColorSelectorConfiguration::hslS:
        m_lastClickPos.setX( hslS );
        emit paramChanged(-1, -1, -1, hslS, -1, -1, -1, -1, -1);
        break;
    case KisColorSelectorConfiguration::hsiS:
        m_lastClickPos.setX( hsiS );
        emit paramChanged(-1, -1, -1, -1, -1, hsiS, -1, -1, -1);
        break;
    case KisColorSelectorConfiguration::hsyS:
        m_lastClickPos.setX( hsyS );
        emit paramChanged(-1, -1, -1, -1, -1, -1, -1, hsyS, -1);
        break;
    case KisColorSelectorConfiguration::H:
        m_lastClickPos.setX(qBound<qreal>(0., hsvH, 1.));
        emit paramChanged(hsvH, -1, -1, -1, -1, -1, -1, -1, -1);
        break;
    default:
        Q_ASSERT(false);
        break;
    }
    emit update();
    //Workaround for bug 317648
    setLastMousePosition((m_lastClickPos.x()*width()), (m_lastClickPos.y()*height()));
    KisColorSelectorComponent::setColor(color);
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

//        if (!pixelCacheOffset.isNull()) {
//            warnKrita << "WARNING: offset of the rectangle selector is not null!";
//        }
    }

    painter->drawImage(0,0, m_pixelCache);

    // draw blip
    if(m_lastClickPos!=QPointF(-1,-1) && m_parent->displayBlip()) {
        switch (m_parameter) {
        case KisColorSelectorConfiguration::H:
        case KisColorSelectorConfiguration::hsvS:
        case KisColorSelectorConfiguration::hslS:
        case KisColorSelectorConfiguration::hsiS:
        case KisColorSelectorConfiguration::hsyS:
        case KisColorSelectorConfiguration::V:
        case KisColorSelectorConfiguration::L:
        case KisColorSelectorConfiguration::I:
        case KisColorSelectorConfiguration::Y:
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
        case KisColorSelectorConfiguration::SL:
        case KisColorSelectorConfiguration::SV:
        case KisColorSelectorConfiguration::SV2:
        case KisColorSelectorConfiguration::SI:
        case KisColorSelectorConfiguration::SY:
        case KisColorSelectorConfiguration::hslSH:
        case KisColorSelectorConfiguration::hsvSH:
        case KisColorSelectorConfiguration::hsiSH:
        case KisColorSelectorConfiguration::hsySH:
        case KisColorSelectorConfiguration::VH:
        case KisColorSelectorConfiguration::LH:
        case KisColorSelectorConfiguration::IH:
        case KisColorSelectorConfiguration::YH:
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
    case KisColorSelectorConfiguration::SL:
        color = m_parent->converter()->fromHslF(m_hue, xRel, yRel);
        break;
    case KisColorSelectorConfiguration::SV:
        color = m_parent->converter()->fromHsvF(m_hue, xRel, yRel);
        break;
    case KisColorSelectorConfiguration::SV2:
        color = m_parent->converter()->fromHsvF(m_hue, xRel, xRel + (1.0-xRel)*yRel);
        break;
    case KisColorSelectorConfiguration::SI:
        color = m_parent->converter()->fromHsiF(m_hue, xRel, yRel);
        break;
    case KisColorSelectorConfiguration::SY:
        color = m_parent->converter()->fromHsyF(m_hue, xRel, yRel, R, G, B, Gamma);
        break;
    case KisColorSelectorConfiguration::hsvSH:
        color = m_parent->converter()->fromHsvF(xRel, yRel, m_value);
        break;
    case KisColorSelectorConfiguration::hslSH:
        color = m_parent->converter()->fromHslF(xRel, yRel, m_lightness);
        break;
    case KisColorSelectorConfiguration::hsiSH:
        color = m_parent->converter()->fromHsiF(xRel, yRel, m_intensity);
        break;
    case KisColorSelectorConfiguration::hsySH:
        color = m_parent->converter()->fromHsyF(xRel, yRel, m_luma, R, G, B, Gamma);
        break;
    case KisColorSelectorConfiguration::VH:
        color = m_parent->converter()->fromHsvF(xRel, m_hsvSaturation, yRel);
        break;
    case KisColorSelectorConfiguration::LH:
        color = m_parent->converter()->fromHslF(xRel, m_hslSaturation, yRel);
        break;
    case KisColorSelectorConfiguration::IH:
        color = m_parent->converter()->fromHsiF(xRel, m_hsiSaturation, yRel);
        break;
    case KisColorSelectorConfiguration::YH:
        color = m_parent->converter()->fromHsyF(xRel, m_hsySaturation, yRel, R, G, B, Gamma);
        break;
    case KisColorSelectorConfiguration::H:
        color = m_parent->converter()->fromHsvF(relPos, 1, 1);
        break;
    case KisColorSelectorConfiguration::hsvS:
        color = m_parent->converter()->fromHsvF(m_hue, relPos, m_value);
        break;
    case KisColorSelectorConfiguration::hslS:
        color = m_parent->converter()->fromHslF(m_hue, relPos, m_lightness);
        break;
    case KisColorSelectorConfiguration::V:
        color = m_parent->converter()->fromHsvF(m_hue, m_hsvSaturation, relPos);
        break;
    case KisColorSelectorConfiguration::L:
        color = m_parent->converter()->fromHslF(m_hue, m_hslSaturation, relPos);
        break;
    case KisColorSelectorConfiguration::hsiS:
        color = m_parent->converter()->fromHsiF(m_hue, relPos, m_intensity);
        break;
    case KisColorSelectorConfiguration::I:
        color = m_parent->converter()->fromHsiF(m_hue, m_hsiSaturation, relPos);
        break;
    case KisColorSelectorConfiguration::hsyS:
        color = m_parent->converter()->fromHsyF(m_hue, relPos, m_luma, R, G, B, Gamma);
        break;
    case KisColorSelectorConfiguration::Y:
        color = m_parent->converter()->fromHsyF(m_hue, m_hsySaturation, relPos, R, G, B, Gamma);
        break;
    default:
        Q_ASSERT(false);

        return color;
    }

    return color;
}
