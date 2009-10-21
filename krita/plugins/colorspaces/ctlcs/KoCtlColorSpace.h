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

#ifndef _KO_CTL_COLOR_SPACE_H_
#define _KO_CTL_COLOR_SPACE_H_

#include <KoColorSpace.h>

class KoCtlColorProfile;
class KoCtlColorSpaceInfo;

class KoCtlColorSpace : public KoColorSpace
{
public:
    /**
     * This class is use when creating color space that are defined using the Color Transformation Language.
     */
    KoCtlColorSpace(const KoCtlColorSpaceInfo*, const KoCtlColorProfile* profile);
    ~KoCtlColorSpace();
    virtual KoColorSpace* clone() const;
    virtual quint32 channelCount() const;
    virtual quint32 colorChannelCount() const;
    virtual quint32 pixelSize() const;
    virtual QString channelValueText(const quint8 *pixel, quint32 channelIndex) const;
    virtual QString normalisedChannelValueText(const quint8 *pixel, quint32 channelIndex) const;
    virtual void normalisedChannelsValue(const quint8 *pixel, QVector<float> &channels) const;
    virtual void fromNormalisedChannelsValue(quint8 *pixel, const QVector<float> &values) const;
    virtual quint8 scaleToU8(const quint8 * srcPixel, qint32 channelPos) const;
    virtual quint16 scaleToU16(const quint8 * srcPixel, qint32 channelPos) const;
    virtual void singleChannelPixel(quint8 *dstPixel, const quint8 *srcPixel, quint32 channelIndex) const;
    virtual bool profileIsCompatible(const KoColorProfile* profile) const;
    static bool profileIsCompatible(const KoCtlColorSpaceInfo*, const KoColorProfile* profile);
    virtual bool hasHighDynamicRange() const;
    virtual const KoColorProfile * profile() const;
    virtual KoColorProfile * profile();
    virtual KoColorTransformation *createBrightnessContrastAdjustment(const quint16 *transferValues) const;
    virtual KoColorTransformation *createDesaturateAdjustment() const;
    virtual KoColorTransformation *createPerChannelAdjustment(const quint16 * const* transferValues) const;
    virtual KoColorTransformation *createDarkenAdjustment(qint32 shade, bool compensate, qreal compensation) const;
    virtual KoColorTransformation *createInvertTransformation() const;
    virtual quint8 difference(const quint8* src1, const quint8* src2) const;
    virtual void fromQColor(const QColor& color, quint8 *dst, const KoColorProfile * profile) const;
    virtual void toQColor(const quint8 *src, QColor *c, const KoColorProfile * profile) const;
    virtual quint8 intensity8(const quint8 * src) const;
    virtual KoID mathToolboxId() const;
    virtual void colorToXML(const quint8* pixel, QDomDocument& doc, QDomElement& colorElt) const;
    virtual void colorFromXML(quint8* pixel, const QDomElement& elt) const;
    virtual KoID colorModelId() const;
    virtual KoID colorDepthId() const;
    virtual quint8 alpha(const quint8 * pixel) const;
    virtual void setAlpha(quint8 * pixels, quint8 alpha, qint32 nPixels) const;
    virtual void multiplyAlpha(quint8 * pixels, quint8 alpha, qint32 nPixels) const;
    virtual void applyAlphaU8Mask(quint8 * pixels, const quint8 * alpha, qint32 nPixels) const;
    virtual void applyInverseAlphaU8Mask(quint8 * pixels, const quint8 * alpha, qint32 nPixels) const;
    virtual bool willDegrade(ColorSpaceIndependence independence) const;
    int alphaPos() const;
private:
    struct Private;
    Private* const d;
};

#endif
