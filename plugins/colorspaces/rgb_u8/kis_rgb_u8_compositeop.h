/*
 *  Copyright (c) 2006 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software const; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation const; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY const; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program const; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

 Some code is derived from GraphicsMagick/magick/composite.c and is 
 subject to the following license and copyright:

  Copyright (C) 2002 GraphicsMagick Group, an organization dedicated
  to making software imaging solutions freely available.

  Permission is hereby granted, free of charge, to any person obtaining
  a copy of this software and associated documentation files
  ("GraphicsMagick"), to deal in GraphicsMagick without restriction,
  including without limitation the rights to use, copy, modify, merge,
  publish, distribute, sublicense, and/or sell copies of GraphicsMagick,
  and to permit persons to whom GraphicsMagick is furnished to do so,
  subject to the following conditions:

  The above copyright notice and this permission notice shall be included
  in all copies or substantial portions of GraphicsMagick.

  The software is provided "as is", without warranty of any kind, express
  or implied, including but not limited to the warranties of
  merchantability, fitness for a particular purpose and noninfringement.
  In no event shall GraphicsMagick Group be liable for any claim,
  damages or other liability, whether in an action of contract, tort or
  otherwise, arising from, out of or in connection with GraphicsMagick
  or the use or other dealings in GraphicsMagick.

  Except as contained in this notice, the name of the GraphicsMagick
  Group shall not be used in advertising or otherwise to promote the
  sale, use or other dealings in GraphicsMagick without prior written
  authorization from the GraphicsMagick Group.

   Other code is derived from gwenview/src/qxcfi.* - this is released under
  the terms of the LGPL
 
 */
#ifndef KIS_RGB_U8_COMPOSITEOP
#define KIS_RGB_U8_COMPOSITEOP

#include <KoCompositeOp.h>

/**
 * Ugly class that contains all rgb composite ops
 */
class KisRgbU8CompositeOp : public KoCompositeOp {
public:

    KisRgbU8CompositeOp(KoColorSpace * cs, const QString& id, const QString& description, const bool userVisible = true);

    virtual ~KisRgbU8CompositeOp() {};

    void composite(quint8 *dstRowStart, qint32 dstRowStride, 
			   const quint8 *srcRowStart, qint32 srcRowStride, 
			   const quint8 *maskRowStart, qint32 maskRowStride, 
			   qint32 rows, qint32 numColumns, 
			   quint8 opacity,
			   const QBitArray & channelFlags) const;

private:
    void compositeOver(quint8 *dst, qint32 dstRowSize, const quint8 *src, qint32 srcRowSize, const quint8 *mask, qint32 maskRowSize, qint32 rows, qint32 columns, quint8 opacity) const;
    void compositeMultiply(quint8 *dst, qint32 dstRowSize, const quint8 *src, qint32 srcRowSize, const quint8 *mask, qint32 maskRowSize, qint32 rows, qint32 columns, quint8 opacity) const;
    void compositeDivide(quint8 *dst, qint32 dstRowSize, const quint8 *src, qint32 srcRowSize, const quint8 *mask, qint32 maskRowSize, qint32 rows, qint32 columns, quint8 opacity) const;
    void compositeScreen(quint8 *dst, qint32 dstRowSize, const quint8 *src, qint32 srcRowSize, const quint8 *mask, qint32 maskRowSize, qint32 rows, qint32 columns, quint8 opacity) const;
    void compositeOverlay(quint8 *dst, qint32 dstRowSize, const quint8 *src, qint32 srcRowSize, const quint8 *mask, qint32 maskRowSize, qint32 rows, qint32 columns, quint8 opacity) const;
    void compositeDodge(quint8 *dst, qint32 dstRowSize, const quint8 *src, qint32 srcRowSize, const quint8 *mask, qint32 maskRowSize, qint32 rows, qint32 columns, quint8 opacity) const;
    void compositeBurn(quint8 *dst, qint32 dstRowSize, const quint8 *src, qint32 srcRowSize, const quint8 *mask, qint32 maskRowSize, qint32 rows, qint32 columns, quint8 opacity) const;
    void compositeDarken(quint8 *dst, qint32 dstRowSize, const quint8 *src, qint32 srcRowSize, const quint8 *mask, qint32 maskRowSize, qint32 rows, qint32 columns, quint8 opacity) const;
    void compositeLighten(quint8 *dst, qint32 dstRowSize, const quint8 *src, qint32 srcRowSize, const quint8 *mask, qint32 maskRowSize, qint32 rows, qint32 columns, quint8 opacity) const;
    void compositeHue(quint8 *dst, qint32 dstRowSize, const quint8 *src, qint32 srcRowSize, const quint8 *mask, qint32 maskRowSize, qint32 rows, qint32 columns, quint8 opacity) const;
    void compositeSaturation(quint8 *dst, qint32 dstRowSize, const quint8 *src, qint32 srcRowSize, const quint8 *mask, qint32 maskRowSize, qint32 rows, qint32 columns, quint8 opacity) const;
    void compositeValue(quint8 *dst, qint32 dstRowSize, const quint8 *src, qint32 srcRowSize, const quint8 *mask, qint32 maskRowSize, qint32 rows, qint32 columns, quint8 opacity) const;
    void compositeColor(quint8 *dst, qint32 dstRowSize, const quint8 *src, qint32 srcRowSize, const quint8 *mask, qint32 maskRowSize, qint32 rows, qint32 columns, quint8 opacity) const;
    void compositeErase(quint8 *dst, qint32 dstRowSize, const quint8 *src, qint32 srcRowSize, const quint8 *mask, qint32 maskRowSize, qint32 rows, qint32 columns, quint8 opacity) const;

