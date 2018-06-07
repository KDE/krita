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

#ifndef KISRESOURCELOCATOR_H
#define KISRESOURCELOCATOR_H

#include <QScopedPointer>
#include <QStringList>
#include <QString>

#include <kritaresources_export.h>


/**
 * The KisResourceLocator class is used to find resources of
 * a certain type.
 */
class KRITARESOURCES_EXPORT KisResourceLocator
{
public:

    static const QString resourceLocationKey;
    static const QStringList resourceTypeFolders;

    KisResourceLocator();
    ~KisResourceLocator();

    enum class LocatorError {
        Ok,
        LocationReadOnly,
        CannotCreateLocation
    };

    /**
     * @brief initialize Setup the resource locator for use.
     *
     * @param installationResourcesLocation the place where the resources
     * that come packaged with Krita reside.
     */
    LocatorError initialize(const QString &installationResourcesLocation);

    QString lastErrorMessage() const;

private:

    LocatorError firstTimeInstallation(const QString &installationResourcesLocation);

    class Private;
    QScopedPointer<Private> d;
};

#endif // KISRESOURCELOCATOR_H
