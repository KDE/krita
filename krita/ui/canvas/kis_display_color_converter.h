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

#ifndef __KIS_DISPLAY_COLOR_CONVERTER_H
#define __KIS_DISPLAY_COLOR_CONVERTER_H

#include <QScopedPointer>

#include <KoColorConversionTransformation.h>

#include "kis_types.h"
#include "canvas/kis_display_filter.h"

class KoColor;
class KoColorProfile;


/**
 * Special helper class that provides primitives for converting colors when
 * displaying. We have at least 3 color spaces:
 *
 * 1) Image color space (any: RGB, CMYK, Lab, etc)
 * 2) Display color space (a limited RGB color space)
 * 3) Color selectors color space (the one where color selectors generate
 *    their HSV-based colors. Right now it is sRGB.
 *
 * KoColor may be in any of these color spaces. QColor should always
 * be in the display color space only.
 */
class KRITAUI_EXPORT KisDisplayColorConverter
{
public:
    KisDisplayColorConverter(const KoColorSpace *imageColorSpace,
                             const KoColorProfile *monitorProfile,
                             KisDisplayFilterSP displayFilter);

    virtual ~KisDisplayColorConverter();

    QColor toQColor(const KoColor &c);
    QImage toQImage(KisPaintDeviceSP srcDevice);

    KoColor fromHsvF(qreal h, qreal s, qreal v, qreal a = 1.0);
    KoColor fromHslF(qreal h, qreal s, qreal l, qreal a = 1.0);

    void getHsvF(const KoColor &srcColor, qreal *h, qreal *s, qreal *v, qreal *a = 0);

    static KoColorConversionTransformation::Intent renderingIntent();
    static KoColorConversionTransformation::ConversionFlags conversionFlags();

private:
    // is not possible to implement!
    KoColor toKoColor(const QColor &c);

private:
    struct Private;
    Private * const m_d;
};

#endif /* __KIS_DISPLAY_COLOR_CONVERTER_H */
