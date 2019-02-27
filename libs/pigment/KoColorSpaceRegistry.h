/*
 *  Copyright (c) 2003 Patrick Julien  <freak@codepimps.org>
 *  Copyright (c) 2004,2010 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
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

#ifndef KOCOLORSPACEREGISTRY_H
#define KOCOLORSPACEREGISTRY_H

#include <QObject>
#include <QList>
#include <QString>
#include "kritapigment_export.h"

#include <KoGenericRegistry.h>
#include <KoColorSpace.h>
#include <KoColorSpaceFactory.h>
#include <KoConfig.h>

class KoColorProfile;
class KoColorConversionSystem;
class KoColorConversionCache;
class KoColorConversionTransformation;

/**
 * The registry for colorspaces and profiles.
 * This class contains:
 *      - a registry of colorspace instantiated with specific profiles.
 *      - a registry of singleton colorspace factories.
 *      - a registry of icc profiles
 *
 * Locking policy details:
 *
 * Basically, we have two levels of locks in the registry:
 * 1) (outer level) is Private::registrylock, which controls the structures
 *     of the color space registry itself
 * 2) (inner level) is KoColorProfileStorage::Private::lock controls
 *    the structures related to profiles.
 *
 * The locks can be taken individually, but if you are going to take both
 * of them, you should always follow the order 1) registry; 2) profiles.
 * Otherwise you'll get a deadlock.
 *
 * To avoid recursive deadlocks, all the dependent classes
 * (KoColorConversionSystem and KoColorSpaceFactory) now do not use the direct
 * links to the registry. Instead, they use special private interfaces that
 * skip recursive locking and ensure we take a lock twice.
 */
class KRITAPIGMENT_EXPORT KoColorSpaceRegistry
{
public:
    KoColorSpaceRegistry();

    enum ColorSpaceListVisibility {
        OnlyUserVisible = 1, ///< Only user visible color space
        AllColorSpaces = 4 ///< All color space even those not visible to the user
    };
    enum ColorSpaceListProfilesSelection {
        OnlyDefaultProfile = 1, ///< Only add the default profile
        AllProfiles = 4 ///< Add all profiles
    };

    /**
     * Return an instance of the KoColorSpaceRegistry
     * Creates an instance if that has never happened before and returns the singleton instance.
     */
    static KoColorSpaceRegistry * instance();

    virtual ~KoColorSpaceRegistry();

public:
    /**
      * add a color space to the registry
      * @param item the color space factory to add
      */
    void add(KoColorSpaceFactory* item);

    /**
     * Remove a color space factory from the registry. Note that it is the
     * responsibility of the caller to ensure that the colorspaces are not
     * used anymore.
     */
    void remove(KoColorSpaceFactory* item);

    /**
     * Add a profile to the profile map but do not add it to the
     * color conversion system yet.
     * @param profile the new profile to be registered.
     */
    void addProfileToMap(KoColorProfile *p);

    /**
     * register the profile with the color space registry
     * @param profile the new profile to be registered so it can be combined with
     *  colorspaces.
     */
    void addProfile(KoColorProfile* profile);
    void addProfile(const KoColorProfile* profile); // TODO why ?
    void removeProfile(KoColorProfile* profile);

    /**
     * Create an alias to a profile with a different name. Then @ref profileByName
     * will return the profile @p to when passed @p name as a parameter.
     */
    void addProfileAlias(const QString& name, const QString& to);

    /**
     * @return the profile alias, or name if not aliased
     */
    QString profileAlias(const QString& name) const;

    /**
     * create a profile of the specified type.
     */
    const KoColorProfile *createColorProfile(const QString & colorModelId, const QString & colorDepthId, const QByteArray& rawData);

    /**
     * Return a profile by its given name, or 0 if none registered.
     * @return a profile by its given name, or 0 if none registered.
     * @param name the product name as set on the profile.
     * @see addProfile()
     * @see KoColorProfile::productName()
     */
    const KoColorProfile * profileByName(const QString & name) const ;


    /**
     * Returns a profile by its unique id stored/calculated in the header.
     * The first call to this function might take long, because the map is
     * created on the first use only (atm used by SVG only)
     * @param id unique ProfileID of the profile (MD5 sum of its header)
     * @return the profile or 0 if not found
     */
    const KoColorProfile *profileByUniqueId(const QByteArray &id) const;

