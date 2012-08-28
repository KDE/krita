/* This file is part of the KDE project
   Copyright (C) 2003-2006 Jarosław Staniek <staniek@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "global.h"

using namespace KexiDB;

DatabaseVersionInfo::DatabaseVersionInfo()
{
    major = 0;
    minor = 0;
}

DatabaseVersionInfo::DatabaseVersionInfo(uint majorVersion, uint minorVersion)
{
    major = majorVersion;
    minor = minorVersion;
}

//------------------------

ServerVersionInfo::ServerVersionInfo()
{
    major = 0;
    minor = 0;
    release = 0;
}

void ServerVersionInfo::clear()
{
    major = 0;
    minor = 0;
    release = 0;
    string.clear();
}

//------------------------

DatabaseVersionInfo KexiDB::version()
{
    return KEXIDB_VERSION;
}
