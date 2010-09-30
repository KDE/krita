/* This file is part of the KOffice libraries
    Copyright (c) 2003 David Faure <faure@kde.org>

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

#include "kofficeversion.h"

unsigned int KOffice::version()
{
    return KOFFICE_VERSION;
}

unsigned int KOffice::versionMajor()
{
    return KOFFICE_VERSION_MAJOR;
}

unsigned int KOffice::versionMinor()
{
    return KOFFICE_VERSION_MINOR;
}

unsigned int KOffice::versionRelease()
{
    return KOFFICE_VERSION_RELEASE;
}

const char *KOffice::versionString()
{
    return KOFFICE_VERSION_STRING;
}
