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

#ifndef KIS_COLORSPACE_REGISTRY_H_
#define KIS_COLORSPACE_REGISTRY_H_

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
class KRITACORE_EXPORT KisColorSpaceRegistry : public KisGenericRegistry<KisColorSpace *> {

public:
    virtual ~KisColorSpaceRegistry();

    /**
     * Get the singleton instance of this registry
     */
    static KisColorSpaceRegistry* instance();

    /**
     * Convenience method to get the often used rgb8 colorspace
     */
    static KisColorSpace * getRGB8();

    /**
     * Convenience method to get the often used alpha colorspace
     */
    static KisColorSpace * getAlpha8();

    /**
     * Convenience method to get the often used xyz16 colorspace
     */
    static KisColorSpace * getXYZ16();

    /**
     * Reload the profiles from disk
     */
    void resetProfiles();

    /**
     * Return the profile associated with the given product name,
     * or 0.
     */
    KisProfile *  getProfileByName(const QString & name);

    /**
     * Return the vector of profiles for this colorspace
     */
    QValueVector<KisProfile *>  profilesFor(KisColorSpace * cs);


    /**
     * Add a new pixel op to the relevant color strategy.
     * @param pixelop the pixel operation
     */
    void addFallbackPixelOp(KisPixelOp * pixelop);

    /**
     * Return a pixel from the list of default pixelops. If
     * the specified pixelop doesn't exist, then 0 will be
     * returned.
     */
    KisPixelOp * getFallbackPixelOp(KisID pixelop);

private:
    KisColorSpaceRegistry();
    KisColorSpaceRegistry(const KisColorSpaceRegistry&);
    KisColorSpaceRegistry operator=(const KisColorSpaceRegistry&);

private:
    static KisColorSpaceRegistry *m_singleton;
    static KisColorSpace * m_rgb;
    static KisColorSpace * m_alpha;
    static KisColorSpace * m_xyz;

    QMap<QString, KisProfile * > m_profileMap;
    QMap<KisID, KisPixelOp*> m_defaultPixelOps;
};

#endif // KIS_COLORSPACE_REGISTRY_H_

