/*
 *  Copyright (c) 2003 Patrick Julien  <freak@codepimps.org>
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
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

#ifndef KIS_COLORSPACE_FACTORY_REGISTRY_H_
#define KIS_COLORSPACE_FACTORY_REGISTRY_H_
#include "qobject.h"
#include "kis_generic_registry.h"
#include "kis_colorspace.h"

class QStringList;
class KisPaintDeviceAction;

/**
 * This class contains:
 *      - a registry of colorspace instantiated with specific profiles.
 *      - a registry of singleton colorspace factories.
 *      - a registry of icc profiles
 */
class KisColorSpaceFactoryRegistry : public QObject,  public KisGenericRegistry<KisColorSpaceFactory *> {


    Q_OBJECT

public:

    /**
     * Create a new colorspacefactory registry. The registry will
     * load all colorspace modules that have the right version and
     * all profiles given in the list. It is always possible
     * to add more profiles with addProfile()
     *
     * @param profileFileNames a list of all filenames of all profiles that need to be loaded initially
     */
    KisColorSpaceFactoryRegistry(QStringList profileFileNames);

    virtual ~KisColorSpaceFactoryRegistry();

    /**
     * Add the profile to the list.
     */
    void addProfile(KisProfile * p);

    /**
     * Return the profile associated with the given product name,
     * or 0.
     */
    KisProfile *  getProfileByName(const QString & name);

    /**
     * Return the vector of profiles for this colorspacefactory
     */
    QValueVector<KisProfile *>  profilesFor(KisColorSpaceFactory * cs);

    QValueVector<KisProfile *>  profilesFor(KisID id);

    /**
     * Return the colorspace + profile as named, or NULL if impossible combination.
     */
    KisColorSpace *  getColorSpace(const KisID & csID, const QString & profileName);

    /**
     * Return the colorspace + profile -- where the profile is matched on the name of the specified profile
     */
    KisColorSpace * getColorSpace(const KisID & csID, const KisProfile * profile);

    /**
     * Convenience method to get the often used alpha colorspace
     */
    KisColorSpace * getAlpha8();

    /**
     * Convenience method to get an RGB colorspace with the default lcms profile
     */
    KisColorSpace * getRGB8();

    /**
     * add a KisConstructPaintDeviceAction to the registry for a colorspace
     * 
     * These actions are exectued when an image is created on the first layer
     * in the image, on the image width and height rect.
     */
    void addPaintDeviceAction(KisColorSpace* cs, KisPaintDeviceAction* action);

    /**
     * Get a list of KisConstructPaintDeviceAction for a colorspace
     */
    QValueVector<KisPaintDeviceAction *> paintDeviceActionsFor(KisColorSpace* cs);

private:
    KisColorSpaceFactoryRegistry();
    KisColorSpaceFactoryRegistry(const KisColorSpaceFactoryRegistry&);
    KisColorSpaceFactoryRegistry operator=(const KisColorSpaceFactoryRegistry&);

private:

    QMap<QString, KisProfile * > m_profileMap;
    QMap<QString, KisColorSpace * > m_csMap;
    typedef QValueVector<KisPaintDeviceAction *> PaintActionVector;
    QMap<KisID, PaintActionVector> m_paintDevActionMap;
    KisColorSpace *m_alphaCs;
};

#endif // KIS_COLORSPACE_FACTORY_REGISTRY_H_

