/*
 *  SPDX-FileCopyrightText: 2005 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2006-2007 Cyrille Berger <cberger@cberger.net>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */
#ifndef KOCOLORSPACEFACTORY_H
#define KOCOLORSPACEFACTORY_H

#include "KoColorSpaceConstants.h"
#include "KoColorConversionTransformation.h"
#include <KoID.h>
#include "kritapigment_export.h"

class KoColorProfile;
class KoColorConversionTransformationFactory;

/**
 * This class is used to create color spaces.
 */
class KRITAPIGMENT_EXPORT KoColorSpaceFactory
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
     * @return true if the color space should be shown in a User Interface, or false
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
     * @return the cost of the usage of the colorspace in the conversion graph. The higher the cost,
     * the less probably the color space will be chosen for the conversion.
     */
    virtual int crossingCost() const = 0;

    /**
     * Returns the default icc profile for use with this colorspace. This may be ""
     *
     * @return the default icc profile name
     */
    virtual QString defaultProfile() const = 0;

    struct ProfileRegistrationInterface
    {
        virtual ~ProfileRegistrationInterface() {}
        virtual const KoColorProfile* profileByName(const QString &profileName) const = 0;
        virtual void registerNewProfile(KoColorProfile *profile) = 0;
    };

    /**
     * Create a color profile from a memory array, if possible, otherwise return 0.
     * This will replace the existing profile with the name in the KoColorSpaceRegistry
     *
     * This will call the decendant's createColorProfile()
     */
    const KoColorProfile* colorProfile(const QByteArray& rawData, ProfileRegistrationInterface *registrationInterface) const;

    /**
     * Create or reuse the existing colorspace for the given profile.
     *
     * This will call the decendant's createColorSpace
     */
    const KoColorSpace *grabColorSpace(const KoColorProfile *profile);

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
