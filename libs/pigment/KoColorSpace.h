/*
 *  Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef KOCOLORSPACE_H
#define KOCOLORSPACE_H

#include <config.h>
#include <lcms.h>

#include <QImage>
#include <q3valuevector.h>
#include <q3valuelist.h>

#include <KoCompositeOp.h>
#include <KoChannelInfo.h>

class DCOPObject;

class KoColorProfile;
class KoColorSpaceFactoryRegistry;
class KisMathToolbox;
class KisFilter;

const quint8 OPACITY_TRANSPARENT = 0;
const quint8 OPACITY_OPAQUE = UCHAR_MAX;

class KoColorAdjustment
{
public:

    KoColorAdjustment() {};
    virtual ~KoColorAdjustment() {};
};


enum ColorSpaceIndependence {
    FULLY_INDEPENDENT,
    TO_LAB16,
    TO_RGBA8
};

/**
 * A KoColorSpace is the definition of a certain color model.
 * This is the definition of the public API for
 * colormodels.
 * Each instance of this class represents a prober color space
 * as subclasses attach a KoColorProfile at runtime
 */
class PIGMENT_EXPORT KoColorSpace {

protected:
    /// Only for use by classes that serve as baseclass fro real colorspaces
    KoColorSpace() {};

public:
    /// Should be called by real colorspaces
    KoColorSpace(const KoID &id, KoColorSpaceFactoryRegistry * parent);
    virtual ~KoColorSpace();

    virtual bool operator==(const KoColorSpace& rhs) const {
        return id().id() == rhs.id().id();
    }


public:

    //========== Channels =====================================================//

    /// Return a vector describing all the channels this color model has.
    virtual Q3ValueVector<KoChannelInfo *> channels() const = 0;

    /**
     * The total number of channels for a single pixel in this color model
     */
    virtual quint32 nChannels() const = 0;

    /**
     * The total number of color channels (excludes alpha and substance) for a single
     * pixel in this color model.
     */
    virtual quint32 nColorChannels() const = 0;

    /**
     * The total number of substance channels for a single pixel
     * in this color model
     */
    virtual quint32 nSubstanceChannels() const { return 0; };

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
     * channels normalised to the floating point range 0 to 1, if appropriate.
     */
    virtual QString normalisedChannelValueText(const quint8 *pixel, quint32 channelIndex) const = 0;

    /**
     * Convert the value of the channel at the specified position into
     * an 8-bit value. The position is not the number of bytes, but
     * the position of the channel as defined in the channel info list.
     */
    virtual quint8 scaleToU8(const quint8 * srcPixel, qint32 channelPos) = 0;

    /**
     * Convert the value of the channel at the specified position into
     * a 16-bit value. This may be upscaling or downscaling, depending
     * on the defined value of the channel
     */
     virtual quint16 scaleToU16(const quint8 * srcPixel, qint32 channelPos) = 0;

     /**
      * Set dstPixel to the pixel containing only the given channel of srcPixel. The remaining channels
      * should be set to whatever makes sense for 'empty' channels of this colour space,
      * with the intent being that the pixel should look like it only has the given channel.
      */
     virtual void getSingleChannelPixel(quint8 *dstPixel, const quint8 *srcPixel, quint32 channelIndex) = 0;

    //========== Identification ===============================================//

    /**
     * ID for use in files and internally: unchanging name +
     * i18n'able description.
     */
    virtual KoID id() const {return m_id;};

    /**
     * lcms colorspace type definition.
     */
    virtual quint32 colorSpaceType() = 0;

    virtual icColorSpaceSignature colorSpaceSignature() = 0;

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
    virtual bool willDegrade(ColorSpaceIndependence independence) = 0;

    //========== Capabilities =================================================//

    /**
     * Returns the list of user-visible composite ops supported by this colourspace. Internal
     * ops such as COPY, CLEAR, and ERASE, are not included as these make no sense
     * for layers in the full image model.
     */
    virtual KoCompositeOpList userVisiblecompositeOps() const = 0;

    /**
     * Returns true if the colorspace supports channel values outside the
     * (normalised) range 0 to 1.
     */
    virtual bool hasHighDynamicRange() const = 0;


    //========== Display profiles =============================================//

    /**
     * Return the profile of this color space. This may be 0
     */
    virtual KoColorProfile * getProfile() = 0;

//================= Conversion functions ==================================//


    /**
     * The fromQColor methods take a given color defined as an RGB QColor
     * and fills a byte array with the corresponding color in the
     * the colorspace managed by this strategy.
     *
     * @param c the QColor that will be used to fill dst
     * @param dst a pointer to a pixel
     * @param profile the optional profile that describes the color values of QColor
     */
    virtual void fromQColor(const QColor& c, quint8 *dst, KoColorProfile * profile = 0) = 0;

