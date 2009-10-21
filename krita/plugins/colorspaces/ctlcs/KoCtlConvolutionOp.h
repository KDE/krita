/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
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

#ifndef _KOCTL_CONVOLUTION_OP_H_
#define _KOCTL_CONVOLUTION_OP_H_

#include <KoColorSpace.h>

class KoCtlColorSpace;
class KoCtlAccumulator;
class KoCtlColorSpaceInfo;

class KoCtlConvolutionOp : public KoConvolutionOp
{
public:
    KoCtlConvolutionOp(KoCtlColorSpace* _colorSpace, const KoCtlColorSpaceInfo* _info);
    virtual ~KoCtlConvolutionOp();
    virtual void convolveColors(const quint8* const* colors, const qint32* kernelValues, quint8 *dst, qint32 factor, qint32 offset, qint32 nPixels, const QBitArray & channelFlags) const;
private:
    QList<KoCtlAccumulator*> m_accumulators;
    KoCtlColorSpace* m_colorSpace;
};

#endif
