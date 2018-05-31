/*
 * Copyright (C) 2018 Boudewijn Rempt <boud@valdyas.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KISRESOURCECACHEDB_H
#define KISRESOURCECACHEDB_H

#include <QObject>
#include <QScopedPointer>

#include <kritaresources_export.h>


/**
 * @brief The KisResourceCacheDb class encapsulates the database that
 * caches information about the resources available to the user.
 *
 * KisApplication creates and initializes the database. All other methods
 * are static and can be used from anywhere.
 */
class KRITARESOURCES_EXPORT KisResourceCacheDb
{
public:

    static const QString dbLocationKey;
    static const QString ResourceCacheDbFilename;
    static const QString databaseVersion;

    /**
     * @brief KisResourceCacheDb create a resource cache database.
     */
    explicit KisResourceCacheDb();
    ~KisResourceCacheDb();

    /**
     * @brief isValid
     * @return true if the database has been correctly created, false if the database cannot be used
     */
    bool isValid() const;

    /**
     * @brief initialize
     * @param location the location of the database
     * @return true if the database has been initialized correctly
     */
    bool initialize(const QString &location) const;

private:

    friend class TestResourceCacheDb;
    static const QStringList resourceTypes;
    static const QStringList originTypes;

    class Private;
    QScopedPointer<Private> d;

};

#endif // KISRESOURCECACHEDB_H