    bool profileIsCompatible(const KoColorProfile* profile, const QString &colorSpaceId);

    /**
     * Return the list of profiles for a colorspace with the argument id.
     * Profiles will not work with any color space, you can query which profiles
     * that are registered with this registry can be used in combination with the
     * argument factory.
     * @param colorSpaceId the colorspace-id with which all the returned profiles will work.
     * @return a list of profiles for the factory
     */
    QList<const KoColorProfile *>  profilesFor(const QString& csID) const;
    QString defaultProfileForColorSpace(const QString &colorSpaceId) const;

    /**
     * This function is called by the color space to create a color conversion
     * between two color space. This function search in the graph of transformations
     * the best possible path between the two color space.
     */
    KoColorConversionTransformation* createColorConverter(const KoColorSpace * srcColorSpace, const KoColorSpace * dstColorSpace, KoColorConversionTransformation::Intent renderingIntent, KoColorConversionTransformation::ConversionFlags conversionFlags) const;

    /**
     * This function creates two transformations, one from the color space and one to the
     * color space. The destination color space is picked from a list of color space, such
     * as the conversion between the two color space is of the best quality.
     *
     * The typical use case of this function is for KoColorTransformationFactory which
     * doesn't support all color spaces, so unsupported color space have to find an
     * acceptable conversion in order to use that KoColorTransformationFactory.
     *
     * @param colorSpace the source color space
     * @param possibilities a list of color space among which we need to find the best
     *                      conversion
     * @param fromCS the conversion from the source color space will be affected to this
     *               variable
     * @param toCS the revert conversion to the source color space will be affected to this
     *             variable
     */
    void createColorConverters(const KoColorSpace* colorSpace, const QList< QPair<KoID, KoID> >& possibilities, KoColorConversionTransformation*& fromCS, KoColorConversionTransformation*& toCS) const;

    /**
     * Return a colorspace that works with the parameter profile.
     * @param colorSpaceId the ID string of the colorspace that you want to have returned
     * @param profile the profile be combined with the colorspace
     * @return the wanted colorspace, or 0 when the cs and profile can not be combined.
     */
    const KoColorSpace * colorSpace(const QString & colorModelId, const QString & colorDepthId, const KoColorProfile *profile);

    /**
     * Return a colorspace that works with the parameter profile.
     * @param profileName the name of the KoColorProfile to be combined with the colorspace
     * @return the wanted colorspace, or 0 when the cs and profile can not be combined.
     */
    const KoColorSpace * colorSpace(const QString & colorModelId, const QString & colorDepthId, const QString &profileName);

    const KoColorSpace * colorSpace(const QString & colorModelId, const QString & colorDepthId);

    /**
     * Return the id of the colorspace that have the defined colorModelId with colorDepthId.
     * @param colorModelId id of the color model
     * @param colorDepthId id of the color depth
     * @return the id of the wanted colorspace, or "" if no colorspace correspond to those ids
     */
    QString colorSpaceId(const QString & colorModelId, const QString & colorDepthId) const;
    /**
     * It's a convenient function that behave like the above.
     * Return the id of the colorspace that have the defined colorModelId with colorDepthId.
     * @param colorModelId id of the color model
     * @param colorDepthId id of the color depth
     * @return the id of the wanted colorspace, or "" if no colorspace correspond to those ids
     */
    QString colorSpaceId(const KoID& colorModelId, const KoID& colorDepthId) const;

    /**
     * @return the identifier of the color model for the given color space id.
     *
     * This function is a compatibility function used to get the color space from
     * all kra files.
     */
    KoID colorSpaceColorModelId(const QString & _colorSpaceId) const;

    /**
     * @return the identifier of the color depth for the given color space id.
     *
     * This function is a compatibility function used to get the color space from
     * all kra files.
     */
    KoID colorSpaceColorDepthId(const QString & _colorSpaceId) const;

    /**
     * Convenience methods to get the often used alpha colorspaces
     */
    const KoColorSpace *alpha8();
    const KoColorSpace *alpha16();
#ifdef HAVE_OPENEXR
    const KoColorSpace *alpha16f();
#endif
    const KoColorSpace *alpha32f();

    /**
     * Convenience method to get an RGBA 8bit colorspace. If a profile is not specified,
     * an sRGB profile will be used.
     * @param profileName the name of an RGB color profile
     * @return the wanted colorspace, or 0 if the color space and profile can not be combined.
     */
    const KoColorSpace * rgb8(const QString &profileName = QString());

