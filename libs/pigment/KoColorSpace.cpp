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

#include "KoColorSpace.h"

#include <QThreadStorage>
#include <QByteArray>
#include <QBitArray>

#include "KoChannelInfo.h"
#include "DebugPigment.h"
#include "KoCompositeOp.h"
#include "KoColorTransformation.h"
#include "KoColorTransformationFactory.h"
#include "KoColorTransformationFactoryRegistry.h"
#include "KoColorConversionCache.h"
#include "KoColorConversionSystem.h"
#include "KoColorSpaceRegistry.h"
#include "KoColorProfile.h"
#include "KoCopyColorConversionTransformation.h"
#include "KoFallBackColorTransformation.h"
#include "KoUniqueNumberForIdServer.h"

#include "KoColorSpace_p.h"

KoColorSpace::KoColorSpace()
    : d (new Private())
{
}

KoColorSpace::KoColorSpace(const QString &id, const QString &name, KoMixColorsOp* mixColorsOp, KoConvolutionOp* convolutionOp )
    : d (new Private())
{
    d->id = id;
    d->idNumber = KoUniqueNumberForIdServer::instance()->numberForId( d->id );
    d->name = name;
    d->mixColorsOp = mixColorsOp;
    d->convolutionOp = convolutionOp;
    d->transfoToRGBA16 = 0;
    d->transfoFromRGBA16 = 0;
    d->transfoToLABA16 = 0;
    d->transfoFromLABA16 = 0;
    d->deletability = NotOwnedByRegistry;
}

KoColorSpace::~KoColorSpace()
{
    Q_ASSERT(d->deletability != OwnedByRegistryDoNotDelete);

    qDeleteAll(d->compositeOps);
    foreach(KoChannelInfo * channel, d->channels)
    {
        delete channel;
    }
    if (d->deletability == NotOwnedByRegistry) {
        KoColorConversionCache* cache = KoColorSpaceRegistry::instance()->colorConversionCache();
        if (cache) {
            cache->colorSpaceIsDestroyed(this);
        }
    }
    delete d->mixColorsOp;
    delete d->convolutionOp;
    delete d;
}

bool KoColorSpace::operator==(const KoColorSpace& rhs) const {
    const KoColorProfile* p1 = rhs.profile();
    const KoColorProfile* p2 = profile();
    return d->idNumber == rhs.d->idNumber && ((p1 == p2) || (*p1 == *p2 ));
}

QString KoColorSpace::id() const {return d->id;}

QString KoColorSpace::name() const {return d->name;}

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

quint8 *KoColorSpace::allocPixelBuffer(quint32 numPixels, bool clear, quint8 defaultvalue) const
{
    if (numPixels == 0) {
        return 0;
    }
    quint8* buf = new quint8[pixelSize()*numPixels];
    if (clear) {
        memset(buf, defaultvalue, pixelSize() * numPixels);
    }
    return buf;
}

QList<KoCompositeOp*> KoColorSpace::compositeOps() const
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
    else {
        warnPigment << "Asking for non-existent composite operation " << id << ", returning " << COMPOSITE_OVER;
        return d->compositeOps.value( COMPOSITE_OVER );
    }
}

void KoColorSpace::addCompositeOp(const KoCompositeOp * op)
{
    if ( op->colorSpace()->id() == id()) {
        d->compositeOps.insert( op->id(), const_cast<KoCompositeOp*>( op ) );
    }
}

const KoColorConversionTransformation* KoColorSpace::toLabA16Converter() const
{
    if(!d->transfoToLABA16)
    {
        d->transfoToLABA16 = KoColorSpaceRegistry::instance()->colorConversionSystem()->createColorConverter(this, KoColorSpaceRegistry::instance()->lab16("") ) ;
    }
    return d->transfoToLABA16;
}

const KoColorConversionTransformation* KoColorSpace::fromLabA16Converter() const
{
    if(!d->transfoFromLABA16)
    {
        d->transfoFromLABA16 = KoColorSpaceRegistry::instance()->colorConversionSystem()->createColorConverter( KoColorSpaceRegistry::instance()->lab16("") , this ) ;
    }
    return d->transfoFromLABA16;
}
const KoColorConversionTransformation* KoColorSpace::toRgbA16Converter() const
{
    if(!d->transfoToRGBA16)
    {
        d->transfoToRGBA16 = KoColorSpaceRegistry::instance()->colorConversionSystem()->createColorConverter( this, KoColorSpaceRegistry::instance()->rgb16("") ) ;
    }
    return d->transfoToRGBA16;
}
const KoColorConversionTransformation* KoColorSpace::fromRgbA16Converter() const
{
    if(!d->transfoFromRGBA16)
    {
        d->transfoFromRGBA16 = KoColorSpaceRegistry::instance()->colorConversionSystem()->createColorConverter( KoColorSpaceRegistry::instance()->rgb16("") , this ) ;
    }
    return d->transfoFromRGBA16;
}

