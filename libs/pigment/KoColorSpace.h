/*
 *  Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2006-2007 Cyrille Berger <cberger@cberger.net>
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
#ifndef KOCOLORSPACE_H
#define KOCOLORSPACE_H

#include <limits.h>

#include <QtGui/QImage>
#include <QtCore/QHash>
#include <QtCore/QVector>
#include <QtCore/QList>
#include <QtCore/QBitArray>

#include "KoColorSpaceConstants.h"
#include "KoColorConversionTransformation.h"
#include <KoChannelInfo.h>
#include <KoID.h>
#include "pigment_export.h"

class QDomDocument;
class QDomElement;
class KoCompositeOp;
class KoColorProfile;
class KoColorTransformation;
class KoColorConversionTransformationFactory;
class QBitArray;

enum Deletability {
    OwnedByRegistryDoNotDelete,
    OwnedByRegistryRegistyDeletes,
    NotOwnedByRegistry
};

enum ColorSpaceIndependence {
    FULLY_INDEPENDENT,
    TO_LAB16,
    TO_RGBA8,
    TO_RGBA16
};


#include "KoMixColorsOp.h"
#include "KoConvolutionOp.h"

/**
 * A KoColorSpace is the definition of a certain color space.
 *
 * A color model and a color space are two related concepts. A color
 * model is more general in that it describes the channels involved and
 * how they in broad terms combine to describe a color. Examples are
 * RGB, HSV, CMYK.
 *
 * A color space is more specific in that it also describes exactly how
 * the channels are combined. So for each color model there can be a
 * number of specific color spaces. So RGB is the model and sRGB,
 * adobeRGB, etc are colorspaces.
 *
 * In Pigment KoColorSpace acts as both a color model and a color space.
 * You can think of the class definition as the color model, but the
 * instance of the class as representing a colorspace.
 *
 * A third concept is the profile represented by KoColorProfile. It
 * represents the info needed to specialize a color model into a color
 * space.
 *
 * KoColorSpace is an abstract class serving as an interface.
 *
 * Subclasses implement actual color spaces
 * Some subclasses implement only some parts and are named Traits
 *
 */
class PIGMENTCMS_EXPORT KoColorSpace
{
    friend class KoColorSpaceRegistry;
    friend class KoColorSpaceFactory;
protected:
    /// Only for use by classes that serve as baseclass for real color spaces
    KoColorSpace();

public:

    /// Should be called by real color spaces
    KoColorSpace(const QString &id, const QString &name, KoMixColorsOp* mixColorsOp, KoConvolutionOp* convolutionOp);

    virtual bool operator==(const KoColorSpace& rhs) const;
protected:
    virtual ~KoColorSpace();

public:
    //========== Channels =====================================================//

    /// Return a list describing all the channels this color model has.
    virtual QList<KoChannelInfo *> channels() const;

    /**
     * The total number of channels for a single pixel in this color model
     */
    virtual quint32 channelCount() const = 0;

    /**
     * The total number of color channels (excludes alpha and substance) for a single
     * pixel in this color model.
     */
    virtual quint32 colorChannelCount() const = 0;

    /**
     * returns a QBitArray that contains true for the specified
     * channel types:
     *
     * @param color if true, set all color channels to true
     * @param alpha if true, set all alpha channels to true
     * @param substance if true, set all substance channels to true
     * @param substrate if true, set all substrate channels to true
     *
     * The order of channels is the colorspace descriptive order,
     * not the pixel order.
     */
    QBitArray channelFlags(bool color = true, bool alpha = false, bool substance = false, bool substrate = false) const;

    /**
     * The size in bytes of a single pixel in this color model
     */
    virtual quint32 pixelSize() const = 0;

    /**
     * Return a string with the channel's value suitable for display in the gui.
     */
    virtual QString channelValueText(const quint8 *pixel, quint32 channelIndex) const = 0;

    /**
     * Return a string with the channel's value with integer
     * channels normalised to the floating point range 0 to 1, if
     * appropriate.
     */
    virtual QString normalisedChannelValueText(const quint8 *pixel, quint32 channelIndex) const = 0;

