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
#ifndef KOCOLORSPACEFACTORY_H
#define KOCOLORSPACEFACTORY_H

#include "KoColorSpaceConstants.h"
#include "KoColorConversionTransformation.h"
#include <KoChannelInfo.h>
#include <KoID.h>
#include "pigment_export.h"

class KoCompositeOp;
class KoColorProfile;
class KoColorTransformation;
class KoColorConversionTransformationFactory;
class QBitArray;

/**
 * This class is used to create color spaces.
 */
class PIGMENTCMS_EXPORT KoColorSpaceFactory
{
protected:
    KoColorSpaceFactory();
public:
    virtual ~KoColorSpaceFactory();
    /**
     * Return the unchanging name of this color space
     */
    virtual QString id() const = 0;

    /**
     * return the i18n'able description.
     */
    virtual QString name() const = 0;

    /**
     * @return true if the color space should be shown in an User Interface, or false
     *         other wise.
     */
    virtual bool userVisible() const = 0;

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
     * @param profile a pointer to a color profile
     * @return true if the color profile can be used by a color space created by
     * this factory
     */
    virtual bool profileIsCompatible(const KoColorProfile* profile) const = 0;

    /**
     * @return the name of the color space engine for this color space, or "" if none
     */
    virtual QString colorSpaceEngine() const = 0;

    /**
     * @return true if the color space supports High-Dynamic Range.
     */
    virtual bool isHdr() const = 0;

    /**
     * @return the reference depth, that is for a color space where all channels have the same
     * depth, this is the depth of one channel, for a color space with different bit depth for
     * each channel, it's usually the highest bit depth. This value is used by the Color
     * Conversion System to check if a lost of bit depth during a color conversion is
     * acceptable, for instance when converting from RGB32bit to XYZ16bit, it's acceptable to go
     * through a conversion to RGB16bit, while it's not the case for RGB32bit to XYZ32bit.
     */
    virtual int referenceDepth() const = 0;

    /**
     * @return the list of color conversion provided by this colorspace, the factories
     * constructed by this functions are owned by the caller of the function
     */
    virtual QList<KoColorConversionTransformationFactory*> colorConversionLinks() const = 0;

    /**
     * Returns the default icc profile for use with this colorspace. This may be ""
     *
     * @return the default icc profile name
     */
    virtual QString defaultProfile() const = 0;

    /**
     * Create a color profile from a memory array, if possible, otherwise return 0.
     */
    const KoColorProfile* colorProfile(const QByteArray& rawData) const;

    KoColorSpace* grabColorSpace(const KoColorProfile* profile);

    void releaseColorSpace(KoColorSpace *);
protected:
    /**
     * creates a color space using the given profile.
     */
    virtual KoColorSpace *createColorSpace(const KoColorProfile *) const = 0;
    virtual KoColorProfile* createColorProfile(const QByteArray& rawData) const = 0;
private:
    struct Private;
    Private* const d;
};

#endif // KOCOLORSPACEFACTORY_H
