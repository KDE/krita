/*
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2.1 of the License, or
 *  (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef KORGBU16COLORSPACE_H
#define KORGBU16COLORSPACE_H

#include <QColor>

#include "KoSimpleColorSpace.h"
#include "KoSimpleColorSpaceFactory.h"
#include "KoColorModelStandardIds.h"

struct KoBgrU16Traits;

/**
 * The alpha mask is a special color strategy that treats all pixels as
 * alpha value with a color common to the mask. The default color is white.
 */
class KoRgbU16ColorSpace : public KoSimpleColorSpace<KoBgrU16Traits>
{

public:

    KoRgbU16ColorSpace();
    ~KoRgbU16ColorSpace() override;

    static QString colorSpaceId();

    virtual KoColorSpace* clone() const;

    void fromQColor(const QColor& color, quint8 *dst, const KoColorProfile * profile = 0) const override;

    void toQColor(const quint8 *src, QColor *c, const KoColorProfile * profile = 0) const override;
    
    void toHSY(const QVector<double> &channelValues, qreal *hue, qreal *sat, qreal *luma) const override;
    QVector <double> fromHSY(qreal *hue, qreal *sat, qreal *luma) const override;
    void toYUV(const QVector<double> &channelValues, qreal *y, qreal *u, qreal *v) const override;
    QVector <double> fromYUV(qreal *y, qreal *u, qreal *v) const override;

};

class KoRgbU16ColorSpaceFactory : public KoSimpleColorSpaceFactory
{

public:
    KoRgbU16ColorSpaceFactory()
            : KoSimpleColorSpaceFactory(KoRgbU16ColorSpace::colorSpaceId(),
                                        i18n("RGB (16-bit integer/channel, unmanaged)"),
                                        true,
                                        RGBAColorModelID,
                                        Integer16BitsColorDepthID) {
    }

    KoColorSpace *createColorSpace(const KoColorProfile *) const override {
        return new KoRgbU16ColorSpace();
    }

};


#endif
