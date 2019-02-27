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
#include <kconfig.h>
#include <kconfiggroup.h>
#include <ksharedconfig.h>

#include <KisGamutMaskViewConverter.h>

#include "kis_display_color_converter.h"
#include "kis_acs_pixel_cache_renderer.h"


KisColorSelectorWheel::KisColorSelectorWheel(KisColorSelector *parent) :
    KisColorSelectorComponent(parent),
    m_lastClickPos(-1,-1),
    m_renderAreaSize(1,1),
    m_renderAreaOffsetX(0.0),
    m_renderAreaOffsetY(0.0),
    m_toRenderArea(QTransform())
{
    m_viewConverter = new KisGamutMaskViewConverter();
}

KoColor KisColorSelectorWheel::selectColor(int x, int y)
{
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

    switch (m_parameter) {
    case KisColorSelectorConfiguration::hsvSH:
        emit paramChanged(angle, radius, -1, -1, -1, -1, -1, -1, -1);
        break;
    case KisColorSelectorConfiguration::hslSH:
        emit paramChanged(angle, -1, -1, radius, -1, -1, -1, -1, -1);
        break;
    case KisColorSelectorConfiguration::hsiSH:
        emit paramChanged(angle, -1, -1, -1, -1, radius, -1, -1, -1);
        break;
    case KisColorSelectorConfiguration::hsySH:
        emit paramChanged(angle, -1, -1, -1, -1, -1, -1, radius, -1);
        break;
    case KisColorSelectorConfiguration::VH:
        emit paramChanged(angle, -1, radius, -1, -1, -1, -1, -1, -1);
        break;
    case KisColorSelectorConfiguration::LH:
        emit paramChanged(angle, -1, -1, -1, radius, -1, -1, -1, -1);
        break;
    case KisColorSelectorConfiguration::IH:
        emit paramChanged(angle, -1, -1, -1, -1, -1, radius, -1, -1);
        break;
    case KisColorSelectorConfiguration::YH:
        emit paramChanged(angle, -1, -1, -1, -1, -1, -1, -1, radius);
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

void KisColorSelectorWheel::setColor(const KoColor &color)
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
    m_parent->converter()->getHsiF(color, &hsiH, &hsiS, &hsiI);
    m_parent->converter()->getHsyF(color, &hsyH, &hsyS, &hsyY, R, G, B, Gamma);

    //workaround, for some reason the HSI and HSY algorithms are fine, but they don't seem to update the selectors properly.
    hsiH=hslH;
    hsyH=hslH;

    qreal angle = 0.0, radius = 0.0;
    angle = hsvH;
    angle *= 2. * M_PI;
    angle -= M_PI;
    switch (m_parameter) {
    case KisColorSelectorConfiguration::LH:
        emit paramChanged(hslH, -1, -1, -1, hslL, -1, -1, -1, -1);
        radius = hslL;
        break;
    case KisColorSelectorConfiguration::VH:
        emit paramChanged(hsvH, -1, hsvV, -1, -1, -1, -1, -1, -1);
        radius = hsvV;
        break;
    case KisColorSelectorConfiguration::IH:
        emit paramChanged(hslH, -1, -1, -1, -1, -1, hsiI, -1, -1);
        radius = hsiI;
        break;
    case KisColorSelectorConfiguration::YH:
        emit paramChanged(hsvH, -1, -1, -1, -1, -1, -1, -1, hsyY);
        radius = hsyY;
        break;
    case KisColorSelectorConfiguration::hsvSH:
        emit paramChanged(hsvH, hsvS, -1, -1, -1, -1, -1, -1, -1);
        radius = hsvS;
        break;
    case KisColorSelectorConfiguration::hslSH:
        emit paramChanged(hslH, -1, -1, hslS, -1, -1, -1, -1, -1);
        radius = hslS;
        break;
    case KisColorSelectorConfiguration::hsiSH:
        emit paramChanged(hsiH, -1, -1, -1, -1, hsiS, -1, -1, -1);
        radius = hsiS;
        break;
    case KisColorSelectorConfiguration::hsySH:
        emit paramChanged(hsyH, -1, -1, -1, -1, -1, -1, hsyS, -1);
        radius = hsyS;
        break;
    default:
        Q_ASSERT(false);
        break;
    }
    radius *= 0.5;

    m_lastClickPos.setX(cos(angle)*radius+0.5);
    m_lastClickPos.setY(sin(angle)*radius+0.5);

    //workaround for bug 279500

    if(m_lastClickPos!=QPoint(-1,-1) && m_parent->displayBlip()) {
        QPoint pos = (m_lastClickPos*qMin(width(), height())).toPoint();
        if(width() < height()) {
            pos.setY(pos.y()+height()/2-width()/2);
        } else {
            pos.setX(pos.x()+width()/2-height()/2);
        }

        setLastMousePosition(pos.x(), pos.y());
    }

    KisColorSelectorComponent::setColor(color);
}

void KisColorSelectorWheel::paint(QPainter* painter)
{

    if(isDirty()) {
        KisPaintDeviceSP realPixelCache;
        Acs::PixelCacheRenderer::render(this, m_parent->converter(), QRect(0, 0, width(), height()), realPixelCache, m_pixelCache, m_pixelCacheOffset);

        //antialiasing for wheel
        QPainter tmpPainter(&m_pixelCache);
        tmpPainter.setRenderHint(QPainter::Antialiasing);
        tmpPainter.setPen(QPen(QColor(0,0,0,0), 2.5));
        tmpPainter.setCompositionMode(QPainter::CompositionMode_Clear);
        int size=qMin(width(), height());

        m_renderAreaSize = QSize(size,size);
        m_renderAreaOffsetX = ((qreal)width()-(qreal)m_renderAreaSize.width())*0.5;
        m_renderAreaOffsetY = ((qreal)height()-(qreal)m_renderAreaSize.height())*0.5;
        m_toRenderArea.reset();
        m_toRenderArea.translate(-m_renderAreaOffsetX,-m_renderAreaOffsetY);

        m_viewConverter->setViewSize(m_renderAreaSize);
        if (m_currentGamutMask) {
           m_viewConverter->setMaskSize(m_currentGamutMask->maskSize());
        }

        QPoint ellipseCenter(width() / 2 - size / 2, height() / 2 - size / 2);
        ellipseCenter -= m_pixelCacheOffset;

        tmpPainter.drawEllipse(ellipseCenter.x(), ellipseCenter.y(), size, size);
    }

    painter->drawImage(m_pixelCacheOffset.x(),m_pixelCacheOffset.y(), m_pixelCache);

    // draw gamut mask
    if (m_gamutMaskOn && m_currentGamutMask) {
        QImage maskBuffer  = QImage(m_renderAreaSize.width(), m_renderAreaSize.height(), QImage::Format_ARGB32_Premultiplied);
        maskBuffer.fill(0);
        QPainter maskPainter(&maskBuffer);

        QRect rect = QRect(0, 0, m_renderAreaSize.width(), m_renderAreaSize.height());
        maskPainter.setRenderHint(QPainter::Antialiasing, true);

        maskPainter.resetTransform();
        maskPainter.translate(rect.width()/2, rect.height()/2);
        maskPainter.scale(rect.width()/2, rect.height()/2);

        maskPainter.setPen(QPen(QBrush(Qt::white), 0.002));
        maskPainter.setBrush(QColor(128,128,128,255)); // middle gray

        maskPainter.drawEllipse(QPointF(0,0), 1.0, 1.0);

        maskPainter.resetTransform();

        maskPainter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        m_currentGamutMask->paint(maskPainter, *m_viewConverter, m_maskPreviewActive);

        maskPainter.setCompositionMode(QPainter::CompositionMode_SourceOver);
        m_currentGamutMask->paintStroke(maskPainter, *m_viewConverter, m_maskPreviewActive);

        painter->drawImage(m_renderAreaOffsetX, m_renderAreaOffsetY, maskBuffer);
    }

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

KoColor KisColorSelectorWheel::colorAt(int x, int y, bool forceValid)
{
    KoColor color(Qt::transparent, m_parent->colorSpace());

    Q_ASSERT(x>=0 && x<=width());
    Q_ASSERT(y>=0 && y<=height());

    const qreal xRel = x - 0.5 * width();
    const qreal yRel = y - 0.5 * height();
    const qreal minDimension = qMin(width(), height());

    qreal radius = sqrt(xRel*xRel+yRel*yRel);
    if (radius > 0.5 * minDimension) {
        if (!forceValid) {
            return color;
        } else {
            radius = 0.5 * minDimension;
        }
    }
    radius /= 0.5 * minDimension;

    qreal angle = std::atan2(yRel, xRel);
    angle += M_PI;
    angle /= 2 * M_PI;

    switch(m_parameter) {
    case KisColorSelectorConfiguration::hsvSH:
        color = m_parent->converter()->fromHsvF(angle, radius, m_value);
        break;
    case KisColorSelectorConfiguration::hslSH:
        color = m_parent->converter()->fromHslF(angle, radius, m_lightness);
        break;
    case KisColorSelectorConfiguration::hsiSH:
        color = m_parent->converter()->fromHsiF(angle, radius, m_intensity);
        break;
    case KisColorSelectorConfiguration::hsySH:
        color = m_parent->converter()->fromHsyF(angle, radius, m_luma, R, G, B, Gamma);
        break;
    case KisColorSelectorConfiguration::VH:
        color = m_parent->converter()->fromHsvF(angle, m_hsvSaturation, radius);
        break;
    case KisColorSelectorConfiguration::LH:
        color = m_parent->converter()->fromHslF(angle, m_hslSaturation, radius);
        break;
    case KisColorSelectorConfiguration::IH:
        color = m_parent->converter()->fromHsiF(angle, m_hsiSaturation, radius);
        break;
    case KisColorSelectorConfiguration::YH:
        color = m_parent->converter()->fromHsyF(angle, m_hsySaturation, radius, R, G, B, Gamma);
        break;
    default:
        Q_ASSERT(false);

        return color;
    }
    return color;
}

bool KisColorSelectorWheel::allowsColorSelectionAtPoint(const QPoint &pt) const
{
    return !m_gamutMaskOn || !m_currentGamutMask ||
        m_currentGamutMask->coordIsClear(m_toRenderArea.map(QPointF(pt)),
                                         *m_viewConverter, m_maskPreviewActive);
}
