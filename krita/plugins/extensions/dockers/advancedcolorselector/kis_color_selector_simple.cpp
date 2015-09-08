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
#include <kglobal.h>
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
        emit paramChanged(relPos, -1, -1, -1, -1, -1, -1, -1, -1);
        break;
    case KisColorSelector::hsvS:
        emit paramChanged(-1, relPos, -1, -1, -1, -1, -1, -1, -1);
        break;
    case KisColorSelector::hslS:
        emit paramChanged(-1, -1, -1, relPos, -1, -1, -1, -1, -1);
        break;
	case KisColorSelector::hsiS:
        emit paramChanged(-1, -1, -1, -1, -1, relPos, -1, -1, -1);
        break;
	case KisColorSelector::hsyS:
        emit paramChanged(-1, -1, -1, -1, -1, -1, -1, relPos, -1);
        break;	
    case KisColorSelector::V:
        emit paramChanged(-1, -1, relPos, -1, -1, -1, -1, -1, -1);
        break;
    case KisColorSelector::L:
        emit paramChanged(-1, -1, -1, -1, relPos, -1, -1, -1, -1);
        break;
	case KisColorSelector::I:
        emit paramChanged(-1, -1, -1, -1, -1, -1, relPos, -1, -1);
        break;
	case KisColorSelector::Y:
        emit paramChanged(-1, -1, -1, -1, -1, -1, -1, -1, relPos);
        break;
    case KisColorSelector::SL:
        emit paramChanged(-1, -1, -1, xRel, yRel, -1, -1, -1, -1);
        break;
	case KisColorSelector::SI:
        emit paramChanged(-1, -1, -1, -1, -1, xRel, yRel, -1, -1);
        break;
	case KisColorSelector::SY:
        emit paramChanged(-1, -1, -1, -1, -1, -1, -1, xRel, yRel);
        break;
    case KisColorSelector::SV2:
    case KisColorSelector::SV:
        emit paramChanged(-1, xRel, yRel, -1, -1, -1, -1, -1, -1);
        break;
    case KisColorSelector::hsvSH:
        emit paramChanged(xRel, yRel, -1, -1, -1, -1, -1, -1, -1);
        break;
    case KisColorSelector::hslSH:
        emit paramChanged(xRel, -1, -1, yRel, -1, -1, -1, -1, -1);
        break;
	case KisColorSelector::hsiSH:
        emit paramChanged(xRel, -1, -1, -1, -1, yRel, -1, -1, -1);
        break;
	case KisColorSelector::hsySH:
        emit paramChanged(xRel, -1, -1, -1, -1, -1, -1, yRel, -1);
        break;
    case KisColorSelector::VH:
        emit paramChanged(xRel, -1, yRel, -1, -1, -1, -1, -1, -1);
        break;
    case KisColorSelector::LH:
        emit paramChanged(xRel, -1, -1, -1, yRel, -1, -1, -1, -1);
        break;
	case KisColorSelector::IH:
        emit paramChanged(xRel, -1, -1, -1, -1, -1, yRel, -1, -1);
        break;
	case KisColorSelector::YH:
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
    KConfigGroup cfg = KGlobal::config()->group("advancedColorSelector");
	R = cfg.readEntry("lumaR", 0.2126);
    G = cfg.readEntry("lumaG", 0.7152);
    B = cfg.readEntry("lumaB", 0.0722);
    m_parent->converter()->getHsvF(color, &hsvH, &hsvS, &hsvV);
    m_parent->converter()->getHslF(color, &hslH, &hslS, &hslL);
    //here we add our convertor options
    m_parent->converter()->getHsiF(color, &hsiH, &hsiS, &hsiI);
	m_parent->converter()->getHsyF(color, &hsyH, &hsyS, &hsyY, R, G, B);

	//workaround, for some reason the HSI and HSY algorithms are fine, but they don't seem to update the selectors properly.
	hsiH=hslH;
	hsyH=hslH;

    switch (m_parameter) {
    case KisColorSelector::SL:
        m_lastClickPos.setX(hslS);
        m_lastClickPos.setY(1 - hslL);
        emit paramChanged(-1, -1, -1, hslS, hslL, -1, -1, -1, -1);
        break;
	case KisColorSelector::SI:
        m_lastClickPos.setX(hsiS);
        m_lastClickPos.setY(1 - hsiI);
        emit paramChanged(-1, -1, -1, -1, -1, hsiS, hsiI, -1, -1);
        break;
	case KisColorSelector::SY:
        m_lastClickPos.setX(hsyS);
        m_lastClickPos.setY(1 - hsyY);
        emit paramChanged(-1, -1, -1, -1, -1, -1, -1, hsyS, hsyY);
        break;
    case KisColorSelector::LH:
        m_lastClickPos.setX(qBound<qreal>(0., hslH, 1.));
        m_lastClickPos.setY(1 - hslL);
        emit paramChanged(hslH, -1, -1, -1, hslL, -1, -1, -1, -1);
        break;
    case KisColorSelector::SV:
        m_lastClickPos.setX(hsvS);
        m_lastClickPos.setY(1 - hsvV);
        emit paramChanged(-1, hsvS, hsvV, -1, -1, -1, -1, -1, -1);
        break;
    case KisColorSelector::SV2: {
        qreal xRel = hsvS;
        qreal yRel = 0.5;

        if(xRel != 1.0)
            yRel = 1.0 - qBound<qreal>(0.0, (hsvV - xRel) / (1.0 - xRel), 1.0);

        m_lastClickPos.setX(xRel);
        m_lastClickPos.setY(yRel);
        emit paramChanged(-1, -1, -1, xRel, yRel, -1, -1, -1, -1);
        break;
    }
    case KisColorSelector::VH:
        m_lastClickPos.setX(qBound<qreal>(0., hsvH, 1.));
        m_lastClickPos.setY(1 - hsvV);
        emit paramChanged(hsvH, -1, hsvV, -1, -1, -1, -1, -1, -1);
        break;
	case KisColorSelector::IH:
        m_lastClickPos.setX(qBound<qreal>(0., hsiH, 1.));
        m_lastClickPos.setY(1 - hsiI);
        emit paramChanged(hsiH, -1, -1, -1, -1, -1, hsiI, -1, -1);
        break;
	case KisColorSelector::YH:
        m_lastClickPos.setX(qBound<qreal>(0., hsyH, 1.));
        m_lastClickPos.setY(1 - hsyY);
        emit paramChanged(hsyH, -1, -1, -1, -1, -1, -1, -1, hsyY);
        break;
    case KisColorSelector::hsvSH:
        m_lastClickPos.setX(qBound<qreal>(0., hsvH, 1.));
        m_lastClickPos.setY(1 - hsvS);
        emit paramChanged(hsvH, hsvS, -1, -1, -1, -1, -1, -1, -1);
        break;
    case KisColorSelector::hslSH:
        m_lastClickPos.setX(qBound<qreal>(0., hslH, 1.));
        m_lastClickPos.setY(1 - hslS);
        emit paramChanged(hslH, -1, -1, hslS, -1, -1, -1, -1, -1);
        break;
	
    case KisColorSelector::hsiSH:
        m_lastClickPos.setX(qBound<qreal>(0., hsiH, 1.));
        m_lastClickPos.setY(1 - hsiS);
        emit paramChanged(hsiH, -1, -1, hsiS, -1, -1, -1, -1, -1);
        break;
	
    case KisColorSelector::hsySH:
        m_lastClickPos.setX(qBound<qreal>(0., hsyH, 1.));
        m_lastClickPos.setY(1 - hsyS);
        emit paramChanged(hsyH, -1, -1, hsyS, -1, -1, -1, -1, -1);
        break;
    case KisColorSelector::L:
        m_lastClickPos.setX(qBound<qreal>(0., hslL, 1.));
        emit paramChanged(-1, -1, -1, -1, hslL, -1, -1, -1, -1);
        break;
	case KisColorSelector::I:
        m_lastClickPos.setX(qBound<qreal>(0., hsiI, 1.));
        emit paramChanged(-1, -1, -1, -1, -1, -1, hsiI, -1, -1);
        break;
    case KisColorSelector::V:
        m_lastClickPos.setX(hsvV);
        emit paramChanged(-1, -1, hsvV, -1, -1, -1, -1, -1, -1);
        break;
	case KisColorSelector::Y:
        m_lastClickPos.setX(qBound<qreal>(0., hsyY, 1.));
        emit paramChanged(-1, -1, -1, -1, -1, -1, -1, -1, hsyY);
        break;
    case KisColorSelector::hsvS:
        m_lastClickPos.setX( hsvS );
        emit paramChanged(-1, hsvS, -1, -1, -1, -1, -1, -1, -1);
        break;
    case KisColorSelector::hslS:
        m_lastClickPos.setX( hslS );
        emit paramChanged(-1, -1, -1, hslS, -1, -1, -1, -1, -1);
        break;
	case KisColorSelector::hsiS:
        m_lastClickPos.setX( hsiS );
        emit paramChanged(-1, -1, -1, -1, -1, hsiS, -1, -1, -1);
        break;
	case KisColorSelector::hsyS:
        m_lastClickPos.setX( hsyS );
        emit paramChanged(-1, -1, -1, -1, -1, -1, -1, hsyS, -1);
        break;
    case KisColorSelector::H:
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
        case KisColorSelector::H:
        case KisColorSelector::hsvS:
        case KisColorSelector::hslS:
		case KisColorSelector::hsiS:
		case KisColorSelector::hsyS:
        case KisColorSelector::V:
        case KisColorSelector::L:
		case KisColorSelector::I:
		case KisColorSelector::Y:
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
		case KisColorSelector::SI:
		case KisColorSelector::SY:
        case KisColorSelector::hslSH:
        case KisColorSelector::hsvSH:
		case KisColorSelector::hsiSH:
		case KisColorSelector::hsySH:
        case KisColorSelector::VH:
        case KisColorSelector::LH:
		case KisColorSelector::IH:
		case KisColorSelector::YH:
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
	case KisColorSelector::SI:
        color = m_parent->converter()->fromHsiF(m_hue, xRel, yRel);
        break;
	case KisColorSelector::SY:
        color = m_parent->converter()->fromHsyF(m_hue, xRel, yRel, R, G, B);
        break;
    case KisColorSelector::hsvSH:
        color = m_parent->converter()->fromHsvF(xRel, yRel, m_value);
        break;
    case KisColorSelector::hslSH:
        color = m_parent->converter()->fromHslF(xRel, yRel, m_lightness);
        break;
	case KisColorSelector::hsiSH:
        color = m_parent->converter()->fromHsiF(xRel, yRel, m_intensity);
        break;
	case KisColorSelector::hsySH:
        color = m_parent->converter()->fromHsyF(xRel, yRel, m_luma, R, G, B);
        break;
    case KisColorSelector::VH:
        color = m_parent->converter()->fromHsvF(xRel, m_hsvSaturation, yRel);
        break;
    case KisColorSelector::LH:
        color = m_parent->converter()->fromHslF(xRel, m_hslSaturation, yRel);
        break;
	case KisColorSelector::IH:
        color = m_parent->converter()->fromHsiF(xRel, m_hsiSaturation, yRel);
        break;
	case KisColorSelector::YH:
        color = m_parent->converter()->fromHsyF(xRel, m_hsySaturation, yRel, R, G, B);
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
	case KisColorSelector::hsiS:
        color = m_parent->converter()->fromHsiF(m_hue, relPos, m_intensity);
		break;
	case KisColorSelector::I:
        color = m_parent->converter()->fromHsiF(m_hue, m_hsiSaturation, relPos);
        break;
	case KisColorSelector::hsyS:
        color = m_parent->converter()->fromHsyF(m_hue, relPos, m_luma, R, G, B);
		break;
	case KisColorSelector::Y:
        color = m_parent->converter()->fromHsyF(m_hue, m_hsySaturation, relPos, R, G, B);
        break;
    default:
        Q_ASSERT(false);

        return color;
    }

    return color;
}