void KoColorSpace::toLabA16(const quint8 * src, quint8 * dst, quint32 nPixels) const
{
    toLabA16Converter()->transform( src, dst, nPixels);
}

void KoColorSpace::fromLabA16(const quint8 * src, quint8 * dst, quint32 nPixels) const
{
    fromLabA16Converter()->transform( src, dst, nPixels);
}

void KoColorSpace::toRgbA16(const quint8 * src, quint8 * dst, quint32 nPixels) const
{
    toRgbA16Converter()->transform( src, dst, nPixels);
}

void KoColorSpace::fromRgbA16(const quint8 * src, quint8 * dst, quint32 nPixels) const
{
    fromRgbA16Converter()->transform( src, dst, nPixels);
}

KoColorConversionTransformation* KoColorSpace::createColorConverter(const KoColorSpace * dstColorSpace, KoColorConversionTransformation::Intent renderingIntent) const
{
    if( *this == *dstColorSpace)
    {
        return new KoCopyColorConversionTransformation(this);
    } else {
        return KoColorSpaceRegistry::instance()->colorConversionSystem()->createColorConverter( this, dstColorSpace, renderingIntent);
    }
}

bool KoColorSpace::convertPixelsTo(const quint8 * src,
                                   quint8 * dst,
                                   const KoColorSpace * dstColorSpace,
                                   quint32 numPixels,
                                   KoColorConversionTransformation::Intent renderingIntent) const
{
    if( *this == *dstColorSpace)
    {
        memcpy( dst, src, numPixels * sizeof(quint8) * pixelSize());
    } else {
        KoCachedColorConversionTransformation cct = KoColorSpaceRegistry::instance()->colorConversionCache()->cachedConverter(this, dstColorSpace, renderingIntent);
        cct.transformation()->transform(src, dst, numPixels);
    }
    return true;
}


void KoColorSpace::bitBlt(quint8 *dst,
                          qint32 dststride,
                          bool dstAlphaLocked,
                          const KoColorSpace * srcSpace,
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
        bitBlt(dst, dststride, dstAlphaLocked, srcSpace, src, srcRowStride, srcAlphaMask, maskRowStride, opacity, rows, cols, d->compositeOps.value( op ), channelFlags);
    }
    else {
        bitBlt(dst, dststride, dstAlphaLocked, srcSpace, src, srcRowStride, srcAlphaMask, maskRowStride, opacity, rows, cols, d->compositeOps.value( COMPOSITE_OVER ), channelFlags);
    }

}

void KoColorSpace::bitBlt(quint8 *dst,
                          qint32 dststride,
                          bool dstAlphaLocked,
                          const KoColorSpace * srcSpace,
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
        bitBlt(dst, dststride, dstAlphaLocked, srcSpace, src, srcRowStride, srcAlphaMask, maskRowStride, opacity, rows, cols, d->compositeOps.value( op ));
    }
    else {
        bitBlt(dst, dststride, dstAlphaLocked, srcSpace, src, srcRowStride, srcAlphaMask, maskRowStride, opacity, rows, cols, d->compositeOps.value( COMPOSITE_OVER ) );
    }
}

void KoColorSpace::bitBlt(quint8 *dst,
                          qint32 dstRowStride,
                          bool dstAlphaLocked,
                          const KoColorSpace * srcSpace,
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
    Q_ASSERT_X(*op->colorSpace() == *this, "KoColorSpace::bitBlt", QString("Composite op is for color space %1 (%2) while this is %3 (%4)" ).arg( op->colorSpace()->id() ).arg(op->colorSpace()->profile()->name()).arg(id()).arg(profile()->name()).toLatin1() );

    if (rows <= 0 || cols <= 0)
        return;

    quint8* alphaBytes = 0;
    if (dstAlphaLocked) {
        alphaBytes = getAlphaBytes(dst, dstRowStride, rows, cols);
    }

    if (!(*this == *srcSpace)) {

        quint32 conversionBufferStride = cols * pixelSize();
        QVector<quint8> * conversionCache = 
                threadLocalConversionCache(rows * conversionBufferStride);

        quint8* conversionData = conversionCache->data();

        for (qint32 row = 0; row < rows; row++) {
            srcSpace->convertPixelsTo(src + row * srcRowStride,
                                      conversionData + row * conversionBufferStride,
                                      this, cols);
        }

        op->composite( dst, dstRowStride,
                       conversionData, conversionBufferStride,
                       srcAlphaMask, maskRowStride,
                       rows,  cols,
                       opacity, channelFlags );
    }
    else {
        op->composite( dst, dstRowStride,
                       src, srcRowStride,
                       srcAlphaMask, maskRowStride,
                       rows,  cols,
                       opacity, channelFlags );
    }

    if (dstAlphaLocked) {
        applyAlphaBytes(dst, alphaBytes, dstRowStride, rows, cols);
        delete[] alphaBytes;
    }

}

