/*
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2007 Emanuele Tamponi <emanuele@valinor.it>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef KO_COMPOSITE_COPY_OP_ABSTRACT_H
#define KO_COMPOSITE_COPY_OP_ABSTRACT_H

/**
 * Generic implementation of the COPY composite op.
 * Used automatically by all colorspaces that derive from KoColorSpaceAbstract.
 */
class KoCompositeOpCopy : public KoCompositeOp
{

    using KoCompositeOp::composite;

public:

    explicit KoCompositeOpCopy(KoColorSpace * cs)
            : KoCompositeOp(cs, COMPOSITE_COPY, i18n("Copy"), KoCompositeOp::categoryMix()) {
    }

public:

    void composite(quint8 *dstRowStart,
                   qint32 dstRowStride,
                   const quint8 *srcRowStart,
                   qint32 srcRowStride,
                   const quint8 *maskRowStart,
                   qint32 maskRowStride,
                   qint32 rows,
                   qint32 numColumns,
                   quint8 opacity,
                   const QBitArray & channelFlags) const {

        Q_UNUSED(maskRowStart);
        Q_UNUSED(maskRowStride);
        Q_UNUSED(channelFlags);
        Q_UNUSED(opacity);

        qint32 srcInc = (srcRowStride == 0) ? 0 : colorSpace()->pixelSize();

        quint8 *dst = dstRowStart;
        const quint8 *src = srcRowStart;
        const KoColorSpace* cs = colorSpace();
        qint32 bytesPerPixel = cs->pixelSize();

        while (rows > 0) {
            if (srcInc == 0) {
                quint8* dstN = dst;
                qint32 columns = numColumns;
                while (columns > 0) {
                    memcpy(dstN, src, bytesPerPixel);
                    dstN += bytesPerPixel;
                    columns--;
                }
            } else {
                memcpy(dst, src, numColumns * bytesPerPixel);
            }

            // XXX: what is the reason for this code? I think we should copy the alpha channel as well.
            //if (opacity != OPACITY_OPAQUE) {
            //    cs->multiplyAlpha(dst, opacity, numColumns);
            //}

            dst += dstRowStride;
            src += srcRowStride;
            --rows;
        }
    }
};

#endif
