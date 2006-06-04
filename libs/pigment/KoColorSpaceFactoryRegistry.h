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

#ifndef KOCOLORSPACEFACTORYREGISTRY_H
#define KOCOLORSPACEFACTORYREGISTRY_H

#include <QObject>
#include <QMap>
#include <QList>
#include <QString>
#include <koffice_export.h>

#include <KoGenericRegistry.h>
#include "KoColorSpace.h"

class QStringList;
class KisPaintDeviceAction;

/**
 * This class contains:
 *      - a registry of colorspace instantiated with specific profiles.
 *      - a registry of singleton colorspace factories.
 *      - a registry of icc profiles
 */
class PIGMENT_EXPORT KoColorSpaceFactoryRegistry : public QObject,  public KisGenericRegistry<KoColorSpaceFactory *> {


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
    KoColorSpaceFactoryRegistry(QStringList profileFileNames);

    virtual ~KoColorSpaceFactoryRegistry();

    /**
     * Add the profile to the list.
     */
    void addProfile(KoColorProfile * p);

    /**
     * Return the profile associated with the given product name,
     * or 0.
     */
    KoColorProfile *  getProfileByName(const QString & name);

    /**
     * Return the list of profiles for this colorspacefactory
     */
    QList<KoColorProfile *>  profilesFor(KoColorSpaceFactory * cs);

    QList<KoColorProfile *>  profilesFor(KoID id);

    /**
     * Return the colorspace + profile as named, or NULL if impossible combination.
     */
    KoColorSpace *  getColorSpace(const KoID & csID, const QString & profileName);

    /**
     * Return the colorspace + profile -- where the profile is matched on the name of the specified profile
     */
    KoColorSpace * getColorSpace(const KoID & csID, const KoColorProfile * profile);

    /**
     * Convenience method to get the often used alpha colorspace
     */
    KoColorSpace * getAlpha8();

    /**
     * Convenience method to get an RGB colorspace with the default lcms profile
     */
    KoColorSpace * getRGB8();

    /**
     * add a KisConstructPaintDeviceAction to the registry for a colorspace
     *
     * These actions are exectued when an image is created on the first layer
     * in the image, on the image width and height rect.
     */
    void addPaintDeviceAction(KoColorSpace* cs, KisPaintDeviceAction* action);

    /**
     * Get a list of KisConstructPaintDeviceAction for a colorspace
     */
    QList<KisPaintDeviceAction *> paintDeviceActionsFor(KoColorSpace* cs);

private:
    KoColorSpaceFactoryRegistry();
    KoColorSpaceFactoryRegistry(const KoColorSpaceFactoryRegistry&);
    KoColorSpaceFactoryRegistry operator=(const KoColorSpaceFactoryRegistry&);

private:

    QMap<QString, KoColorProfile * > m_profileMap;
    QMap<QString, KoColorSpace * > m_csMap;
    typedef QList<KisPaintDeviceAction *> PaintActionList;
    QMap<KoID, PaintActionList> m_paintDevActionMap;
    KoColorSpace *m_alphaCs;
};

#endif // KOCOLORSPACEFACTORYREGISTRY_H

