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

#define KOFFICE_VERSION_STRING "2.3 Alpha 1"
#define KOFFICE_VERSION_MAJOR 2
#define KOFFICE_VERSION_MINOR 2
#define KOFFICE_VERSION_RELEASE 71
#define KOFFICE_MAKE_VERSION( a,b,c ) (((a) << 16) | ((b) << 8) | (c))

#define KOFFICE_ALPHA 1
// #define KOFFICE_BETA 2
// #define KOFFICE_RC 1


#define KOFFICE_VERSION \
    KOFFICE_MAKE_VERSION(KOFFICE_VERSION_MAJOR,KOFFICE_VERSION_MINOR,KOFFICE_VERSION_RELEASE)

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
inline unsigned int version()
{
    return KOFFICE_VERSION;
}

/**
 * Returns the major number of KOffice's version, e.g.
 * 1 for KOffice 1.2.3.
 * @return the major version number
 */
inline unsigned int versionMajor()
{
    return KOFFICE_VERSION_MAJOR;
}

/**
 * Returns the minor number of KOffice's version, e.g.
 * 2 for KOffice 1.2.3.
 * @return the minor version number
 */
inline unsigned int versionMinor() {
    return KOFFICE_VERSION_MINOR;
}

/**
 * Returns the release of KOffice's version, e.g.
 * 3 for KOffice 1.2.3.
 * @return the release number
 */
inline unsigned int versionRelease() {
    return KOFFICE_VERSION_RELEASE;
}

/**
 * Returns the KOffice version as string, e.g. "1.2.3".
 * @return the KOffice version. You can keep the string forever
 */
inline const char *versionString() {
    return KOFFICE_VERSION_STRING;
}

}

#endif // _KOFFICE_VERSION_H_
