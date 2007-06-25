/*
 * Copyright (C) 2007 Adrian Page <adrian@pagenet.plus.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef KOLCMSRGBCOLORPROFILE_H
#define KOLCMSRGBCOLORPROFILE_H

#include "KoLcmsColorProfile.h"

/**
 * A class for creating an lcms RGB profile from a set of chromaticities.
 */
class PIGMENT_EXPORT KoLcmsRGBColorProfile : public KoLcmsColorProfile {

public:
    /**
     * The chromaticities of the primaries and whitepoint of an RGB profile.
     */
    struct Chromaticities {
        cmsCIExyYTRIPLE primaries;
        cmsCIExyY whitePoint;
    };

    /**
     * Create an RGB profile for the colorspace defined by a set of chromaticities.
     *
     * @param chromaticities the chromaticities of the primaries and whitepoint
     * @param gamma gamma value for the red, green and blue transfer functions
     */
    KoLcmsRGBColorProfile(const Chromaticities &chromaticities, double gamma);

    /**
     * Extract the chromaticities of the primaries and whitepoint of an RGB 
     * profile.
     * 
     * @param profile handle of the lcms RGB profile
     */
    static Chromaticities chromaticitiesFromProfile(cmsHPROFILE profile);

    /**
     * Extract the chromaticities of the primaries and whitepoint of an RGB 
     * profile.
     * 
     * @param profile the RGB profile
     */
    static Chromaticities chromaticitiesFromProfile(KoLcmsRGBColorProfile *profile);
};

#endif // KOLCMSRGBCOLORPROFILE_H

