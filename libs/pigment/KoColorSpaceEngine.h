/*
 *  SPDX-FileCopyrightText: 2008 Cyrille Berger <cberger@cberger.net>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef _KO_COLOR_SPACE_ENGINE_H_
#define _KO_COLOR_SPACE_ENGINE_H_

#include <KoColorConversionTransformationAbstractFactory.h>
#include <KoGenericRegistry.h>

class KoColorProfile;

/**
 * A KoColorSpaceEngine is a class use to create color conversion
 * transformation between color spaces, for which all profiles can
 * output to all profiles.
 *
 * Typically, when you have an ICC color space and color profile, you
 * can convert to any other ICC color space and color profile. While
 * creating a KoColorTransformationFactory for each of this transformation
 * is possible, the number of links will make the Color Conversion explode
 * System. KoColorSpaceEngine provides a virtual node in the Color
 * Conversion System that can convert to any other node supported by the
 * engine.
 */
class KRITAPIGMENT_EXPORT KoColorSpaceEngine : public KoColorConversionTransformationAbstractFactory
{
public:
    KoColorSpaceEngine(const QString& id, const QString& name);
    ~KoColorSpaceEngine() override;
    const QString& id() const;
    const QString& name() const;
    virtual const KoColorProfile* addProfile(const QString &filename) = 0;
    virtual const KoColorProfile* addProfile(const QByteArray &data) = 0;
    /**
     * @brief getProfile
     * This tries to get a profile matching the given characteristics. If there's no such profile
     * in the colorspace registery, a profile will be generated and added to the registery.
     * @param colorants a double of xy (for xyY) values, this expects the first two as the white point,
     * then the red, green and blue. If there's only a whitepoint, a grayscale profile will be returned.
     * @param colorPrimaries the color primaries type as defined in KoColorProfile.
     * @param transferFunction the transfer function, as defined in KoColorProfile.
     * @return a profile that matches these characteristics.
     */
    virtual const KoColorProfile* getProfile(QVector<double> colorants, int colorPrimaries, int transferFunction) = 0;
    virtual void removeProfile(const QString &filename) = 0;

    /**
     * \return true if the color space can be converted via this engine
     */
    virtual bool supportsColorSpace(const QString& colorModelId, const QString& colorDepthId, const KoColorProfile *profile) const;

private:
    struct Private;
    Private* const d;
};

class KRITAPIGMENT_EXPORT KoColorSpaceEngineRegistry : public KoGenericRegistry< KoColorSpaceEngine* >
{
public:
    KoColorSpaceEngineRegistry();
    ~KoColorSpaceEngineRegistry() override;
    static KoColorSpaceEngineRegistry* instance();
};

#endif
