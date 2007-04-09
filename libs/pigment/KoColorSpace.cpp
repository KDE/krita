/*
 *  Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
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
#include <QThreadStorage>
#include <QByteArray>
#include <QBitArray>

#include "KoColorSpace.h"
#include "KoChannelInfo.h"
#include <kdebug.h>

#include "KoCompositeOp.h"
#include "KoColorTransformation.h"

struct KoColorSpace::Private {
    QString id;
    QString name;
    QHash<QString, KoCompositeOp *> compositeOps;
    KoColorSpaceRegistry * parent;
    QList<KoChannelInfo *> channels;
    KoMixColorsOp* mixColorsOp;
    KoConvolutionOp* convolutionOp;
    QThreadStorage< QVector<quint8>* > conversionCache;

    DWORD cmType;  // The colorspace type as defined by littlecms
    icColorSpaceSignature colorSpaceSignature; // The colorspace signature as defined in icm/icc files
};

KoColorSpace::KoColorSpace()
    : d (new Private())
{
}

KoColorSpace::KoColorSpace(const QString &id, const QString &name, KoColorSpaceRegistry * parent, KoMixColorsOp* mixColorsOp, KoConvolutionOp* convolutionOp, DWORD cmType, icColorSpaceSignature colorSpaceSignature )
    : d (new Private())
{
    d->id = id;
    d->name = name;
    d->parent = parent;
    d->mixColorsOp = mixColorsOp;
    d->convolutionOp = convolutionOp;
    d->cmType = cmType;
    d->colorSpaceSignature = colorSpaceSignature;
}

KoColorSpace::~KoColorSpace()
{
    delete d->mixColorsOp;
    delete d->convolutionOp;
    delete d;
}

QString KoColorSpace::id() const {return d->id;}

QString KoColorSpace::name() const {return d->name;}

quint32 KoColorSpace::colorSpaceType() const { return d->cmType; }
icColorSpaceSignature KoColorSpace::colorSpaceSignature() const { return d->colorSpaceSignature; }

QList<KoChannelInfo *> KoColorSpace::channels() const
{
    return d->channels;
}

QBitArray KoColorSpace::channelFlags(bool color, bool alpha, bool substance, bool substrate) const
{
    QBitArray ba( d->channels.size() );
    if ( !color && !alpha && !substance && !substrate ) return ba;

    for ( int i = 0; i < d->channels.size(); ++i ) {
        KoChannelInfo * channel = d->channels.at( i );
        if ( ( color && channel->channelType() == KoChannelInfo::COLOR ) ||
             ( alpha && channel->channelType() == KoChannelInfo::ALPHA ) ||
             ( substrate && channel->channelType() == KoChannelInfo::SUBSTRATE ) ||
             ( substance && channel->channelType() == KoChannelInfo::SUBSTANCE ) )
            ba.setBit( i, true );
    }
    return ba;
}

QBitArray KoColorSpace::setChannelFlagsToPixelOrder(const QBitArray & origChannelFlags) const
{
    if ( origChannelFlags.isEmpty() ) return origChannelFlags;

    QBitArray orderedChannelFlags( origChannelFlags.size() );
    for ( int i = 0; i < origChannelFlags.size(); ++i ) {

        KoChannelInfo * channel = d->channels.at( i );
        orderedChannelFlags.setBit( channel->pos(), origChannelFlags.testBit( i ) );
    }
    return orderedChannelFlags;
}

QBitArray KoColorSpace::setChannelFlagsToColorSpaceOrder( const QBitArray & origChannelFlags ) const
{
    if ( origChannelFlags.isEmpty() ) return origChannelFlags;

    QBitArray orderedChannelFlags( origChannelFlags.size() );
    for ( int i = 0; i < orderedChannelFlags.size(); ++i )
    {
        KoChannelInfo * channel = d->channels.at( i );
        orderedChannelFlags.setBit( i, origChannelFlags.testBit( channel->pos() ) );
    }
    return orderedChannelFlags;
}

void KoColorSpace::addChannel(KoChannelInfo * ci)
{
    d->channels.push_back(ci);
}

quint8 *KoColorSpace::allocPixelBuffer(quint32 numPixels) const
{
    return new quint8[pixelSize()*numPixels];
}

QList<KoCompositeOp*> KoColorSpace::userVisiblecompositeOps() const
{
    return d->compositeOps.values();
}


KoMixColorsOp* KoColorSpace::mixColorsOp() const {
    return d->mixColorsOp;
}


KoConvolutionOp* KoColorSpace::convolutionOp() const {
    return d->convolutionOp;
}

const KoCompositeOp * KoColorSpace::compositeOp(const QString & id) const
{
    if ( d->compositeOps.contains( id ) )
        return d->compositeOps.value( id );
    else
        return d->compositeOps.value( COMPOSITE_OVER );
}

void KoColorSpace::addCompositeOp(const KoCompositeOp * op)
{
    if ( op->colorSpace()->id() == id()) {
        d->compositeOps.insert( op->id(), const_cast<KoCompositeOp*>( op ) );
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
    quint8 *pixels = new quint8[sizeof(quint16)*4*numPixels];
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
    if ( d->compositeOps.contains( op ) ) {
        bitBlt(dst, dststride, srcSpace, src, srcRowStride, srcAlphaMask, maskRowStride, opacity, rows, cols, d->compositeOps.value( op ), channelFlags);
    }
    else {
        bitBlt(dst, dststride, srcSpace, src, srcRowStride, srcAlphaMask, maskRowStride, opacity, rows, cols, d->compositeOps.value( COMPOSITE_OVER ), channelFlags);
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
    if ( d->compositeOps.contains( op ) ) {
        bitBlt(dst, dststride, srcSpace, src, srcRowStride, srcAlphaMask, maskRowStride, opacity, rows, cols, d->compositeOps.value( op ));
    }
    else {
        bitBlt(dst, dststride, srcSpace, src, srcRowStride, srcAlphaMask, maskRowStride, opacity, rows, cols, d->compositeOps.value( COMPOSITE_OVER ) );
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

        QVector<quint8> * conversionCache = threadLocalConversionCache(len);
        quint8* conversionData = conversionCache->data();

        for (qint32 row = 0; row < rows; row++) {
            srcSpace->convertPixelsTo(src + row * srcRowStride,
                                      conversionData + row * cols * pixelSize(), this,
                                      cols);
        }

        // The old srcRowStride is no longer valid because we converted to the current cs
        srcRowStride = cols * pixelSize();

        op->composite( dst, dststride,
                       conversionData, srcRowStride,
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

        QVector<quint8> * conversionCache = threadLocalConversionCache(len);
        quint8* conversionData = conversionCache->data();

        for (qint32 row = 0; row < rows; row++) {
            srcSpace->convertPixelsTo(src + row * srcRowStride,
                                      conversionData + row * cols * pixelSize(), this,
                                      cols);
        }

        // The old srcRowStride is no longer valid because we converted to the current cs
        srcRowStride = cols * pixelSize();

        op->composite( dst, dststride,
                       conversionData, srcRowStride,
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

QVector<quint8> * KoColorSpace::threadLocalConversionCache(quint32 size) const
{
    QVector<quint8> * ba = 0;
    if ( !d->conversionCache.hasLocalData() ) {
        ba = new QVector<quint8>( size, '0' );
        d->conversionCache.setLocalData( ba );
    }
    else {
        ba = d->conversionCache.localData();
        if ( ( quint8 )ba->size() < size )
            ba->resize( size );
    }
    return ba;
}