    /**
     * Return a QVector of floats with channels' values normalized
     * to floating point range 0 to 1.
     */
    virtual void normalisedChannelsValue(const quint8 *pixel, QVector<float> &channels) const = 0;

    /**
     * Write in the pixel the value from the normalized vector.
     */
    virtual void fromNormalisedChannelsValue(quint8 *pixel, const QVector<float> &values) const = 0;

    /**
     * Convert the value of the channel at the specified position into
     * an 8-bit value. The position is not the number of bytes, but
     * the position of the channel as defined in the channel info list.
     */
    virtual quint8 scaleToU8(const quint8 * srcPixel, qint32 channelPos) const = 0;

    /**
     * Convert the value of the channel at the specified position into
     * a 16-bit value. This may be upscaling or downscaling, depending
     * on the defined value of the channel
     */
    virtual quint16 scaleToU16(const quint8 * srcPixel, qint32 channelPos) const = 0;

    /**
     * Set dstPixel to the pixel containing only the given channel of srcPixel. The remaining channels
     * should be set to whatever makes sense for 'empty' channels of this color space,
     * with the intent being that the pixel should look like it only has the given channel.
     */
    virtual void singleChannelPixel(quint8 *dstPixel, const quint8 *srcPixel, quint32 channelIndex) const = 0;

    //========== Identification ===============================================//

    /**
     * ID for use in files and internally: unchanging name. As the id must be unique
     * it is usually the concatenation of the id of the color model and of the color
     * depth, for instance "RGBA8" or "CMYKA16" or "XYZA32f".
     */
    virtual QString id() const;

    /**
     * User visible name which contains the name of the color model and of the color depth.
     * For intance "RGBA (8-bits)" or "CMYKA (16-bits)".
     */
    virtual QString name() const;

    /**
     * @return a string that identify the color model (for instance "RGB" or "CMYK" ...)
     * @see KoColorModelStandardIds.h
     */
    virtual KoID colorModelId() const = 0;
    /**
     * @return a string that identify the bit depth (for instance "U8" or "F16" ...)
     * @see KoColorModelStandardIds.h
     */
    virtual KoID colorDepthId() const = 0;

    /**
     * @return true if the profile given in argument can be used by this color space
     */
    virtual bool profileIsCompatible(const KoColorProfile* profile) const = 0;

    /**
     * If false, images in this colorspace will degrade considerably by
     * functions, tools and filters that have the given measure of colorspace
     * independence.
     *
     * @param independence the measure to which this colorspace will suffer
     *                     from the manipulations of the tool or filter asking
     * @return false if no degradation will take place, true if degradation will
     *         take place
     */
    virtual bool willDegrade(ColorSpaceIndependence independence) const = 0;

    //========== Capabilities =================================================//

    /**
     * Returns the list of user-visible composite ops supported by this colorspace.
     */
    virtual QList<KoCompositeOp*> compositeOps() const;

    /**
     * Retrieve a single composite op from the ones this colorspace offers.
     * If the requeste composite op does not exist, COMPOSITE_OVER is returned.
     */
    virtual const KoCompositeOp * compositeOp(const QString & id) const;

    /**
     * add a composite op to this colorspace.
     */
    virtual void addCompositeOp(const KoCompositeOp * op);

    /**
     * Returns true if the colorspace supports channel values outside the
     * (normalised) range 0 to 1.
     */
    virtual bool hasHighDynamicRange() const = 0;


    //========== Display profiles =============================================//

    /**
     * Return the profile of this color space.
     */
    virtual const KoColorProfile * profile() const = 0;
    /**
     * Return the profile of this color space.
     */
    virtual KoColorProfile * profile() = 0;

//================= Conversion functions ==================================//


    /**
     * The fromQColor methods take a given color defined as an RGB QColor
     * and fills a byte array with the corresponding color in the
     * the colorspace managed by this strategy.
     *
     * @param color the QColor that will be used to fill dst
     * @param dst a pointer to a pixel
     * @param profile the optional profile that describes the color values of QColor
     */
    virtual void fromQColor(const QColor& color, quint8 *dst, const KoColorProfile * profile = 0) const = 0;

