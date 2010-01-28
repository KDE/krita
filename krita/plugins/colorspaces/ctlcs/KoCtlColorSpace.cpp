/*
 *  Copyright (c) 2008-2009 Cyrille Berger <cberger@cberger.net>
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

#include "KoCtlColorSpace.h"

#include <QDomElement>

#include "KoColorSpaceAbstract.h"
#include "KoColorSpaceRegistry.h"
#include "KoColorSpaceMaths.h"
#include "KoCtlColorProfile.h"
#include "KoCtlColorSpaceInfo.h"
#include "KoCtlChannel.h"

#include <kis_debug.h>

#include "KoCtlTemplatesRegistry.h"
#include "KoCtlCompositeOp.h"
#include "KoCtlMixColorsOp.h"
#include "KoCtlConvolutionOp.h"
#include <boost/graph/graph_concepts.hpp>

struct KoCtlColorSpace::Private {
    Private() : alphaCtlChannel(0) {}
    KoCtlColorProfile* profile;
    const KoCtlColorSpaceInfo* info;
    mutable quint16 * qcolordata; // A small buffer for conversion from and to qcolor.
    QList<KoCtlChannel*> ctlChannels;
    KoCtlChannel* alphaCtlChannel;
};

KoCtlColorSpace::KoCtlColorSpace(const KoCtlColorSpaceInfo* info, const KoCtlColorProfile* profile) : KoColorSpace(info->colorSpaceId(), info->name(), new KoCtlMixColorsOp(this, info), new KoCtlConvolutionOp(this, info)), d(new Private)
{
    Q_ASSERT(profile);
    d->info = info;
    d->profile = static_cast<KoCtlColorProfile*>(profile->clone());
    d->qcolordata = new quint16[4];
    this->addCompositeOp(new KoCompositeOpCopy(this));
    for (int i = 0; i < info->channels().size(); ++i) {
        d->ctlChannels.push_back(0);
    }
    foreach(const KoCtlColorSpaceInfo::ChannelInfo* cinfo, info->channels()) {
        addChannel(new KoChannelInfo(cinfo->name(), cinfo->index(), cinfo->channelType(), cinfo->valueType(), cinfo->size(), cinfo->color()));
        KoCtlChannel* ctlchannel = 0;

        switch (cinfo->valueType()) {
        case KoChannelInfo::UINT8:
            ctlchannel = new KoCtlChannelImpl<quint8>(cinfo->position(), info->pixelSize());
            break;
        case KoChannelInfo::UINT16:
            ctlchannel = new KoCtlChannelImpl<quint16>(cinfo->position(), info->pixelSize());
            break;
            /*            case KoChannelInfo::UINT32:
                            ctlchannel = new KoCtlChannelImpl<quint32>( cinfo->position(), info->pixelSize() );
                            break;
                        case KoChannelInfo::INT8:
                            ctlchannel = new KoCtlChannelImpl<qint8>( cinfo->position(), info->pixelSize() );
                            break;
                        case KoChannelInfo::INT16:
                            ctlchannel = new KoCtlChannelImpl<qint16>( cinfo->position(), info->pixelSize() );
                            break;*/
        case KoChannelInfo::FLOAT16:
            ctlchannel = new KoCtlChannelImpl<half>(cinfo->position(), info->pixelSize());
            break;
        case KoChannelInfo::FLOAT32:
            ctlchannel = new KoCtlChannelImpl<float>(cinfo->position(), info->pixelSize());
            break;
            /*            case KoChannelInfo::FLOAT64:
                            ctlchannel = new KoCtlChannelImpl<double>( cinfo->position(), info->pixelSize() );
                            break;*/
        default:
            qFatal("Unimplemented");
        }

        Q_ASSERT(ctlchannel);
        d->ctlChannels[ cinfo->index()] = ctlchannel;
        if (cinfo->channelType() == KoChannelInfo::ALPHA) {
            Q_ASSERT(d->alphaCtlChannel == 0);
            d->alphaCtlChannel = ctlchannel;
        }
    }
    Q_ASSERT(d->info->alphaPos() == -1 || d->alphaCtlChannel != 0);
    foreach(OpenCTL::Template* templat,  KoCtlTemplatesRegistry::instance()->compositeOps()) {
        KoCTLCompositeOp* cop = new KoCTLCompositeOp(templat, this, d->info->pixelDescription());
        if (cop->isValid()) {
            addCompositeOp(cop);
        }
    }
}

KoCtlColorSpace::~KoCtlColorSpace()
{
    foreach(KoCtlChannel* ctlChannel, d->ctlChannels) {
        delete ctlChannel;
    }
    delete d;
}

KoColorSpace* KoCtlColorSpace::clone() const
{
    return new KoCtlColorSpace(d->info, d->profile);
}

quint32 KoCtlColorSpace::channelCount() const
{
    return d->info->channels().size();
}

