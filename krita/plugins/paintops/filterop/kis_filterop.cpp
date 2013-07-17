/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2004-2008 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2004 Clarence Dang <dang@kde.org>
 *  Copyright (c) 2004 Adrian Page <adrian@pagenet.plus.com>
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
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

#include "kis_filterop.h"

#include <kis_debug.h>

#include <KoColorSpaceRegistry.h>
#include <KoColorTransformation.h>
#include <KoColor.h>
#include <KoInputDevice.h>

#include <kis_processing_information.h>
#include <filter/kis_filter_registry.h>
#include <filter/kis_filter.h>
#include <filter/kis_filter_configuration.h>
#include <kis_brush.h>
#include <kis_global.h>
#include <kis_painter.h>
#include <kis_paint_device.h>
#include <kis_properties_configuration.h>
#include <kis_selection.h>
#include <kis_pressure_size_option.h>
#include <kis_filter_option.h>
#include <kis_filterop_settings.h>
#include <kis_iterator_ng.h>
#include <kis_fixed_paint_device.h>

KisFilterOp::KisFilterOp(const KisFilterOpSettings *settings, KisPainter *painter, KisImageWSP image)
        : KisBrushBasedPaintOp(settings, painter)
        , settings(settings)
        , m_filterConfiguration(0)
{
    Q_UNUSED(image);
    Q_ASSERT(settings);
    Q_ASSERT(painter);
    m_tmpDevice = source()->createCompositionSourceDevice();
    m_sizeOption.readOptionSetting(settings);
    m_rotationOption.readOptionSetting(settings);
    m_sizeOption.sensor()->reset();
    m_rotationOption.sensor()->reset();
    m_filter = KisFilterRegistry::instance()->get(settings->getString(FILTER_ID));
    m_filterConfiguration = settings->filterConfig();
    m_smudgeMode = settings->getBool(FILTER_SMUDGE_MODE);
}

KisFilterOp::~KisFilterOp()
{
}

qreal KisFilterOp::paintAt(const KisPaintInformation& info)
{
    if (!painter()) {
        return 1.0;
    }

    if (!m_filter) {
        return 1.0;
    }

    if (!source()) {
        return 1.0;
    }

    KisBrushSP brush = m_brush;;
    if (!brush) return 1.0;

    if (! brush->canPaintFor(info))
        return 1.0;

    qreal scale = m_sizeOption.apply(info);
    if ((scale * brush->width()) <= 0.01 || (scale * brush->height()) <= 0.01) return spacing(scale);

    setCurrentScale(scale);

    qreal rotation = m_rotationOption.apply(info);
    QPointF hotSpot = brush->hotSpot(scale, scale, rotation, info);
    QPointF pt = info.pos() - hotSpot;


    // Split the coordinates into integer plus fractional parts. The integer
    // is where the dab will be positioned and the fractional part determines
    // the sub-pixel positioning.
    qint32 x;
    qreal xFraction;
    qint32 y;
    qreal yFraction;

    splitCoordinate(pt.x(), &x, &xFraction);
    splitCoordinate(pt.y(), &y, &yFraction);

    qint32 maskWidth = brush->maskWidth(scale, rotation, xFraction, yFraction, info);
    qint32 maskHeight = brush->maskHeight(scale, rotation, xFraction, yFraction, info);

    // Filter the paint device
    QRect rect = QRect(0, 0, maskWidth, maskHeight);
    QRect neededRect = m_filter->neededRect(rect.translated(x, y), m_filterConfiguration);

    if (!m_smudgeMode) {
        m_tmpDevice->clear();
    }

    KisPainter p(m_tmpDevice);
    p.bitBltOldData(QPoint(neededRect.x()-x, neededRect.y()-y), source(), neededRect);
    m_filter->process(m_tmpDevice, rect, m_filterConfiguration, 0);

    // Fetch the mask
    static const KoColorSpace *cs = KoColorSpaceRegistry::instance()->alpha8();
    static KoColor color(Qt::black, cs);

    KisFixedPaintDeviceSP fixedDab = m_dabCache->fetchDab(cs,
                                                          color,
                                                          scale, scale,
                                                          rotation,
                                                          info,
                                                          xFraction,
                                                          yFraction);

    // Blit the paint device onto the layer
    QRect dabRect = QRect(0, 0, maskWidth, maskHeight);
    QRect dstRect = QRect(x, y, dabRect.width(), dabRect.height());

    if (dstRect.isNull() || dstRect.isEmpty() || !dstRect.isValid()) return 1.0;

    painter()->
        bitBltWithFixedSelection(dstRect.x(), dstRect.y(),
                                 m_tmpDevice, fixedDab,
                                 0,0,
                                 0,0,
                                 maskWidth, maskHeight);

    return spacing(scale);
}