    /**
     * The toQColor methods take a byte array that is at least pixelSize() long
     * and converts the contents to a QColor, using the given profile as a source
     * profile and the optional profile as a destination profile.
     *
     * @param src a pointer to the source pixel
     * @param c the QColor that will be filled with the color at src
     * @param profile the optional profile that describes the color in c, for instance the monitor profile
     */
    virtual void toQColor(const quint8 *src, QColor *c, const KoColorProfile * profile = 0) const = 0;

    /**
     * Convert the pixels in data to (8-bit BGRA) QImage using the specified profiles.
     *
     * @param data A pointer to a contiguous memory region containing width * height pixels
     * @param width in pixels
     * @param height in pixels
     * @param dstProfile destination profile
     * @param renderingIntent the rendering intent
     */
    virtual QImage convertToQImage(const quint8 *data, qint32 width, qint32 height,
                                   const KoColorProfile *  dstProfile, KoColorConversionTransformation::Intent renderingIntent = KoColorConversionTransformation::IntentPerceptual) const ;

    /**
     * This functions allocates the ncessary memory for numPixels number of pixels.
     * It is your responsibility to delete[] it.
     */
    quint8 *allocPixelBuffer(quint32 numPixels, bool clear = false, quint8 defaultvalue = 0) const;

    /**
     * Convert the specified data to Lab (D50). All colorspaces are guaranteed to support this
     *
     * @param src the source data
     * @param dst the destination data
     * @param nPixels the number of source pixels
     */
    virtual void toLabA16(const quint8 * src, quint8 * dst, quint32 nPixels) const;

    /**
     * Convert the specified data from Lab (D50). to this colorspace. All colorspaces are
     * guaranteed to support this.
     *
     * @param src the pixels in 16 bit lab format
     * @param dst the destination data
     * @param nPixels the number of pixels in the array
     */
    virtual void fromLabA16(const quint8 * src, quint8 * dst, quint32 nPixels) const;

    /**
     * Convert the specified data to sRGB 16 bits. All colorspaces are guaranteed to support this
     *
     * @param src the source data
     * @param dst the destination data
     * @param nPixels the number of source pixels
     */
    virtual void toRgbA16(const quint8 * src, quint8 * dst, quint32 nPixels) const;

    /**
     * Convert the specified data from sRGB 16 bits. to this colorspace. All colorspaces are
     * guaranteed to support this.
     *
     * @param src the pixels in 16 bit rgb format
     * @param dst the destination data
     * @param nPixels the number of pixels in the array
     */
    virtual void fromRgbA16(const quint8 * src, quint8 * dst, quint32 nPixels) const;

    /**
     * Create a color conversion transformation.
     */
    virtual KoColorConversionTransformation* createColorConverter(const KoColorSpace * dstColorSpace, KoColorConversionTransformation::Intent renderingIntent = KoColorConversionTransformation::IntentPerceptual) const;

    /**
     * Convert a byte array of srcLen pixels *src to the specified color space
     * and put the converted bytes into the prepared byte array *dst.
     *
     * Returns false if the conversion failed, true if it succeeded
     *
     * This function is not thread-safe. If you want to apply multiple conversion
     * in different threads at the same time, you need to create one color converter
     * per-thread using createColorConverter.
     */
    virtual bool convertPixelsTo(const quint8 * src,
                                 quint8 * dst, const KoColorSpace * dstColorSpace,
                                 quint32 numPixels,
                                 KoColorConversionTransformation::Intent renderingIntent = KoColorConversionTransformation::IntentPerceptual) const;

//============================== Manipulation functions ==========================//


//
// The manipulation functions have default implementations that _convert_ the pixel
// to a QColor and back. Reimplement these methods in your color strategy!
//

    /**
     * Get the alpha value of the given pixel, downscaled to an 8-bit value.
     */
    virtual quint8 opacityU8(const quint8 * pixel) const = 0;
    virtual qreal opacityF(const quint8 * pixel) const = 0;

    /**
     * Set the alpha channel of the given run of pixels to the given value.
     *
     * pixels -- a pointer to the pixels that will have their alpha set to this value
     * alpha --  a downscaled 8-bit value for opacity
     * nPixels -- the number of pixels
     *
     */
    virtual void setOpacity(quint8 * pixels, quint8 alpha, qint32 nPixels) const = 0;
    virtual void setOpacity(quint8 * pixels, qreal alpha, qint32 nPixels) const = 0;

