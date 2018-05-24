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

const QString ResourceCacheDbFilename = "resourcecache.sqlite";

/**
 * @brief The KisResourceCacheDb class encapsulates the database that
 * caches information about the resources available to the user.
 */
class KRITARESOURCES_EXPORT KisResourceCacheDb
{
public:

    enum class ResourceCacheDbStatus {
        OK,
        NoDatabase,
    };

    KisResourceCacheDb();
    ~KisResourceCacheDb();

    /**
     * @brief isValid
     * @return true if the database has been correctly created
     */
    bool isValid() const;

private:

    class Private;
    QScopedPointer<Private> d;

};

#endif // KISRESOURCECACHEDB_H
