/*
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef KOLABCOLORSPACE_H
#define KOLABCOLORSPACE_H

#include <QColor>
#include <QBitArray>

#include "DebugPigment.h"

#include "KoSimpleColorSpace.h"
#include "KoColorSpaceTraits.h"
#include "KoSimpleColorSpaceFactory.h"
#include "KoColorModelStandardIds.h"

/**
 * Basic and simple implementation of the LAB colorspace
 */
class KoLabColorSpace : public KoSimpleColorSpace<KoLabU16Traits>
{

public:

    KoLabColorSpace();

    virtual ~KoLabColorSpace();

    static QString colorSpaceId();

    virtual KoColorSpace* clone() const;

    virtual void fromQColor(const QColor& color, quint8 *dst, const KoColorProfile * profile = 0) const;

    virtual void toQColor(const quint8 *src, QColor *c, const KoColorProfile * profile = 0) const;

private:

    static const quint32 CHANNEL_L = 0;
    static const quint32 CHANNEL_A = 1;
    static const quint32 CHANNEL_B = 2;
    static const quint32 CHANNEL_ALPHA = 3;
    static const quint32 MAX_CHANNEL_L = 0xff00;
    static const quint32 MAX_CHANNEL_AB = 0xffff;
    static const quint32 CHANNEL_AB_ZERO_OFFSET = 0x8000;


};

class KoLabColorSpaceFactory : public KoSimpleColorSpaceFactory
{

public:

    KoLabColorSpaceFactory()
            : KoSimpleColorSpaceFactory("LABA",
                                        i18n("L*a*b* (16-bit integer/channel, unmanaged)"),
                                        true,
                                        LABAColorModelID,
                                        Integer16BitsColorDepthID,
                                        16) {
    }

    virtual KoColorSpace *createColorSpace(const KoColorProfile *) const {
        return new KoLabColorSpace();
    }

};

#endif