    /**
     * Multiply the alpha channel of the given run of pixels by the given value.
     *
     * pixels -- a pointer to the pixels that will have their alpha set to this value
     * alpha --  a downscaled 8-bit value for opacity
     * nPixels -- the number of pixels
     *
     */
    virtual void multiplyAlpha(quint8 * pixels, quint8 alpha, qint32 nPixels) const = 0;

    /**
     * Applies the specified 8-bit alpha mask to the pixels. We assume that there are just
     * as many alpha values as pixels but we do not check this; the alpha values
     * are assumed to be 8-bits.
     */
    virtual void applyAlphaU8Mask(quint8 * pixels, const quint8 * alpha, qint32 nPixels) const = 0;

    /**
     * Applies the inverted 8-bit alpha mask to the pixels. We assume that there are just
     * as many alpha values as pixels but we do not check this; the alpha values
     * are assumed to be 8-bits.
     */
    virtual void applyInverseAlphaU8Mask(quint8 * pixels, const quint8 * alpha, qint32 nPixels) const = 0;

    /**
     * Create an adjustment object for adjusting the brightness and contrast
     * transferValues is a 256 bins array with values from 0 to 0xFFFF
     * This function is thread-safe, but you need to create one KoColorTransformation per thread.
     */
    virtual KoColorTransformation *createBrightnessContrastAdjustment(const quint16 *transferValues) const = 0;

    /**
     * Create an adjustment object for desaturating
     * This function is thread-safe, but you need to create one KoColorTransformation per thread.
     */
    virtual KoColorTransformation *createDesaturateAdjustment() const = 0;

    /**
     * Create an adjustment object for adjusting individual channels
     * transferValues is an array of colorChannelCount number of 256 bins array with values from 0 to 0xFFFF
     * This function is thread-safe, but you need to create one KoColorTransformation per thread.
     */
    virtual KoColorTransformation *createPerChannelAdjustment(const quint16 * const* transferValues) const = 0;

    /**
     * Darken all color channels with the given amount. If compensate is true,
     * the compensation factor will be used to limit the darkening.
     *
     */
    virtual KoColorTransformation *createDarkenAdjustment(qint32 shade, bool compensate, qreal compensation) const = 0;

    /**
     * Invert color channels of the given pixels
     * This function is thread-safe, but you need to create one KoColorTransformation per thread.
     */
    virtual KoColorTransformation *createInvertTransformation() const = 0;

    /**
     * Get the difference between 2 colors, normalized in the range (0,255). Only completely
     * opaque and completely transparent are taken into account when computing the different;
     * other transparency levels are not regarded when finding the difference.
     */
    virtual quint8 difference(const quint8* src1, const quint8* src2) const = 0;

    /**
     * @return the mix color operation of this colorspace (do not delete it locally, it's deleted by the colorspace).
     */
    virtual KoMixColorsOp* mixColorsOp() const;

    /**
     * @return the convolution operation of this colorspace (do not delete it locally, it's deleted by the colorspace).
     */
    virtual KoConvolutionOp* convolutionOp() const;

    /**
     * Calculate the intensity of the given pixel, scaled down to the range 0-255. XXX: Maybe this should be more flexible
     */
    virtual quint8 intensity8(const quint8 * src) const = 0;

    /**
     * Create a mathematical toolbox compatible with this colorspace
     */
    virtual KoID mathToolboxId() const = 0;