quint32 KoCtlColorSpace::colorChannelCount() const
{
    return d->info->colorChannelCount();
}

quint32 KoCtlColorSpace::pixelSize() const
{
    return d->info->pixelSize();
}

QString KoCtlColorSpace::channelValueText(const quint8 *pixel, quint32 channelIndex) const
{
    return d->ctlChannels[channelIndex]->channelValueText(pixel);
}

QString KoCtlColorSpace::normalisedChannelValueText(const quint8 *pixel, quint32 channelIndex) const
{
    return d->ctlChannels[channelIndex]->normalisedChannelValueText(pixel);
}

void KoCtlColorSpace::normalisedChannelsValue(const quint8 *pixel, QVector<float> &channels) const
{
    Q_UNUSED(pixel);
    Q_UNUSED(channels);
}

void KoCtlColorSpace::fromNormalisedChannelsValue(quint8 *pixel, const QVector<float> &values) const
{
    for (int i = 0; i < d->ctlChannels.size(); ++i) {
        d->ctlChannels[i]->scaleFromF32(pixel, values[i]);
    }
}
quint8 KoCtlColorSpace::scaleToU8(const quint8 * srcPixel, qint32 channelIndex) const
{
    return d->ctlChannels[channelIndex]->scaleToU8(srcPixel);
}

quint16 KoCtlColorSpace::scaleToU16(const quint8 * srcPixel, qint32 channelIndex) const
{
    return d->ctlChannels[channelIndex]->scaleToU16(srcPixel);
}

void KoCtlColorSpace::singleChannelPixel(quint8 *dstPixel, const quint8 *srcPixel, quint32 channelIndex) const
{
    return d->ctlChannels[channelIndex]->singleChannelPixel(dstPixel, srcPixel);
}

bool KoCtlColorSpace::profileIsCompatible(const KoColorProfile* profile) const
{
    return profileIsCompatible(d->info, profile);
}

bool KoCtlColorSpace::profileIsCompatible(const KoCtlColorSpaceInfo* info, const KoColorProfile* profile)
{
    const KoCtlColorProfile* ctlp = dynamic_cast<const KoCtlColorProfile*>(profile);
    if (!ctlp) return false;
    dbgPlugins << ctlp->colorModel() << ctlp->colorDepth() << info->colorModelId() << info->colorDepthId();
    if (ctlp && ctlp->colorModel() == info->colorModelId().id()
            && (ctlp->colorDepth() == info->colorDepthId().id()
                || (ctlp->colorDepth() == "F" && (info->colorDepthId().id() == "F16" || info->colorDepthId().id() == "F32")))) {
        return true;
    }
    return false;
}


bool KoCtlColorSpace::hasHighDynamicRange() const
{
    return false;
}

const KoColorProfile * KoCtlColorSpace::profile() const
{
    return d->profile;
}

KoColorProfile * KoCtlColorSpace::profile()
{
    return d->profile;
}

KoColorTransformation *KoCtlColorSpace::createBrightnessContrastAdjustment(const quint16 *transferValues) const
{
    Q_UNUSED(transferValues);
    return 0;
}

KoColorTransformation *KoCtlColorSpace::createDesaturateAdjustment() const
{
    return 0;
}

KoColorTransformation *KoCtlColorSpace::createPerChannelAdjustment(const quint16 * const* transferValues) const
{
    Q_UNUSED(transferValues);
    return 0;
}

KoColorTransformation *KoCtlColorSpace::createDarkenAdjustment(qint32 shade, bool compensate, qreal compensation) const
{
    Q_UNUSED(shade);
    Q_UNUSED(compensate);
    Q_UNUSED(compensation);
    return 0;
}

KoColorTransformation *KoCtlColorSpace::createInvertTransformation() const
{
    return 0;
}

quint8 KoCtlColorSpace::difference(const quint8* src1, const quint8* src2) const
{
    Q_UNUSED(src1);
    Q_UNUSED(src2);
    return 0;
}

void KoCtlColorSpace::fromQColor(const QColor& color, quint8 *dst, const KoColorProfile * profile) const
{
    Q_UNUSED(profile);
    d->qcolordata[3] = 0xFFFF;
    d->qcolordata[2] = KoColorSpaceMaths<quint8, quint16>::scaleToA(color.red());
    d->qcolordata[1] = KoColorSpaceMaths<quint8, quint16>::scaleToA(color.green());
    d->qcolordata[0] = KoColorSpaceMaths<quint8, quint16>::scaleToA(color.blue());
    this->fromRgbA16((const quint8*)d->qcolordata, dst, 1);
    this->setAlpha(dst, color.alpha(), 1);
}

