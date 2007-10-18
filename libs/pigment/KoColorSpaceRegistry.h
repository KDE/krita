/*
 *  Copyright (c) 2003 Patrick Julien  <freak@codepimps.org>
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
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

#ifndef KOCOLORSPACEFACTORYREGISTRY_H
#define KOCOLORSPACEFACTORYREGISTRY_H

#include <QObject>
#include <QMap>
#include <QList>
#include <QString>
#include <pigment_export.h>

#include <KoGenericRegistry.h>

class KisPaintDeviceAction;
class KoColorSpace;
class KoColorSpaceFactory;
class KoColorProfile;
class KoColorConversionSystem;

/**
 *
 * XXX: USE STATIC DELETER! USE STATIC DELETER!
 *
 * The registry for colorspaces and profiles.
 * This class contains:
 *      - a registry of colorspace instantiated with specific profiles.
 *      - a registry of singleton colorspace factories.
 *      - a registry of icc profiles
 */
class PIGMENT_EXPORT KoColorSpaceRegistry : public QObject,  public KoGenericRegistry<KoColorSpaceFactory *> {


    Q_OBJECT
public:
    enum ColorSpaceListVisibility {
        OnlyUserVisible = 1, ///< Only user visible color space
        AllColorSpaces = 4 ///< All color space even those not visible to the user
    };
public:

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
     * add a color space to the registry
     * @param id the id of the color space
     * @param item the color space factory to add
     */
    void add(const QString &id, KoColorSpaceFactory* item);

    /**
     * register the profile with the color space registry
     * @param profile the new profile to be registered so it can be combined with
     *  colorspaces.
     */
    void addProfile(KoColorProfile * profile);

    /**
     * Return a profile by its given name, or 0 if none registered.
     * @return a profile by its given name, or 0 if none registered.
     * @param name the product name as set on the profile.
     * @see addProfile()
     * @see KoColorProfile::productName()
     */
    KoColorProfile *  profileByName(const QString & name) const ;

    /**
     * Return the list of profiles for the argument colorspacefactory.
     * Profiles will not work with any color space, you can query which profiles
     * that are registered with this registry can be used in combination with the
     * argument factory.
     * @param factory the factory with which all the returned profiles will work.
     * @return a list of profiles for the factory
     */
    QList<KoColorProfile *>  profilesFor(KoColorSpaceFactory * factory);

    /**
     * Return the list of profiles for a colorspace with the argument id.
     * Profiles will not work with any color space, you can query which profiles
     * that are registered with this registry can be used in combination with the
     * argument factory.
     * @param id the colorspace-id with which all the returned profiles will work.
     * @return a list of profiles for the factory
     */
    QList<KoColorProfile *>  profilesFor(KoID id) {
        return profilesFor(id.id());
    }

    /**
     * Return the list of profiles for a colorspace with the argument id.
     * Profiles will not work with any color space, you can query which profiles
     * that are registered with this registry can be used in combination with the
     * argument factory.
     * @param colorSpaceId the colorspace-id with which all the returned profiles will work.
     * @return a list of profiles for the factory
     */
    QList<KoColorProfile *>  profilesFor(const QString &colorSpaceId);

    /**
     * Return a colorspace that works with the parameter profile.
     * @param csID the ID of the colorspace that you want to have returned
     * @param profileName the name of the KoColorProfile to be combined with the colorspace
     * @return the wanted colorspace, or 0 when the cs and profile can not be combined.
     */
    KoColorSpace *  colorSpace(const KoID &csID, const QString & profileName) {
        return colorSpace(csID.id(), profileName);
    }

    /**
     * Return a colorspace that works with the parameter profile.
     * @param colorSpaceId the ID string of the colorspace that you want to have returned
     * @param profile the profile be combined with the colorspace
     * @return the wanted colorspace, or 0 when the cs and profile can not be combined.
     */
    KoColorSpace * colorSpace(const QString &colorSpaceId, const KoColorProfile *profile);

