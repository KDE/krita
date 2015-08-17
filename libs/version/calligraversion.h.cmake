/* This file is part of the Calligra libraries
    Copyright (c) 2003 David Faure <faure@kde.org>
    Copyright (c) 2003 Lukas Tinkl <lukas@kde.org>
    Copyright (c) 2004 Nicolas Goutte <goutte@kde.org>

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

#include "koversion_export.h"

// -- WARNING: do not edit values below, instead edit CALLIGRA_VERSION_*, etc in /CMakeLists.txt --

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
* (Fictional) example: "2.8 Alpha"
*/
#define CALLIGRA_VERSION_STRING "@CALLIGRA_VERSION_STRING@"

/**
 * @def CALLIGRA_VERSION_MAJOR
 * @ingroup CalligraMacros
 * @brief Major version of Calligra, at compile time
*/
#define CALLIGRA_VERSION_MAJOR @CALLIGRA_VERSION_MAJOR@

/**
 * @def CALLIGRA_STABLE_VERSION_MINOR
 * @ingroup CalligraMacros
 * @brief Minor version of stable Calligra, at compile time
 * CALLIGRA_VERSION_MINOR is computed based on this value.
 * Unstable versions subtract 1 from this value, e.g. 2.5 Alpha has minor 4.
*/
#define CALLIGRA_STABLE_VERSION_MINOR @CALLIGRA_STABLE_VERSION_MINOR@

/**
 * @def CALLIGRA_VERSION_RELEASE
 * @ingroup CalligraMacros
 * @brief Release version of Calligra, at compile time.
 * 89 for Alpha.
 */
#define CALLIGRA_VERSION_RELEASE @CALLIGRA_VERSION_RELEASE@

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
 * @def CALLIGRA_VERSION_MINOR
 * @ingroup CalligraMacros
 * @brief Minor version of Calligra, at compile time
*/
#ifdef CALLIGRA_STABLE
# define CALLIGRA_VERSION_MINOR CALLIGRA_STABLE_VERSION_MINOR
#else // Unstable versions subtract 1 from this value, e.g. 2.5 Alpha has minor 4.
# define CALLIGRA_VERSION_MINOR (CALLIGRA_STABLE_VERSION_MINOR - 1)
#endif

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
 * @ingroup CalligraMacros
 * @brief Check if the Calligra version matches a certain version or is higher
 *
 * This macro is typically used to compile conditionally a part of code:
 * @code
 * #if CALLIGRA_IS_VERSION(2,3,0)
 * // Code for Calligra 2.3.0
 * #else
 * // Code for older Calligra
 * #endif
 * @endcode
 *
 * @warning Especially during development phases of Calligra, be careful
 * when choosing the version number that you are checking against.
 * Otherwise you might risk to break the next Calligra release.
 * Therefore be careful that development version have a
 * version number lower than the released version, so do not check
 * e.g. for Calligra 4.3 with CALLIGRA_IS_VERSION(4,3,0)
 * but with the actual version number at a time a needed feature was introduced, e.g. 4.3.2.
 */
#define CALLIGRA_IS_VERSION(a,b,c) ( CALLIGRA_VERSION >= CALLIGRA_MAKE_VERSION(a,b,c) )

/**
 * @def CALLIGRA_YEAR
 * @ingroup CalligraMacros
 * @brief Year of the Calligra release, set at compile time
 *
 * This macro is used in "About application" dialog for strings such as "© 2012-..., The Author Team".
*/
#define CALLIGRA_YEAR "@CALLIGRA_YEAR@"

/**
 * Namespace for general Calligra functions.
 */
namespace Calligra
{
/**
 * Returns the encoded number of Calligra's version, see the CALLIGRA_VERSION macro.
 * In contrary to that macro this function returns the number of the actually
 * installed Calligra version, not the number of the Calligra version that was
 * installed when the program was compiled.
 * @return the version number, encoded in a single uint
 */
KOVERSION_EXPORT unsigned int version();

/**
 * Returns the major number of Calligra's version, e.g.
 * 1 for Calligra 1.2.3.
 * @return the major version number
 */
KOVERSION_EXPORT unsigned int versionMajor();

/**
 * Returns the minor number of Calligra's version, e.g.
 * 2 for Calligra 1.2.3.
 * @return the minor version number
 */
KOVERSION_EXPORT unsigned int versionMinor();

/**
 * Returns the release of Calligra's version, e.g.
 * 3 for Calligra 1.2.3.
 * @return the release number
 */
KOVERSION_EXPORT unsigned int versionRelease();

/**
 * Returns the Calligra version as string, e.g. "1.2.3".
 * @return the Calligra version. You can keep the string forever.
 */
KOVERSION_EXPORT const char *versionString();
}

#endif // _CALLIGRA_VERSION_H_