void KoCtlColorSpace::toQColor(const quint8 *src, QColor *c, const KoColorProfile * profile) const
{
    Q_UNUSED(profile);
    this->toRgbA16(src, (quint8*)d->qcolordata, 1);
    c->setRgb(
        KoColorSpaceMaths<quint16, quint8>::scaleToA(d->qcolordata[2]),
        KoColorSpaceMaths<quint16, quint8>::scaleToA(d->qcolordata[1]),
        KoColorSpaceMaths<quint16, quint8>::scaleToA(d->qcolordata[0]));
    c->setAlpha(this->alpha(src));
}

quint8 KoCtlColorSpace::intensity8(const quint8 * src) const
{
    Q_UNUSED(src);
    return 0;
}

KoID KoCtlColorSpace::mathToolboxId() const
{
    return KoID("Default", "");
}

void KoCtlColorSpace::colorToXML(const quint8* pixel, QDomDocument& doc, QDomElement& colorElt) const
{
    QString nameOfModel;

    // TODO should be part of the CTLCS definition
    if (d->info->colorModelId().id() == "RGBA") {
        nameOfModel = "RGB";
    }
    if (d->info->colorModelId().id() == "XYZA") {
        nameOfModel = "XYZ";
    }
    if (d->info->colorModelId().id() == "YCbCrA") {
        nameOfModel = "YCbCr";
    }
    QDomElement labElt = doc.createElement(nameOfModel);
    for (int i = 0; i < d->ctlChannels.size(); ++i) {
        const KoCtlChannel* channel = d->ctlChannels[i];
        const KoCtlColorSpaceInfo::ChannelInfo* channelInfo = d->info->channels()[i];
        if (channelInfo->channelType() == KoChannelInfo::COLOR) {
            labElt.setAttribute(channelInfo->shortName(), channel->scaleToF32(pixel));
        }
    }
    labElt.setAttribute("space", profile()->name());
    colorElt.appendChild(labElt);

}
void KoCtlColorSpace::colorFromXML(quint8* pixel, const QDomElement& elt) const
{
    for (int i = 0; i < d->ctlChannels.size(); ++i) {
        const KoCtlChannel* channel = d->ctlChannels[i];
        const KoCtlColorSpaceInfo::ChannelInfo* channelInfo = d->info->channels()[i];
        if (channelInfo->channelType() == KoChannelInfo::COLOR) {
            channel->scaleFromF32(pixel, elt.attribute(channelInfo->shortName()).toDouble());
        }
    }
    setAlpha(pixel, OPACITY_OPAQUE, 1);
}

KoID KoCtlColorSpace::colorModelId() const
{
    return d->info->colorModelId();
}
KoID KoCtlColorSpace::colorDepthId() const
{
    return d->info->colorDepthId();
}

quint8 KoCtlColorSpace::alpha(const quint8 * pixel) const
{
    if (d->alphaCtlChannel) {
        return d->alphaCtlChannel->scaleToU8(pixel);
    } else {
        return 0;
    }
}

qreal KoCtlColorSpace::alpha2(const quint8 * pixel) const
{
    if (d->alphaCtlChannel) {
        return d->alphaCtlChannel->scaleToF32(pixel);
    } else {
        return 0;
    }
}

void KoCtlColorSpace::setAlpha(quint8 * pixels, quint8 alpha, qint32 nPixels) const
{
    if (!d->alphaCtlChannel) return;
    quint32 pixelSize_ = pixelSize();
    for (int i = 0; i < nPixels; ++i, pixels += pixelSize_) {
        d->alphaCtlChannel->scaleFromU8(pixels, alpha);
    }
}

void KoCtlColorSpace::setAlpha2(quint8 * pixels, qreal alpha, qint32 nPixels) const
{
    if (!d->alphaCtlChannel) return;
    quint32 pixelSize_ = pixelSize();
    for (int i = 0; i < nPixels; ++i, pixels += pixelSize_) {
        d->alphaCtlChannel->scaleFromF32(pixels, alpha);
    }
}

void KoCtlColorSpace::multiplyAlpha(quint8 * pixels, quint8 alpha, qint32 nPixels) const
{
    if (!d->alphaCtlChannel) return;
    d->alphaCtlChannel->multiplyU8(pixels, alpha, nPixels);
}

void KoCtlColorSpace::applyAlphaU8Mask(quint8 * pixels, const quint8 * alpha, qint32 nPixels) const
{
    if (!d->alphaCtlChannel) return;
    d->alphaCtlChannel->applyU8Mask(pixels, alpha, nPixels);
}

void KoCtlColorSpace::applyInverseAlphaU8Mask(quint8 * pixels, const quint8 * alpha, qint32 nPixels) const
{
    if (!d->alphaCtlChannel) return;
    d->alphaCtlChannel->applyInverseU8Mask(pixels, alpha, nPixels);
}

bool KoCtlColorSpace::willDegrade(ColorSpaceIndependence /*independence*/) const
{
    // Currently all levels of colorspace independence will degrade floating point
    // colorspaces images.
    return true;
}

int KoCtlColorSpace::alphaPos() const
{
    return d->info->alphaPos();
}