    /**
     * The fromQColor methods take a given color defined as an RGB QColor
     * and fills a byte array with the corresponding color in the
     * the colorspace managed by this strategy.
     *
     * @param c the QColor that will be used to fill dst
     * @param opacity the opacity of the color
     * @param dst a pointer to a pixel
     * @param profile the optional profile that describes the color values of QColor
     */
    virtual void fromQColor(const QColor& c, quint8 opacity, quint8 *dst, KoColorProfile * profile = 0) = 0;

    /**
     * The toQColor methods take a byte array that is at least pixelSize() long
     * and converts the contents to a QColor, using the given profile as a source
     * profile and the optional profile as a destination profile.
     *
     * @param src a pointer to the source pixel
     * @param c the QColor that will be filled with the color at src
     * @param profile the optional profile that describes the color in c, for instance the monitor profile
     */
    virtual void toQColor(const quint8 *src, QColor *c, KoColorProfile * profile = 0) = 0;

    /**
     * The toQColor methods take a byte array that is at least pixelSize() long
     * and converts the contents to a QColor, using the given profile as a source
     * profile and the option profile as a destination profile.
     *
     * @param src a pointer to the source pixel
     * @param c the QColor that will be filled with the color at src
     * @param opacity a pointer to a byte that will be filled with the opacity a src
     * @param profile the optional profile that describes the color in c, for instance the monitor profile
     */
    virtual void toQColor(const quint8 *src, QColor *c, quint8 *opacity, KoColorProfile * profile = 0) = 0;

    /**
     * Convert the pixels in data to (8-bit BGRA) QImage using the specified profiles.
     * The pixels are supposed to be encoded in this color model. The default implementation
     * will convert the pixels using either the profiles or the default profiles for the
     * current colorstrategy and the RGBA colorstrategy. If that is not what you want,
     * or if you think you can do better than lcms, reimplement this methods.
     *
     * @param data A pointer to a contiguous memory region containing width * height pixels
     * @param width in pixels
     * @param height in pixels
     * @param dstProfile destination profile
     * @param renderingIntent the rendering intent
     * @param exposure The exposure setting for rendering a preview of a high dynamic range image.
     */
    virtual QImage convertToQImage(const quint8 *data, qint32 width, qint32 height,
                                   KoColorProfile *  dstProfile, qint32 renderingIntent = INTENT_PERCEPTUAL,
                                   float exposure = 0.0f) = 0;

    /**
     * This functions allocates the ncessary memory for numPixels number of pixels.
     * It is your responsibility to delete[] it.
     */
    quint8 *allocPixelBuffer(quint32 numPixels) const;

    /**
     * Convert the specified data to Lab. All colorspaces are guaranteed to support this
     *
     * @param src the source data
     * @param dst the destination data
     * @param nPixels the number of source pixels
     */
    virtual void toLabA16(const quint8 * src, quint8 * dst, const quint32 nPixels) const = 0;

    /**
     * Convert the specified data from Lab. to this colorspace. All colorspaces are
     * guaranteed to support this.
     *
     * @param src the pixels in 16 bit lab format
     * @param dst the destination data
     * @param nPixels the number of pixels in the array
     */
    virtual void fromLabA16(const quint8 * src, quint8 * dst, const quint32 nPixels) const = 0;

    /**
     * Convert a byte array of srcLen pixels *src to the specified color space
     * and put the converted bytes into the prepared byte array *dst.
     *
     * Returns false if the conversion failed, true if it succeeded
     */
    virtual bool convertPixelsTo(const quint8 * src,
                                 quint8 * dst, KoColorSpace * dstColorSpace,
                                 quint32 numPixels,
                                 qint32 renderingIntent = INTENT_PERCEPTUAL) = 0;

//============================== Manipulation functions ==========================//


//
// The manipulation functions have default implementations that _convert_ the pixel
// to a QColor and back. Reimplement these methods in your color strategy!
//

    /**
    * Get the alpha value of the given pixel, downscaled to an 8-bit value.
    */
    virtual quint8 getAlpha(const quint8 * pixel) const = 0;

    /**
     * Set the alpha channel of the given run of pixels to the given value.
     *
     * pixels -- a pointer to the pixels that will have their alpha set to this value
     * alpha --  a downscaled 8-bit value for opacity
     * nPixels -- the number of pixels
     *
     */
    virtual void setAlpha(quint8 * pixels, quint8 alpha, qint32 nPixels) const = 0;

    /**
     * Multiply the alpha channel of the given run of pixels by the given value.
     *
     * pixels -- a pointer to the pixels that will have their alpha set to this value
     * alpha --  a downscaled 8-bit value for opacity
     * nPixels -- the number of pixels
     *
     */
    virtual void multiplyAlpha(quint8 * pixels, quint8 alpha, qint32 nPixels) = 0;

