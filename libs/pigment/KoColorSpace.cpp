/*
 *  Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "KoColorSpace.h"

#include <kdebug.h>

#include "KoCompositeOp.h"

KoColorSpace::KoColorSpace(const QString &id, const QString &name, KoColorSpaceRegistry * parent)
    : m_id(id)
    , m_name( name )
    , m_parent( parent )
{
    //m_dcop = 0;
}

KoColorSpace::~KoColorSpace()
{
    //delete m_dcop;
}

quint8 *KoColorSpace::allocPixelBuffer(quint32 numPixels) const
{
    return new quint8[pixelSize()*numPixels];
}

KoCompositeOpList KoColorSpace::userVisiblecompositeOps() const
{
    return m_compositeOps.values();
}

const KoCompositeOp * KoColorSpace::compositeOp(const QString & id) const
{
    if ( m_compositeOps.contains( id ) )
        return m_compositeOps.value( id );
    else
        return m_compositeOps.value( COMPOSITE_OVER );
}

void KoColorSpace::addCompositeOp(const KoCompositeOp * op)
{
    if ( op->colorSpace()->id() == id()) {
        m_compositeOps.insert( op->id(), const_cast<KoCompositeOp*>( op ) );
    }
}


bool KoColorSpace::convertPixelsTo(const quint8 * src,
                                   quint8 * dst,
                                   const KoColorSpace * dstColorSpace,
                                   quint32 numPixels,
                                   qint32 renderingIntent) const
{
    Q_UNUSED(renderingIntent);
    // 4 channels: labA, 2 bytes per lab channel
    quint8 *pixels = new quint8[2*4*numPixels];

    toLabA16(src, pixels,numPixels);
    dstColorSpace->fromLabA16(pixels, dst,numPixels);

    delete [] pixels;

    return true;
}

void KoColorSpace::bitBlt(quint8 *dst,
                          qint32 dststride,
                          KoColorSpace * srcSpace,
                          const quint8 *src,
                          qint32 srcRowStride,
                          const quint8 *srcAlphaMask,
                          qint32 maskRowStride,
                          quint8 opacity,
                          qint32 rows,
                          qint32 cols,
                          const QString & op,
                          const QBitArray & channelFlags) const
{
    if ( m_compositeOps.contains( op ) ) {
        bitBlt(dst, dststride, srcSpace, src, srcRowStride, srcAlphaMask, maskRowStride, opacity, rows, cols, m_compositeOps.value( op ), channelFlags);
    }
    else {
        bitBlt(dst, dststride, srcSpace, src, srcRowStride, srcAlphaMask, maskRowStride, opacity, rows, cols, m_compositeOps.value( COMPOSITE_OVER ), channelFlags);
    }

}

void KoColorSpace::bitBlt(quint8 *dst,
                          qint32 dststride,
                          KoColorSpace * srcSpace,
                          const quint8 *src,
                          qint32 srcRowStride,
                          const quint8 *srcAlphaMask,
                          qint32 maskRowStride,
                          quint8 opacity,
                          qint32 rows,
                          qint32 cols,
                          const QString& op) const
{
    if ( m_compositeOps.contains( op ) ) {
        bitBlt(dst, dststride, srcSpace, src, srcRowStride, srcAlphaMask, maskRowStride, opacity, rows, cols, m_compositeOps.value( op ));
    }
    else {
        bitBlt(dst, dststride, srcSpace, src, srcRowStride, srcAlphaMask, maskRowStride, opacity, rows, cols, m_compositeOps.value( COMPOSITE_OVER ) );
    }
}

void KoColorSpace::bitBlt(quint8 *dst,
                                   qint32 dststride,
                                   KoColorSpace * srcSpace,
                                   const quint8 *src,
                                   qint32 srcRowStride,
                                   const quint8 *srcAlphaMask,
                                   qint32 maskRowStride,
                                   quint8 opacity,
                                   qint32 rows,
                                   qint32 cols,
                                   const KoCompositeOp * op,
                                   const QBitArray & channelFlags) const
{
    if (rows <= 0 || cols <= 0)
        return;

    if (this != srcSpace) {
        quint32 len = pixelSize() * rows * cols;

        // If our conversion cache is too small, extend it.
        if (!m_conversionCache.resize( len, Q3GArray::SpeedOptim )) {
            kWarning() << "Could not allocate enough memory for the conversion!\n";
            // XXX: We should do a slow, pixel by pixel bitblt here...
            abort();
        }

        for (qint32 row = 0; row < rows; row++) {
            srcSpace->convertPixelsTo(src + row * srcRowStride,
                                      m_conversionCache.data() + row * cols * pixelSize(), this,
                                      cols);
        }

        // The old srcRowStride is no longer valid because we converted to the current cs
        srcRowStride = cols * pixelSize();

        op->composite( dst, dststride,
                       m_conversionCache.data(), srcRowStride,
                       srcAlphaMask, maskRowStride,
                       rows,  cols,
                       opacity, channelFlags );

    }
    else {
        op->composite( dst, dststride,
                       src, srcRowStride,
                       srcAlphaMask, maskRowStride,
                       rows,  cols,
                       opacity, channelFlags );
    }
}

// XXX: I don't want this code duplication, but also don't want an
//      extra function call in this critical section of code. What to
//      do?
void KoColorSpace::bitBlt(quint8 *dst,
                                   qint32 dststride,
                                   KoColorSpace * srcSpace,
                                   const quint8 *src,
                                   qint32 srcRowStride,
                                   const quint8 *srcAlphaMask,
                                   qint32 maskRowStride,
                                   quint8 opacity,
                                   qint32 rows,
                                   qint32 cols,
                                   const KoCompositeOp * op) const
{
    if (rows <= 0 || cols <= 0)
        return;

    if (this != srcSpace) {
        quint32 len = pixelSize() * rows * cols;

        // If our conversion cache is too small, extend it.
        if (!m_conversionCache.resize( len, Q3GArray::SpeedOptim )) {
            kWarning() << "Could not allocate enough memory for the conversion!\n";
            // XXX: We should do a slow, pixel by pixel bitblt here...
            abort();
        }

        for (qint32 row = 0; row < rows; row++) {
            srcSpace->convertPixelsTo(src + row * srcRowStride,
                                      m_conversionCache.data() + row * cols * pixelSize(), this,
                                      cols);
        }

        // The old srcRowStride is no longer valid because we converted to the current cs
        srcRowStride = cols * pixelSize();

        op->composite( dst, dststride,
                       m_conversionCache.data(), srcRowStride,
                       srcAlphaMask, maskRowStride,
                       rows,  cols,
                       opacity);

    }
    else {
        op->composite( dst, dststride,
                       src,srcRowStride,
                       srcAlphaMask, maskRowStride,
                       rows,  cols,
                       opacity);
    }
}
