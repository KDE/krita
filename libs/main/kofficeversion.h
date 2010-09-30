/* This file is part of the KOffice libraries
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

#ifndef _KOFFICE_VERSION_H_
#define _KOFFICE_VERSION_H_

#include "komain_export.h"

/**
* @def KOFFICE_VERSION_STRING
* @ingroup KOfficeMacros
* @brief Version of KOffice as string, at compile time
*
* This macro contains the KOffice version in string form. As it is a macro,
* it contains the version at compile time. See KOffice::versionString() if you need
* the KOffice version used at runtime.
*/
#define KOFFICE_VERSION_STRING "2.3 Beta 1"

/**
 * @def KOFFICE_VERSION_MAJOR
 * @ingroup KOfficeMacros
 * @brief Major version of KOffice, at compile time
*/
#define KOFFICE_VERSION_MAJOR 2

/**
 * @def KOFFICE_VERSION_MINOR
 * @ingroup KOfficeMacros
 * @brief Minor version of KOffice, at compile time
*/
#define KOFFICE_VERSION_MINOR 2

/**
 * @def KOFFICE_VERSION_RELEASE
 * @ingroup KOfficeMacros
 * @brief Release version of KOffice, at compile time
*/
#define KOFFICE_VERSION_RELEASE 81

/**
 * @ingroup KOfficeMacros
 * @brief Make a number from the major, minor and release number of a KOffice version
 *
 * This function can be used for preprocessing when KOFFICE_IS_VERSION is not
 * appropriate.
 */ 
#define KOFFICE_MAKE_VERSION( a,b,c ) (((a) << 16) | ((b) << 8) | (c))

/**
 * @def KOFFICE_ALPHA
 * @ingroup KOfficeMacros
 * @brief If defined, indicates at compile time that KOffice is in alpha stage
 */
// #define KOFFICE_ALPHA 1

/**
 * @def KOFFICE_BETA
 * @ingroup KOfficeMacros
 * @brief If defined, indicates at compile time that KOffice is in beta stage
 */
#define KOFFICE_BETA 1

/**
 * @def KOFFICE_RC
 * @ingroup KOfficeMacros
 * @brief If defined, indicates at compile time that KOffice is in "release candidate" stage
 */
// #define KOFFICE_RC 1

/**
 * @ingroup KOfficeMacros
 * @brief Version of KOffice as number, at compile time
 *
 * This macro contains the KOffice version in number form. As it is a macro,
 * it contains the version at compile time. See version() if you need
 * the KOffice version used at runtime.
 */
#define KOFFICE_VERSION \
    KOFFICE_MAKE_VERSION(KOFFICE_VERSION_MAJOR,KOFFICE_VERSION_MINOR,KOFFICE_VERSION_RELEASE)

/**
 * @ingroup KOfficeMacros
 * @brief Check if the KOffice version matches a certain version or is higher
 *
 * This macro is typically used to compile conditionally a part of code:
 * @code
 * #if KOFFICE_IS_VERSION(2,3,0)
 * // Code for KOffice 2.3.0
 * #else
 * // Code for older KOffice
 * #endif
 * @endcode
 *
 * @warning Especially during development phases of KOffice, be careful
 * when choosing the version number that you are checking against.
 * Otherwise you might risk to break the next KOffice release.
 * Therefore be careful that development version have a
 * version number lower than the released version, so do not check
 * e.g. for KOffice 4.3 with KOFFICE_IS_VERSION(4,3,0)
 * but with the actual version number at a time a needed feature was introduced, e.g. 4.3.2.
 */
#define KOFFICE_IS_VERSION(a,b,c) ( KOFFICE_VERSION >= KOFFICE_MAKE_VERSION(a,b,c) )

/**
 * Namespace for general KOffice functions.
 */
namespace KOffice
{
/**
 * Returns the encoded number of KOffice's version, see the KOFFICE_VERSION macro.
 * In contrary to that macro this function returns the number of the actually
 * installed KOffice version, not the number of the KOffice version that was
 * installed when the program was compiled.
 * @return the version number, encoded in a single uint
 */
KOMAIN_EXPORT unsigned int version();

/**
 * Returns the major number of KOffice's version, e.g.
 * 1 for KOffice 1.2.3.
 * @return the major version number
 */
KOMAIN_EXPORT unsigned int versionMajor();

/**
 * Returns the minor number of KOffice's version, e.g.
 * 2 for KOffice 1.2.3.
 * @return the minor version number
 */
KOMAIN_EXPORT unsigned int versionMinor();

/**
 * Returns the release of KOffice's version, e.g.
 * 3 for KOffice 1.2.3.
 * @return the release number
 */
KOMAIN_EXPORT unsigned int versionRelease();

/**
 * Returns the KOffice version as string, e.g. "1.2.3".
 * @return the KOffice version. You can keep the string forever.
 */
KOMAIN_EXPORT const char *versionString();
}

#endif // _KOFFICE_VERSION_H_