    void compositeIn(qint32 pixelSize, quint8 *dst, qint32 dstRowSize, const quint8 *src, qint32 srcRowSize,qint32 rows, qint32 cols, quint8 opacity = OPACITY_OPAQUE) const;
    void compositeOut(qint32 pixelSize, quint8 *dst, qint32 dstRowSize, const quint8 *src, qint32 srcRowSize,qint32 rows, qint32 cols, quint8 opacity = OPACITY_OPAQUE) const;
    void compositeAtop(qint32 pixelSize, quint8 *dst, qint32 dstRowSize, const quint8 *src, qint32 srcRowSize,qint32 rows, qint32 cols, quint8 opacity = OPACITY_OPAQUE) const;
    void compositeXor(qint32 pixelSize, quint8 *dst, qint32 dstRowSize, const quint8 *src, qint32 srcRowSize,qint32 rows, qint32 cols, quint8 opacity = OPACITY_OPAQUE) const;
    void compositePlus(qint32 pixelSize, quint8 *dst, qint32 dstRowSize, const quint8 *src, qint32 srcRowSize,qint32 rows, qint32 cols, quint8 opacity = OPACITY_OPAQUE) const;
    void compositeMinus(qint32 pixelSize, quint8 *dst, qint32 dstRowSize, const quint8 *src, qint32 srcRowSize,qint32 rows, qint32 cols, quint8 opacity = OPACITY_OPAQUE) const;
    void compositeAdd(qint32 pixelSize, quint8 *dst, qint32 dstRowSize, const quint8 *src, qint32 srcRowSize,qint32 rows, qint32 cols, quint8 opacity = OPACITY_OPAQUE) const;
    void compositeSubtract(qint32 pixelSize, quint8 *dst, qint32 dstRowSize, const quint8 *src, qint32 srcRowSize,qint32 rows, qint32 cols, quint8 opacity = OPACITY_OPAQUE) const;
    void compositeDiff(qint32 pixelSize, quint8 *dst, qint32 dstRowSize, const quint8 *src, qint32 srcRowSize,qint32 rows, qint32 cols, quint8 opacity = OPACITY_OPAQUE) const;
    void compositeBumpmap(qint32 pixelSize, quint8 *dst, qint32 dstRowSize, const quint8 *src, qint32 srcRowSize,qint32 rows, qint32 cols, quint8 opacity = OPACITY_OPAQUE) const;
    void compositeCopyChannel(quint8 pixel, quint32 pixelSize, quint8 *dst, qint32 dstRowSize, const quint8 *src, qint32 srcRowSize,qint32 rows, qint32 cols, quint8 opacity = OPACITY_OPAQUE) const;
    void compositeCopyRed(qint32 pixelSize, quint8 *dst, qint32 dstRowSize, const quint8 *src, qint32 srcRowSize,qint32 rows, qint32 cols, quint8 opacity = OPACITY_OPAQUE) const;
    void compositeCopyGreen(qint32 pixelSize, quint8 *dst, qint32 dstRowSize, const quint8 *src, qint32 srcRowSize,qint32 rows, qint32 cols, quint8 opacity = OPACITY_OPAQUE) const;
    void compositeCopyBlue(qint32 pixelSize, quint8 *dst, qint32 dstRowSize, const quint8 *src, qint32 srcRowSize,qint32 rows, qint32 cols, quint8 opacity = OPACITY_OPAQUE) const;
    void compositeCopyOpacity(qint32 pixelSize, quint8 *dst, qint32 dstRowSize, const quint8 *src, qint32 srcRowSize,qint32 rows, qint32 cols, quint8 opacity = OPACITY_OPAQUE) const;
    void compositeClear(qint32 pixelSize, quint8 *dst, qint32 dstRowSize, const quint8 *src, qint32 srcRowSize,qint32 rows, qint32 cols, quint8 opacity = OPACITY_OPAQUE) const;
    void compositeDissolve(qint32 pixelSize, quint8 *dst, qint32 dstRowSize, const quint8 *src, qint32 srcRowSize,qint32 rows, qint32 cols, quint8 opacity = OPACITY_OPAQUE) const;
    void compositeCopy(qint32 pixelSize, quint8 *dst, qint32 dstRowSize, const quint8 *src, qint32 srcRowSize, const quint8 *mask, qint32 maskRowStride,qint32 rows, qint32 cols, quint8 opacity = OPACITY_OPAQUE) const;
private:
        quint8 m_pixelSize;
};



#endif