    /**
     * Applies the specified 8-bit alpha mask to the pixels. We assume that there are just
     * as many alpha values as pixels but we do not check this; the alpha values
     * are assumed to be 8-bits.
     */
    virtual void applyAlphaU8Mask(quint8 * pixels, quint8 * alpha, qint32 nPixels) = 0;

    /**
     * Applies the inverted 8-bit alpha mask to the pixels. We assume that there are just
     * as many alpha values as pixels but we do not check this; the alpha values
     * are assumed to be 8-bits.
     */
    virtual void applyInverseAlphaU8Mask(quint8 * pixels, quint8 * alpha, qint32 nPixels) = 0;

    /**
     * Create an adjustment object for adjusting the brightness and contrast
     * transferValues is a 256 bins array with values from 0 to 0xFFFF
     */
    virtual KoColorAdjustment *createBrightnessContrastAdjustment(quint16 *transferValues) = 0;

    /**
     * Create an adjustment object for desaturating
     */
    virtual KoColorAdjustment *createDesaturateAdjustment() = 0;

    /**
     * Create an adjustment object for adjusting individual channels
     * transferValues is an array of nColorChannels number of 256 bins array with values from 0 to 0xFFFF
     */
    virtual KoColorAdjustment *createPerChannelAdjustment(quint16 **transferValues) = 0;

    /**
     * Apply the adjustment created with onr of the other functions
     */
    virtual void applyAdjustment(const quint8 *src, quint8 *dst, KoColorAdjustment *, qint32 nPixels) = 0;

    /**
     * Invert color channels of the given pixels
     */
    virtual void invertColor(quint8 * src, qint32 nPixels) = 0;

    // XXX: What with alpha channels? YYY: Add an overloaded function that takes alpha into account?
    /**
     * Get the difference between 2 colors, normalized in the range (0,255)
     */
    virtual quint8 difference(const quint8* src1, const quint8* src2) = 0;


    /**
     * Mix the colors given their weights and return in dst
     * The sum of weights is assumed 255 */
    virtual void mixColors(const quint8 **colors, const quint8 *weights, quint32 nColors, quint8 *dst) const = 0;

    /**
     * Convolve the given array of pointers to pixels and return the result
     * in dst. The kernel values are clamped between -128 and 128
     */
    virtual void convolveColors(quint8** colors, qint32* kernelValues, KoChannelInfo::enumChannelFlags channelFlags, quint8 *dst, qint32 factor, qint32 offset, qint32 nPixels) const = 0;

    /**
     * Darken all color channels with the given amount. If compensate is true,
     * the compensation factor will be used to limit the darkening.
     *
     * (See the bumpmap filter)
     */
    virtual void darken(const quint8 * src, quint8 * dst, qint32 shade, bool compensate, double compensation, qint32 nPixels) const = 0;

    /**
     * Calculate the intensity of the given pixel, scaled down to the range 0-255. XXX: Maybe this should be more flexible
    */
    virtual quint8 intensity8(const quint8 * src) const = 0;

    /**
     * Create a mathematical toolbox compatible with this colorspace
     */
    virtual KoID mathToolboxID() const =0;

    /**
     * Compose two arrays of pixels together. If source and target
     * are not the same colour model, the source pixels will be
     * converted to the target model.
     */
    virtual void bitBlt(quint8 *dst,
                qint32 dststride,
                KoColorSpace * srcSpace,
                const quint8 *src,
                qint32 srcRowStride,
                const quint8 *srcAlphaMask,
                qint32 maskRowStride,
                quint8 opacity,
                qint32 rows,
                qint32 cols,
                const KoCompositeOp& op) = 0;

    /**
     * The backgroundfilters will be run periodically on the newly
     * created paint device. XXX: Currently this uses times and not
     * threads.
     */
    virtual Q3ValueList<KisFilter*> createBackgroundFilters()
        { return Q3ValueList<KisFilter*>(); };

private:

    DCOPObject * m_dcop;
    KoID m_id;

protected:
    KoColorSpaceFactoryRegistry * m_parent;
    Q3ValueVector<KoChannelInfo *> m_channels;

};

class KoColorSpaceFactory {
public:
	virtual ~KoColorSpaceFactory() {}
    /**
     * ID for use in files and internally: unchanging name +
     * i18n'able description.
     */
    virtual KoID id() const = 0;

    /**
     * lcms colorspace type definition.
     */
    virtual quint32 colorSpaceType() = 0;

    virtual icColorSpaceSignature colorSpaceSignature() = 0;

    virtual KoColorSpace *createColorSpace(KoColorSpaceFactoryRegistry * parent, KoColorProfile *) = 0;

    /**
     * Returns the default icc profile for use with this colorspace. This may be ""
     *
     & @return the default icc profile name
     */
    virtual QString defaultProfile() = 0;

};

#endif // KOCOLORSPACE_H