    /**
     * Return a colorspace that works with the parameter profile.
     * @param profileName the name of the KoColorProfile to be combined with the colorspace
     * @return the wanted colorspace, or 0 when the cs and profile can not be combined.
     */
    KoColorSpace * colorSpace(const QString &colorSpaceId, const QString &profileName);

    /**
     * Return the id of the colorspace that have the defined colorModelId with colorDepthId.
     * @param colorModelId id of the color model
     * @param colorDepthId id of the color depth
     * @return the id of the wanted colorspace, or "" if no colorspace correspond to those ids
     */
    QString colorSpaceId(QString colorModelId, QString colorDepthId);
    /**
     * It's a convenient function that behave like the above.
     * Return the id of the colorspace that have the defined colorModelId with colorDepthId.
     * @param colorModelId id of the color model
     * @param colorDepthId id of the color depth
     * @return the id of the wanted colorspace, or "" if no colorspace correspond to those ids
     */
    QString colorSpaceId(const KoID& colorModelId, const KoID& colorDepthId);
    
    /**
     * Convenience method to get the often used alpha colorspace
     */
    KoColorSpace * alpha8();

    /**
     * Convenience method to get an RGB 8bit colorspace. If a profile is not specified,
     * an sRGB profile will be used.
     * @param profileName the name of an RGB color profile
     * @return the wanted colorspace, or 0 if the color space and profile can not be combined.
     */
    KoColorSpace * rgb8(const QString &profileName = QString());
    
    /**
     * Convenience method to get an RGB 8bit colorspace with the given profile.
     * @param profile an RGB profile
     * @return the wanted colorspace, or 0 if the color space and profile can not be combined.
     */
    KoColorSpace * rgb8(KoColorProfile * profile);
    
    /**
     * Convenience method to get an RGB 16bit colorspace. If a profile is not specified,
     * an sRGB profile will be used.
     * @param profileName the name of an RGB color profile
     * @return the wanted colorspace, or 0 if the color space and profile can not be combined.
     */
    KoColorSpace * rgb16(const QString &profileName = QString());

    /**
     * Convenience method to get an RGB 16bit colorspace with the given profile.
     * @param profile an RGB profile
     * @return the wanted colorspace, or 0 if the color space and profile can not be combined.
     */
    KoColorSpace * rgb16(KoColorProfile * profile);

    /**
     * Convenience method to get an Lab 16bit colorspace. If a profile is not specified,
     * an Lab profile with a D50 whitepoint will be used.
     * @param profileName the name of an Lab color profile
     * @return the wanted colorspace, or 0 if the color space and profile can not be combined.
     */
    KoColorSpace * lab16(const QString &profileName = QString());

    /**
     * Convenience method to get an Lab 16bit colorspace with the given profile.
     * @param profile an Lab profile
     * @return the wanted colorspace, or 0 if the color space and profile can not be combined.
     */
    KoColorSpace * lab16(KoColorProfile * profile);

    /**
     * add a KisConstructPaintDeviceAction to the registry for a colorspace
     */
    void addPaintDeviceAction(KoColorSpace* colorSpace, KisPaintDeviceAction* action);

    /**
     * Get a list of KisConstructPaintDeviceAction for a colorspace
     */
    QList<KisPaintDeviceAction *> paintDeviceActionsFor(KoColorSpace* colorSpace);

    /**
     * @return the list of available color models
     */
    QList<KoID> colorModelsList(ColorSpaceListVisibility option ) const;
    /**
     * @return the list of available color models for the given colorModelId
     */
    QList<KoID> colorDepthList(const KoID& colorModelId, ColorSpaceListVisibility option ) const;
    
    /**
     * @return the color conversion system use by the registry and the color
     * spaces to create color conversion transformation
     */
    const KoColorConversionSystem* colorConversionSystem() const;
private:
    KoColorSpaceRegistry();
    KoColorSpaceRegistry(const KoColorSpaceRegistry&);
    KoColorSpaceRegistry operator=(const KoColorSpaceRegistry&);
    void init();

private:
    struct Private;
    Private * const d;
};

#endif // KOCOLORSPACEFACTORYREGISTRY_H

