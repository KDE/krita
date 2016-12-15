/*
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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
#include "KoAlphaF32ColorSpace.h"

#include <limits.h>
#include <stdlib.h>

#include <QImage>
#include <QBitArray>

#include <klocalizedstring.h>

#include "KoChannelInfo.h"
#include "KoID.h"
#include "KoIntegerMaths.h"
#include "KoCompositeOpOver.h"
#include "KoCompositeOpErase.h"
#include "KoCompositeOpCopy2.h"
#include "KoCompositeOpAlphaDarken.h"
#include <colorprofiles/KoDummyColorProfile.h>

KoAlphaF32ColorSpace::KoAlphaF32ColorSpace() : KoColorSpaceAbstract<AlphaF32Traits>("ALPHAF32", i18n("32 bits float alpha mask"))
{
    addChannel(new KoChannelInfo(i18n("Alpha"), 0, 0, KoChannelInfo::ALPHA, KoChannelInfo::FLOAT32));

    m_compositeOps << new KoCompositeOpOver<AlphaF32Traits>(this)
            << new KoCompositeOpErase<AlphaF32Traits>(this)
            << new KoCompositeOpCopy2<AlphaF32Traits>(this)
            << new KoCompositeOpAlphaDarken<AlphaF32Traits>(this);

    Q_FOREACH (KoCompositeOp *op, m_compositeOps) {
        addCompositeOp(op);
    }
    m_profile = new KoDummyColorProfile;
}

KoAlphaF32ColorSpace::~KoAlphaF32ColorSpace()
{
    qDeleteAll(m_compositeOps);
    delete m_profile;
    m_profile = 0;
}

void KoAlphaF32ColorSpace::fromQColor(const QColor& c, quint8 *dst, const KoColorProfile * /*profile*/) const
{
    dst[0] = c.alpha();
}

void KoAlphaF32ColorSpace::toQColor(const quint8 * src, QColor *c, const KoColorProfile * /*profile*/) const
{
    c->setRgba(qRgba(255, 255, 255, src[0]));
}

quint8 KoAlphaF32ColorSpace::difference(const quint8 *src1, const quint8 *src2) const
{
    // Arithmetic operands smaller than int are converted to int automatically
    return qAbs(src2[0] - src1[0]);
}

quint8 KoAlphaF32ColorSpace::differenceA(const quint8 *src1, const quint8 *src2) const
{
    return difference(src1, src2);
}

QString KoAlphaF32ColorSpace::channelValueText(const quint8 *pixel, quint32 channelIndex) const
{
    Q_ASSERT(channelIndex < channelCount());
    quint32 channelPosition = channels()[channelIndex]->pos();

    return QString().setNum(pixel[channelPosition]);
}

QString KoAlphaF32ColorSpace::normalisedChannelValueText(const quint8 *pixel, quint32 channelIndex) const
{
    Q_ASSERT(channelIndex < channelCount());
    quint32 channelPosition = channels()[channelIndex]->pos();

    return QString().setNum(static_cast<float>(pixel[channelPosition]) / UINT8_MAX);
}

void KoAlphaF32ColorSpace::convolveColors(quint8** colors, qreal * kernelValues, quint8 *dst, qreal factor, qreal offset, qint32 nColors, const QBitArray & channelFlags) const
{
    qreal totalAlpha = 0;

    while (nColors--) {
        qreal weight = *kernelValues;

        if (weight != 0) {
            totalAlpha += (*colors)[0] * weight;
        }
        ++colors;
        ++kernelValues;
    }

    if (channelFlags.isEmpty() || channelFlags.testBit(0))
        dst[0] = CLAMP((totalAlpha / factor) + offset, 0, SCHAR_MAX);
}


QImage KoAlphaF32ColorSpace::convertToQImage(const quint8 *data, qint32 width, qint32 height,
                                          const KoColorProfile *  /*dstProfile*/,
                                          KoColorConversionTransformation::Intent /*renderingIntent*/,
                                          KoColorConversionTransformation::ConversionFlags /*conversionFlags*/) const
{
    QImage img(width, height, QImage::Format_Indexed8);
    QVector<QRgb> table;
    for (int i = 0; i < 256; ++i) table.append(qRgb(i, i, i));
    img.setColorTable(table);

    quint8* data_img;
    for (int i = 0; i < height; ++i) {
        data_img=img.scanLine(i);
        for (int j = 0; j < width; ++j)
            data_img[j]=*(data++);
    }

    return img;
}

KoColorSpace* KoAlphaF32ColorSpace::clone() const
{
    return new KoAlphaF32ColorSpace();
}

bool KoAlphaF32ColorSpace::preferCompositionInSourceColorSpace() const
{
    return true;
}
