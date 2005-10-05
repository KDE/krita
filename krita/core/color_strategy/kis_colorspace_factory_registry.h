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

#include "kis_types.h"
#include "kis_generic_registry.h"
#include "koffice_export.h"
#include "kis_colorspace.h"

class KisPixelOp;
class QStringList;


/**
 * This class contains:
 *      - a registry of singleton color strategies.
 *      - a registry of icc profiles
 *      - a registry of default pixel operations
 */
class KRITACORE_EXPORT KisColorSpaceFactoryRegistry : public KisGenericRegistry<KisColorSpaceFactory *> {

public:
    virtual ~KisColorSpaceFactoryRegistry();

    /**
     * Get the singleton instance of this registry
     */
    static KisColorSpaceFactoryRegistry* instance();

    /**
     * Reload the profiles from disk
     */
    void resetProfiles();

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
     * Convenience method to get the often used xyz16 colorspace
     */
    static KisColorSpace * getXYZ16();

    /**
     * Convenience method to get the often used alpha colorspace
     */
    static KisColorSpace * getAlpha8();

private:
    KisColorSpaceFactoryRegistry();
    KisColorSpaceFactoryRegistry(const KisColorSpaceFactoryRegistry&);
    KisColorSpaceFactoryRegistry operator=(const KisColorSpaceFactoryRegistry&);

private:
    static KisColorSpaceFactoryRegistry *m_singleton;

    QMap<QString, KisProfile * > m_profileMap;
    QMap<QString, KisColorSpace * > m_csMap;
    KisColorSpace *m_xyzCs;
    KisColorSpace *m_alphaCs;
};

#endif // KIS_COLORSPACE_FACTORY_REGISTRY_H_