// XXX: I don't want this code duplication, but also don't want an
//      extra function call in this critical section of code. What to
//      do?
void KoColorSpace::bitBlt(quint8 *dst,
                          qint32 dstRowStride,
                          bool dstAlphaLocked,
                          const KoColorSpace * srcSpace,
                          const quint8 *src,
                          qint32 srcRowStride,
                          const quint8 *srcAlphaMask,
                          qint32 maskRowStride,
                          quint8 opacity,
                          qint32 rows,
                          qint32 cols,
                          const KoCompositeOp * op) const
{
    Q_ASSERT(*op->colorSpace() == *this);

    if (rows <= 0 || cols <= 0)
        return;

    quint8* alphaBytes = 0;
    if (dstAlphaLocked) {
        alphaBytes = getAlphaBytes(dst, dstRowStride, rows, cols);
    }

    if (this != srcSpace) {
        quint32 conversionBufferStride = cols * pixelSize();
        QVector<quint8> * conversionCache = 
                threadLocalConversionCache(rows * conversionBufferStride);

        quint8* conversionData = conversionCache->data();

        for (qint32 row = 0; row < rows; row++) {
            srcSpace->convertPixelsTo(src + row * srcRowStride,
                                      conversionData + row * conversionBufferStride, this,
                                      cols);
        }

        op->composite( dst, dstRowStride,
                       conversionData, conversionBufferStride,
                       srcAlphaMask, maskRowStride,
                       rows,  cols,
                       opacity);

    }
    else {
        op->composite( dst, dstRowStride,
                       src, srcRowStride,
                       srcAlphaMask, maskRowStride,
                       rows, cols,
                       opacity);
    }

    if (dstAlphaLocked) {
        applyAlphaBytes(dst, alphaBytes, dstRowStride, rows, cols);
        delete[] alphaBytes;
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

KoColorTransformation* KoColorSpace::createColorTransformation( const QString & id, const QHash<QString, QVariant> & parameters) const
{
    KoColorTransformationFactory* factory = KoColorTransformationFactoryRegistry::instance()->get( id );
    if(!factory) return 0;
    QPair<KoID, KoID> model( colorModelId(), colorDepthId() );
    QList< QPair<KoID, KoID> > models = factory->supportedModels();
    if(models.contains(model))
    {
        return factory->createTransformation( this, parameters);
    } else {
        // Find the best solution
        // TODO use the color conversion cache
        KoColorConversionTransformation* csToFallBack = 0;
        KoColorConversionTransformation* fallBackToCs = 0;
        KoColorSpaceRegistry::instance()->colorConversionSystem()->createColorConverters(this, models, csToFallBack, fallBackToCs);
        Q_ASSERT(csToFallBack);
        Q_ASSERT(fallBackToCs);
        KoColorTransformation* transfo = factory->createTransformation(fallBackToCs->srcColorSpace(), parameters);
        return new KoFallBackColorTransformation( csToFallBack, fallBackToCs, transfo);
    }
}

QImage KoColorSpace::convertToQImage(const quint8 *data, qint32 width, qint32 height,
                                     const KoColorProfile *dstProfile,
                                     KoColorConversionTransformation::Intent renderingIntent) const

{
    QImage img = QImage(width, height, QImage::Format_ARGB32);

    const KoColorSpace * dstCS = KoColorSpaceRegistry::instance()->rgb8(dstProfile);

    if (data)
        this->convertPixelsTo(const_cast<quint8 *>(data), img.bits(), dstCS, width * height, renderingIntent);

    return img;
}

quint8* KoColorSpace::getAlphaBytes(quint8* pixels, qint32 rowStride, quint32 rows, quint32 columns) const
{
    quint8* alphaPixels = new quint8[rows * columns];

    quint8 *dst = pixels;
    qint32 bytesPerPixel = pixelSize();

    int i = 0;
    while (rows > 0) {
        quint8* dstN = dst;
        int cols = columns;
        while (cols > 0) {
            alphaPixels[i] = alpha(dstN);
            dst += bytesPerPixel;
            --cols;
            ++i;
        }
        dst += rowStride;
        --rows;
    }

    return alphaPixels;
}

void KoColorSpace::applyAlphaBytes(quint8* pixels, quint8* alpha, qint32 rowStride, quint32 rows, quint32 cols) const
{
    quint8 *dst = pixels;

    int i = 0;
    while (rows > 0) {
        qDebug() << rows << i << rowStride << pixels << alpha;
        setAlpha(dst, alpha[i], cols);
        i += cols;
        dst += rowStride;
        --rows;
    }
}
