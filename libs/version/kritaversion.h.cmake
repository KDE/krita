/* This file is part of the Krita libraries
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

#ifndef _KRITA_VERSION_H_
#define _KRITA_VERSION_H_

#include "kritaversion_export.h"

// -- WARNING: do not edit values below, instead edit KRITA_* in /CMakeLists.txt --

/**
* @def KRITA_VERSION_STRING
* @ingroup KritaMacros
* @brief Version of Krita as string, at compile time
*
* This macro contains the Krita version in string form. As it is a macro,
* it contains the version at compile time.
*
* @note The version string might contain spaces and special characters,
* especially for development versions of Krita.
* If you use that macro directly for a file format (e.g. OASIS Open Document)
* or for a protocol (e.g. http) be careful that it is appropriate.
* (Fictional) example: "3.0 Alpha"
*/
#define KRITA_VERSION_STRING "@KRITA_VERSION_STRING@"

/**
 * @def KRITA_STABLE_VERSION_MAJOR
 * @ingroup KritaMacros
 * @brief Major version of stable Krita, at compile time
 * KRITA_VERSION_MAJOR is computed based on this value.
*/
#define KRITA_STABLE_VERSION_MAJOR @KRITA_STABLE_VERSION_MAJOR@

/**
 * @def KRITA_VERSION_MAJOR
 * @ingroup KritaMacros
 * @brief Major version of Krita, at compile time
 *
 * Generally it's the same as KRITA_STABLE_VERSION_MAJOR but for unstable x.0
 * x is decreased by one, e.g. 3.0 Beta is 2.99.
*/
#if !defined KRITA_STABLE && @KRITA_STABLE_VERSION_MINOR@ == 0
# define KRITA_VERSION_MAJOR (KRITA_STABLE_VERSION_MAJOR - 1)
#else
# define KRITA_VERSION_MAJOR KRITA_STABLE_VERSION_MAJOR
#endif

/**
 * @def KRITA_STABLE_VERSION_MINOR
 * @ingroup KritaMacros
 * @brief Minor version of stable Krita, at compile time
 * KRITA_VERSION_MINOR is computed based on this value.
 */
#define KRITA_STABLE_VERSION_MINOR @KRITA_STABLE_VERSION_MINOR@

/**
 * @def KRITA_VERSION_MINOR
 * @ingroup KritaMacros
 * @brief Minor version of Krita, at compile time
 *
 * Generally it's equal to KRITA_STABLE_VERSION_MINOR for stable releases,
 * equal to 99 for x.0 unstable releases (e.g. it's 3.0 Beta has minor version 99),
 * and equal to KRITA_STABLE_VERSION_MINOR-1 for unstable releases other than x.0.
 */
#ifdef KRITA_STABLE
# define KRITA_VERSION_MINOR KRITA_STABLE_VERSION_MINOR
#elif KRITA_STABLE_VERSION_MINOR == 0
# define KRITA_VERSION_MINOR 99
#else
# define KRITA_VERSION_MINOR (KRITA_STABLE_VERSION_MINOR - 1)
#endif

/**
 * @def KRITA_VERSION_RELEASE
 * @ingroup KritaMacros
 * @brief Release version of Krita, at compile time.
 * 89 for Alpha.
 */
#define KRITA_VERSION_RELEASE @KRITA_VERSION_RELEASE@

/**
 * @def KRITA_STABLE_VERSION_RELEASE
 * @ingroup KritaMacros
 * @brief Release version of Krita, at compile time.
 *
 * Equal to KRITA_VERSION_RELEASE for stable releases and 0 for unstable ones.
 */
#ifdef KRITA_STABLE
# define KRITA_STABLE_VERSION_RELEASE 0
#else
# define KRITA_STABLE_VERSION_RELEASE @KRITA_VERSION_RELEASE@
#endif

/**
 * @def KRITA_ALPHA
 * @ingroup KritaMacros
 * @brief If defined (1..9), indicates at compile time that Krita is in alpha stage
 */
#cmakedefine KRITA_ALPHA @KRITA_ALPHA@

/**
 * @def KRITA_BETA
 * @ingroup KritaMacros
 * @brief If defined (1..9), indicates at compile time that Krita is in beta stage
 */
#cmakedefine KRITA_BETA @KRITA_BETA@

/**
 * @def KRITA_RC
 * @ingroup KritaMacros
 * @brief If defined (1..9), indicates at compile time that Krita is in "release candidate" stage
 */
#cmakedefine KRITA_RC @KRITA_RC@

/**
 * @def KRITA_STABLE
 * @ingroup KritaMacros
 * @brief If defined, indicates at compile time that Krita is in stable stage
 */
#cmakedefine KRITA_STABLE @KRITA_STABLE@

/**
 * @ingroup KritaMacros
 * @brief Make a number from the major, minor and release number of a Krita version
 *
 * This function can be used for preprocessing when KRITA_IS_VERSION is not
 * appropriate.
 */
#define KRITA_MAKE_VERSION( a,b,c ) (((a) << 16) | ((b) << 8) | (c))

/**
 * @ingroup KritaMacros
 * @brief Version of Krita as number, at compile time
 *
 * This macro contains the Krita version in number form. As it is a macro,
 * it contains the version at compile time. See version() if you need
 * the Krita version used at runtime.
 */
#define KRITA_VERSION \
    KRITA_MAKE_VERSION(KRITA_VERSION_MAJOR,KRITA_VERSION_MINOR,KRITA_VERSION_RELEASE)

#endif // _KRITA_VERSION_H_
