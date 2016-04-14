/* This file is part of the Calligra libraries
    Copyright (c) 2003 David Faure <faure@kde.org>
    Copyright (c) 2003 Lukas Tinkl <lukas@kde.org>
    Copyright (c) 2004 Nicolas Goutte <goutte@kde.org>
    Copyright (C) 2015 Jarosław Staniek <staniek@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef _CALLIGRA_VERSION_H_
#define _CALLIGRA_VERSION_H_

#include "kritaversion_export.h"

// -- WARNING: do not edit values below, instead edit CALLIGRA_* in /CMakeLists.txt --

/**
* @def CALLIGRA_VERSION_STRING
* @ingroup CalligraMacros
* @brief Version of Calligra as string, at compile time
*
* This macro contains the Calligra version in string form. As it is a macro,
* it contains the version at compile time. See Calligra::versionString() if you need
* the Calligra version used at runtime.
*
* @note The version string might contain spaces and special characters,
* especially for development versions of Calligra.
* If you use that macro directly for a file format (e.g. OASIS Open Document)
* or for a protocol (e.g. http) be careful that it is appropriate.
* (Fictional) example: "3.0 Alpha"
*/
#define CALLIGRA_VERSION_STRING "@CALLIGRA_VERSION_STRING@"

/**
 * @def CALLIGRA_STABLE_VERSION_MAJOR
 * @ingroup CalligraMacros
 * @brief Major version of stable Calligra, at compile time
 * CALLIGRA_VERSION_MAJOR is computed based on this value.
*/
#define CALLIGRA_STABLE_VERSION_MAJOR @CALLIGRA_STABLE_VERSION_MAJOR@

/**
 * @def CALLIGRA_VERSION_MAJOR
 * @ingroup CalligraMacros
 * @brief Major version of Calligra, at compile time
 *
 * Generally it's the same as CALLIGRA_STABLE_VERSION_MAJOR but for unstable x.0
 * x is decreased by one, e.g. 3.0 Beta is 2.99.
*/
#if !defined CALLIGRA_STABLE && @CALLIGRA_STABLE_VERSION_MINOR@ == 0
# define CALLIGRA_VERSION_MAJOR (CALLIGRA_STABLE_VERSION_MAJOR - 1)
#else
# define CALLIGRA_VERSION_MAJOR CALLIGRA_STABLE_VERSION_MAJOR
#endif

/**
 * @def CALLIGRA_STABLE_VERSION_MINOR
 * @ingroup CalligraMacros
 * @brief Minor version of stable Calligra, at compile time
 * CALLIGRA_VERSION_MINOR is computed based on this value.
 */
#define CALLIGRA_STABLE_VERSION_MINOR @CALLIGRA_STABLE_VERSION_MINOR@

/**
 * @def CALLIGRA_VERSION_MINOR
 * @ingroup CalligraMacros
 * @brief Minor version of Calligra, at compile time
 *
 * Generally it's equal to CALLIGRA_STABLE_VERSION_MINOR for stable releases,
 * equal to 99 for x.0 unstable releases (e.g. it's 3.0 Beta has minor version 99),
 * and equal to CALLIGRA_STABLE_VERSION_MINOR-1 for unstable releases other than x.0.
 */
#ifdef CALLIGRA_STABLE
# define CALLIGRA_VERSION_MINOR CALLIGRA_STABLE_VERSION_MINOR
#elif CALLIGRA_STABLE_VERSION_MINOR == 0
# define CALLIGRA_VERSION_MINOR 99
#else
# define CALLIGRA_VERSION_MINOR (CALLIGRA_STABLE_VERSION_MINOR - 1)
#endif

/**
 * @def CALLIGRA_VERSION_RELEASE
 * @ingroup CalligraMacros
 * @brief Release version of Calligra, at compile time.
 * 89 for Alpha.
 */
#define CALLIGRA_VERSION_RELEASE @CALLIGRA_VERSION_RELEASE@

/**
 * @def CALLIGRA_STABLE_VERSION_RELEASE
 * @ingroup CalligraMacros
 * @brief Release version of Calligra, at compile time.
 *
 * Equal to CALLIGRA_VERSION_RELEASE for stable releases and 0 for unstable ones.
 */
#ifdef CALLIGRA_STABLE
# define CALLIGRA_STABLE_VERSION_RELEASE 0
#else
# define CALLIGRA_STABLE_VERSION_RELEASE @CALLIGRA_VERSION_RELEASE@
#endif

/**
 * @def CALLIGRA_ALPHA
 * @ingroup CalligraMacros
 * @brief If defined (1..9), indicates at compile time that Calligra is in alpha stage
 */
#cmakedefine CALLIGRA_ALPHA @CALLIGRA_ALPHA@

/**
 * @def CALLIGRA_BETA
 * @ingroup CalligraMacros
 * @brief If defined (1..9), indicates at compile time that Calligra is in beta stage
 */
#cmakedefine CALLIGRA_BETA @CALLIGRA_BETA@

/**
 * @def CALLIGRA_RC
 * @ingroup CalligraMacros
 * @brief If defined (1..9), indicates at compile time that Calligra is in "release candidate" stage
 */
#cmakedefine CALLIGRA_RC @CALLIGRA_RC@

/**
 * @def CALLIGRA_STABLE
 * @ingroup CalligraMacros
 * @brief If defined, indicates at compile time that Calligra is in stable stage
 */
#cmakedefine CALLIGRA_STABLE @CALLIGRA_STABLE@

/**
 * @ingroup CalligraMacros
 * @brief Make a number from the major, minor and release number of a Calligra version
 *
 * This function can be used for preprocessing when CALLIGRA_IS_VERSION is not
 * appropriate.
 */
#define CALLIGRA_MAKE_VERSION( a,b,c ) (((a) << 16) | ((b) << 8) | (c))

/**
 * @ingroup CalligraMacros
 * @brief Version of Calligra as number, at compile time
 *
 * This macro contains the Calligra version in number form. As it is a macro,
 * it contains the version at compile time. See version() if you need
 * the Calligra version used at runtime.
 */
#define CALLIGRA_VERSION \
    CALLIGRA_MAKE_VERSION(CALLIGRA_VERSION_MAJOR,CALLIGRA_VERSION_MINOR,CALLIGRA_VERSION_RELEASE)

/**
 * @def CALLIGRA_YEAR
 * @ingroup CalligraMacros
 * @brief Year of the Calligra release, set at compile time
 *
 * This macro is used in "About application" dialog for strings such as "© 2012-..., The Author Team".
*/
#define CALLIGRA_YEAR "@CALLIGRA_YEAR@"


#endif // _CALLIGRA_VERSION_H_
