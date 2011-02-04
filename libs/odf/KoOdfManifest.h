/* This file is part of the KDE project
   Copyright (C) 2010-2011 Inge Wallin <inge@lysator.liu.se>

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

#ifndef KOODFMANIFEST_H
#define KOODFMANIFEST_H


#include <QtCore/QString>

#include "koodf_export.h"


class KoStore;


// A class that holds a manifest:file-entry.
class KOODF_EXPORT KoOdfManifestEntry
{
public:
    KoOdfManifestEntry(const QString &fp, const QString &mt, const QString &v);
    ~KoOdfManifestEntry();

    QString fullPath() const;
    void setFullPath(const QString &fp);

    QString mediaType() const;
    void setMediaType(const QString &mt);

    QString version() const;
    void setVersion(const QString &v);

private:
    class Private;
    Private * const d;
};


#endif /* KOODFMANIFEST_H */
