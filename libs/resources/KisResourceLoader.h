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

#ifndef KISRESOURCELOADER_H
#define KISRESOURCELOADER_H

#include <QString>
#include <QStringList>
#include <QImage>
#include <QScopedPointer>
#include <QSharedPointer>

#include <KoResource.h>

#include <kritaresources_export.h>


/**
 * @brief The KisResourceLoader class is an abstract interface
 * class that must be implemented by actual resource classes and
 * registered with the KisResourceLoaderRegistry.
 */
class KRITARESOURCES_EXPORT KisResourceLoader
{
public:

    KisResourceLoader(const QString &folder, const QStringList &mimetypes);
    virtual ~KisResourceLoader();

    /**
     * @return the mimetypes this resource can load
     */
    QStringList mimetypes() const;

    /**
     * @return the folder in the resource storage where resources
     * of this type are located
     */
    QString folder() const;


    /// For registration in KisResourceLoaderRegistry
    virtual QString id() const = 0;

    /**
     * Load this resource.
     * @return true if loading the resource succeeded.
     */
    virtual bool load(QIODevice &dev) = 0;

    /**
     * Save this resource.
     *@return true if saving the resource succeeded.
     */
    virtual bool save(QIODevice &dev) const = 0;

    QImage thumbnail() const;

    QString name() const;

    KoResourceSP resource() const;

protected:

    void setType(const QString &type);

    void setThumbnail(const QImage &thumbnail);

    void setName(const QString &name);

    void setResource(KoResourceSP resource);

private:

    class Private;
    QScopedPointer<Private> d;
};

typedef QSharedPointer<KisResourceLoader> KisResourceLoaderSP;

#endif // KISRESOURCELOADER_H
