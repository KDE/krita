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
#include "KoAlphaF16ColorSpace.h"

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

KoAlphaF16ColorSpace::KoAlphaF16ColorSpace() : KoColorSpaceAbstract<AlphaF16Traits>("ALPHAF16", i18n("16 bits float alpha mask"))
{
    addChannel(new KoChannelInfo(i18n("Alpha"), 0, 0, KoChannelInfo::ALPHA, KoChannelInfo::FLOAT16));

    m_compositeOps << new KoCompositeOpOver<AlphaF16Traits>(this)
            << new KoCompositeOpErase<AlphaF16Traits>(this)
            << new KoCompositeOpCopy2<AlphaF16Traits>(this)
            << new KoCompositeOpAlphaDarken<AlphaF16Traits>(this);

    Q_FOREACH (KoCompositeOp *op, m_compositeOps) {
        addCompositeOp(op);
    }
    m_profile = new KoDummyColorProfile;
}

KoAlphaF16ColorSpace::~KoAlphaF16ColorSpace()
{
    qDeleteAll(m_compositeOps);
    delete m_profile;
    m_profile = 0;
}

void KoAlphaF16ColorSpace::fromQColor(const QColor& c, quint8 *dst, const KoColorProfile * /*profile*/) const
{
    dst[0] = c.alpha();
}

void KoAlphaF16ColorSpace::toQColor(const quint8 * src, QColor *c, const KoColorProfile * /*profile*/) const
{
    c->setRgba(qRgba(255, 255, 255, src[0]));
}

quint8 KoAlphaF16ColorSpace::difference(const quint8 *src1, const quint8 *src2) const
{
    // Arithmetic operands smaller than int are converted to int automatically
    return qAbs(src2[0] - src1[0]);
}

quint8 KoAlphaF16ColorSpace::differenceA(const quint8 *src1, const quint8 *src2) const
{
    return difference(src1, src2);
}

QString KoAlphaF16ColorSpace::channelValueText(const quint8 *pixel, quint32 channelIndex) const
{
    Q_ASSERT(channelIndex < channelCount());
    quint32 channelPosition = channels()[channelIndex]->pos();

    return QString().setNum(pixel[channelPosition]);
}

QString KoAlphaF16ColorSpace::normalisedChannelValueText(const quint8 *pixel, quint32 channelIndex) const
{
    Q_ASSERT(channelIndex < channelCount());
    quint32 channelPosition = channels()[channelIndex]->pos();

    return QString().setNum(static_cast<float>(pixel[channelPosition]) / UINT8_MAX);
}

void KoAlphaF16ColorSpace::convolveColors(quint8** /*colors*/, qreal * /*kernelValues*/, quint8 */*dst*/, qreal /*factor*/, qreal /*offset*/, qint32 /*nColors*/, const QBitArray & /*channelFlags*/) const
{
    warnPigment << i18n("Undefined operation in the alpha color space");
}


QImage KoAlphaF16ColorSpace::convertToQImage(const quint8 *data, qint32 width, qint32 height,
                                          const KoColorProfile *  /*dstProfile*/,
                                          KoColorConversionTransformation::Intent /*renderingIntent*/,
                                          KoColorConversionTransformation::ConversionFlags /*conversionFlags*/) const
{
    warnPigment << i18n("Undefined operation in the alpha color space");
    QImage img(width, height, QImage::Format_Indexed8);
    return img;
}

KoColorSpace *KoAlphaF16ColorSpace::clone() const
{
    return new KoAlphaF16ColorSpace();
}

bool KoAlphaF16ColorSpace::preferCompositionInSourceColorSpace() const
{
    return true;
}