    /**
     * Compose two arrays of pixels together. If source and target
     * are not the same color model, the source pixels will be
     * converted to the target model. We're "dst" -- "dst" pixels are always in _this_
     * colorspace.
     *
     * @param dst pointer to the pixels onto which src will be composited. dst is "below" src.
     * @param dststride the total number of bytes in one line in the dst paint device
     * @param srcSpace the colorspace of the source pixels that will be composited onto "us"
     * @param src pointer to the pixels that will be composited onto "us"
     * @param srcRowStride the total number of bytes in one line in the src paint device
     * @param srcAlphaMask pointer to an alpha mask that determines whether and how much
     *        of src will be composited onto dst
     * @param maskRowStride the total number of bytes in one line in the mask paint device
     * @param rows the number of rows of pixels we'll be compositing
     * @param cols the length in pixels of a single row we'll be compositing.
     * @param op the composition operator to use, e.g. COPY_OVER
     * @param channelFlags a bit array reflecting which channels will be composited and which
     *        channels won't. The order is pixel order, not colorspace order.
     */
    virtual void bitBlt(quint8 *dst,
                        qint32 dststride,
                        const KoColorSpace * srcSpace,
                        const quint8 *src,
                        qint32 srcRowStride,
                        const quint8 *srcAlphaMask,
                        qint32 maskRowStride,
                        quint8 opacity,
                        qint32 rows,
                        qint32 cols,
                        const KoCompositeOp * op,
                        const QBitArray & channelFlags) const;

    /**
     * Convenience function for the above where all channels are turned on.
     */
    virtual void bitBlt(quint8 *dst,
                        qint32 dststride,
                        const KoColorSpace * srcSpace,
                        const quint8 *src,
                        qint32 srcRowStride,
                        const quint8 *srcAlphaMask,
                        qint32 maskRowStride,
                        quint8 opacity,
                        qint32 rows,
                        qint32 cols,
                        const KoCompositeOp * op) const;

    /**
     * Convenience function for the above if you don't have the composite op object yet.
     */
    virtual void bitBlt(quint8 *dst,
                        qint32 dststride,
                        const KoColorSpace * srcSpace,
                        const quint8 *src,
                        qint32 srcRowStride,
                        const quint8 *srcAlphaMask,
                        qint32 maskRowStride,
                        quint8 opacity,
                        qint32 rows,
                        qint32 cols,
                        const QString & op,
                        const QBitArray & channelFlags) const;

    /**
     * Convenience function for the above, if you simply want all channels composited
     */
    virtual void bitBlt(quint8 *dst,
                        qint32 dststride,
                        const KoColorSpace * srcSpace,
                        const quint8 *src,
                        qint32 srcRowStride,
                        const quint8 *srcAlphaMask,
                        qint32 maskRowStride,
                        quint8 opacity,
                        qint32 rows,
                        qint32 cols,
                        const QString& op) const;


    /**
     * Serialize this color following Create's swatch color specification available
     * at http://create.freedesktop.org/wiki/index.php/Swatches_-_colour_file_format
     *
     * This function doesn't create the <color /> element but rather the <CMYK />,
     * <sRGB />, <RGB /> ... elements. It is assumed that colorElt is the <color />
     * element.
     *
     * @param pixel buffer to serialized
     * @param colorElt root element for the serialization, it is assumed that this
     *                 element is <color />
     * @param doc is the document containing colorElt
     */
    virtual void colorToXML(const quint8* pixel, QDomDocument& doc, QDomElement& colorElt) const = 0;

    /**
     * Unserialize a color following Create's swatch color specification available
     * at http://create.freedesktop.org/wiki/index.php/Swatches_-_colour_file_format
     *
     * @param pixel buffer where the color will be unserialized
     * @param elt the element to unserialize (<CMYK />, <sRGB />, <RGB />)
     * @return the unserialize color, or an empty color object if the function failed
     *         to unserialize the color
     */
    virtual void colorFromXML(quint8* pixel, const QDomElement& elt) const = 0;

    KoColorTransformation* createColorTransformation(const QString & id, const QHash<QString, QVariant> & parameters) const;

protected:

    /**
     * Use this function in the constructor of your colorspace to add the information about a channel.
     * @param ci a pointer to the information about a channel
     */
    virtual void addChannel(KoChannelInfo * ci);
    const KoColorConversionTransformation* toLabA16Converter() const;
    const KoColorConversionTransformation* fromLabA16Converter() const;
    const KoColorConversionTransformation* toRgbA16Converter() const;
    const KoColorConversionTransformation* fromRgbA16Converter() const;

protected:

    /**
     * Returns the thread-local conversion cache. If it doesn't exist
     * yet, it is created. If it is currently too small, it is resized.
     */
    QVector<quint8> * threadLocalConversionCache(quint32 size) const;

    struct Private;
    Private * const d;
};

#endif // KOCOLORSPACE_H
