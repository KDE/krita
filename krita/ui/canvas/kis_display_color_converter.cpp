/*
 *  Copyright (c) 2014 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_display_color_converter.h"

#include <KoColor.h>
#include <KoColorSpaceRegistry.h>

#include "kundo2command.h"
#include "kis_config.h"
#include "kis_paint_device.h"


struct KisDisplayColorConverter::Private
{
    const KoColorSpace *imageColorSpace;
    const KoColorSpace *monitorColorSpace;

    const KoColorProfile *monitorProfile;

    KoColorConversionTransformation::Intent renderingIntent;
    KoColorConversionTransformation::ConversionFlags conversionFlags;

    KisDisplayFilterSP displayFilter;
};


KisDisplayColorConverter::KisDisplayColorConverter(const KoColorSpace *imageColorSpace,
                                                   const KoColorProfile *monitorProfile,
                                                   KisDisplayFilterSP displayFilter)
    : m_d(new Private)
{
    m_d->imageColorSpace = imageColorSpace;
    m_d->monitorColorSpace = KoColorSpaceRegistry::instance()->rgb8(monitorProfile);

    m_d->monitorProfile = monitorProfile;

    m_d->renderingIntent = renderingIntent();
    m_d->conversionFlags = conversionFlags();

    m_d->displayFilter = displayFilter;
}

KisDisplayColorConverter::~KisDisplayColorConverter()
{
}

KoColorConversionTransformation::Intent
KisDisplayColorConverter::renderingIntent()
{
    KisConfig cfg;
    return (KoColorConversionTransformation::Intent)cfg.renderIntent();
}

KoColorConversionTransformation::ConversionFlags
KisDisplayColorConverter::conversionFlags()
{
    KoColorConversionTransformation::ConversionFlags conversionFlags =
        KoColorConversionTransformation::HighQuality;

    KisConfig cfg;

    if (cfg.useBlackPointCompensation()) conversionFlags |= KoColorConversionTransformation::BlackpointCompensation;
    if (!cfg.allowLCMSOptimization()) conversionFlags |= KoColorConversionTransformation::NoOptimization;

    return conversionFlags;
}

QColor KisDisplayColorConverter::toQColor(const KoColor &srcColor)
{
    KoColor c(srcColor);
    c.convertTo(m_d->imageColorSpace);

    QByteArray pixel(m_d->monitorColorSpace->pixelSize(), 0);
    c.colorSpace()->convertPixelsTo(c.data(), (quint8*)pixel.data(),
                                     m_d->monitorColorSpace, 1,
                                     m_d->renderingIntent, m_d->conversionFlags);


    // we expect the display profile is rgb8, which is BGRA here
    KIS_ASSERT_RECOVER(m_d->monitorColorSpace->pixelSize() == 4) { return Qt::red; };

    const quint8 *p = (const quint8 *)pixel.constData();
    return QColor(p[2], p[1], p[0], p[3]);
}

QImage KisDisplayColorConverter::toQImage(KisPaintDeviceSP srcDevice)
{
    KisPaintDeviceSP device = srcDevice;

    if (!(*device->colorSpace() == *m_d->imageColorSpace)) {
        device = new KisPaintDevice(*srcDevice);

        KUndo2Command *cmd = device->convertTo(m_d->imageColorSpace);
        delete cmd;
    }

    return device->convertToQImage(m_d->monitorProfile, m_d->renderingIntent, m_d->conversionFlags);
}

KoColor KisDisplayColorConverter::fromHsvF(qreal h, qreal s, qreal v, qreal a)
{
    // generate HSV from sRGB!
    KoColor color(m_d->imageColorSpace);
    color.fromQColor(QColor::fromHsvF(h, s, v, a));

    // sanity check
    // KIS_ASSERT_RECOVER_NOOP(m_d->imageColorSpace == color.colorSpace());

    return color;
}

void KisDisplayColorConverter::getHsvF(const KoColor &srcColor, qreal *h, qreal *s, qreal *v, qreal *a)
{
    // we are going through sRGB here!
    QColor color = srcColor.toQColor();
    color.getHsvF(h, s, v, a);
}

KoColor KisDisplayColorConverter::fromHslF(qreal h, qreal s, qreal l, qreal a)
{
    // generate HSL from sRGB!
    KoColor color(m_d->imageColorSpace);
    color.fromQColor(QColor::fromHslF(h, s, l, a));

    // sanity check
    // KIS_ASSERT_RECOVER_NOOP(m_d->imageColorSpace == color.colorSpace());

    return color;
}

void KisDisplayColorConverter::getHslF(const KoColor &srcColor, qreal *h, qreal *s, qreal *l, qreal *a)
{
    // we are going through sRGB here!
    QColor color = srcColor.toQColor();
    color.getHslF(h, s, l, a);
}
