/* This file is part of the KDE project
   Copyright (C) 2005 David Faure <faure@kde.org>
   Copyright (C) 2007 Thorsten Zachmann <zachmann@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/
#ifndef KOODFREADSTORE_H
#define KOODFREADSTORE_H

class QString;
class QIODevice;
class KoStore;
class KoOasisStyles;
class KoXmlDocument;

/**
 * Helper class around KoStore for reading out ODF files.
 *
 * The class loades and parses files from the KoStore.
 *
 * @author: David Faure <faure@kde.org>
 */
#include <komain_export.h>

class KOMAIN_EXPORT KoOdfReadStore
{
public:
    /// @param store recontents the property of the caller
    explicit KoOdfReadStore( KoStore* store );

    ~KoOdfReadStore();

    KoStore* store() const;
    KoOasisStyles & styles();
    const KoXmlDocument & contentDoc() const;
    const KoXmlDocument & settingsDoc() const;

    bool loadAndParse( QString & errorMessage );

    /** 
     * Load a file from an odf store
     */
    bool loadAndParse( const QString& fileName, KoXmlDocument& doc, QString& errorMessage );

    /** 
     * Load a file and parse from a QIODevice
     * filename argument is just used for debug message
     */
    static bool loadAndParse( QIODevice* fileDevice, KoXmlDocument& doc, QString& errorMessage, const QString& fileName );

    /**
     * Get mimetype from full path, using the manifest
     */
    static QString mimeForPath( const KoXmlDocument& doc, const QString& fullPath );

private:
    struct Private;
    Private * const d;
};

#endif /* KOODFREADSTORE_H */