    /**
     * Convenience method to get an RGBA 8bit colorspace with the given profile.
     * @param profile an RGB profile
     * @return the wanted colorspace, or 0 if the color space and profile can not be combined.
     */
    const KoColorSpace * rgb8(const KoColorProfile * profile);

    /**
     * Convenience method to get an RGBA 16bit colorspace. If a profile is not specified,
     * an sRGB profile will be used.
     * @param profileName the name of an RGB color profile
     * @return the wanted colorspace, or 0 if the color space and profile can not be combined.
     */
    const KoColorSpace * rgb16(const QString &profileName = QString());

    /**
     * Convenience method to get an RGBA 16bit colorspace with the given profile.
     * @param profile an RGB profile
     * @return the wanted colorspace, or 0 if the color space and profile can not be combined.
     */
    const KoColorSpace * rgb16(const KoColorProfile * profile);

    /**
     * Convenience method to get an Lab 16bit colorspace. If a profile is not specified,
     * an Lab profile with a D50 whitepoint will be used.
     * @param profileName the name of an Lab color profile
     * @return the wanted colorspace, or 0 if the color space and profile can not be combined.
     */
    const KoColorSpace * lab16(const QString &profileName = QString());

    /**
     * Convenience method to get an Lab 16bit colorspace with the given profile.
     * @param profile an Lab profile
     * @return the wanted colorspace, or 0 if the color space and profile can not be combined.
     */
    const KoColorSpace * lab16(const KoColorProfile * profile);

    /**
     * Convenience method to get a standard profile for Rec.2020 linear light
     * color space
     */
    const KoColorProfile *p2020G10Profile() const;

    /**
     * Convenience method to get a standard profile for Rec.2020 PQ color space
     */
    const KoColorProfile *p2020PQProfile() const;

    /**
     * Convenience method to get a standard profile for Rec. 709 linear light
     * color space
     */
    const KoColorProfile *p709G10Profile() const;

    /**
     * Convenience method to get a standard profile for Rec. 709 sRGB-tone-
     * response-curve profile
     */
    const KoColorProfile *p709SRGBProfile() const;

    /**
     * @return the list of available color models
     */
    QList<KoID> colorModelsList(ColorSpaceListVisibility option) const;

    /**
     * @return the list of available color models for the given colorModelId
     */
    QList<KoID> colorDepthList(const KoID& colorModelId, ColorSpaceListVisibility option) const;

    /**
     * @return the list of available color models for the given colorModelId
     */
    QList<KoID> colorDepthList(const QString & colorModelId, ColorSpaceListVisibility option) const;

    /**
     * @return the cache of color conversion transformation to be use by KoColorSpace
     */
    KoColorConversionCache* colorConversionCache() const;

    /**
     * @return a permanent colorspace owned by the registry, of the same type and profile
     *         as the one given in argument
     */
    const KoColorSpace* permanentColorspace(const KoColorSpace* _colorSpace);

    /**
     * This function return a list of all the keys in KoID format by using the name() method
     * on the objects stored in the registry.
     */
    QList<KoID> listKeys() const;

private:

    friend class KisCsConversionTest;
    friend class KisIteratorTest;
    friend class KisIteratorNGTest;
    friend class KisPainterTest;
    friend class KisCrashFilterTest;
    friend class KoColorSpacesBenchmark;
    friend class TestKoColorSpaceSanity;
    friend class TestColorConversionSystem;
    friend struct FriendOfColorSpaceRegistry;

    /**
     * @return a list with an instance of all color space with their default profile.
     */
    QList<const KoColorSpace*> allColorSpaces(ColorSpaceListVisibility visibility, ColorSpaceListProfilesSelection pSelection);

    /**
     * @return the color conversion system use by the registry and the color
     * spaces to create color conversion transformation.
     *
     * WARNING: conversion system is guarded by the registry locks, don't
     *          use it anywhere other than unittests!
     */
    const KoColorConversionSystem* colorConversionSystem() const;

private:
    KoColorSpaceRegistry(const KoColorSpaceRegistry&);
    KoColorSpaceRegistry operator=(const KoColorSpaceRegistry&);
    void init();

private:
    struct Private;
    Private * const d;
};

#endif // KOCOLORSPACEREGISTRY_H
