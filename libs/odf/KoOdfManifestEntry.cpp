/* This file is part of the KDE project
   Copyright (C) 2011 Inge Wallin <inge@lysator.liu.se>

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
   Boston, MA 02110-1301, USA.
*/


// Own
#include "KoOdfManifestEntry.h"

#include <QString>

class Q_DECL_HIDDEN KoOdfManifestEntry::Private
{
public:
    Private() {};

    QString  fullPath;          // manifest:full-path
    QString  mediaType;         // manifest:media-type
    QString  version;           // manifest:version  (isNull==true if not present)
};


// ----------------------------------------------------------------


KoOdfManifestEntry::KoOdfManifestEntry(const QString &fullPath, const QString &mediaType,
                                       const QString &version)
    : d(new Private())
{
    d->fullPath = fullPath;
    d->mediaType = mediaType;
    d->version = version;
}

KoOdfManifestEntry::KoOdfManifestEntry(const KoOdfManifestEntry &other)
    : d(new Private())
{
    d->fullPath = other.d->fullPath;
    d->mediaType = other.d->mediaType;
    d->version = other.d->version;
}

KoOdfManifestEntry::~KoOdfManifestEntry()
{
    delete d;
}

KoOdfManifestEntry &KoOdfManifestEntry::operator=(const KoOdfManifestEntry &other)
{
    d->fullPath = other.d->fullPath;
    d->mediaType = other.d->mediaType;
    d->version = other.d->version;

    return *this;
}


QString KoOdfManifestEntry::fullPath() const
{
    return d->fullPath;
}

void KoOdfManifestEntry::setFullPath(const QString &fullPath)
{
    d->fullPath = fullPath;
}

QString KoOdfManifestEntry::mediaType() const
{
    return d->mediaType;
}

void KoOdfManifestEntry::setMediaType(const QString &mediaType)
{
    d->mediaType = mediaType;
}

QString KoOdfManifestEntry::version() const
{
    return d->version;
}

void KoOdfManifestEntry::setVersion(const QString &version)
{
    d->version = version;
}

