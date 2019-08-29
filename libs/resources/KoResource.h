/*  This file is part of the KDE project
    Copyright (c) 2003 Patrick Julien <freak@codepimps.org>
    Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */
#ifndef KORESOURCE_H
#define KORESOURCE_H

#include <QImage>
#include <QString>
#include <QHash>
#include <QSharedPointer>

#include "KisResourceTypes.h"

#include <kritaresources_export.h>

class QDomDocument;
class QDomElement;

/**
 * The KoResource class provides a representation of resources.  This
 * includes, but not limited to, brushes and patterns.
 */
class KRITARESOURCES_EXPORT KoResource
{
public:

    /**
     * Creates a new KoResource object using @p filename.  No file is opened
     * in the constructor, you have to call load.
     *
     * @param filename the file name to save and load from.
     */
    explicit KoResource(const QString &filename);
    virtual ~KoResource();

    bool operator ==(const KoResource &other) const
    {
        return other.md5() == md5();
    }

public:
    /**
     * Load this resource.
     * @return true if loading the resource succeeded.
     */
    virtual bool load() = 0;
    virtual bool loadFromDevice(QIODevice *dev) = 0;

    /**
     * Save this resource.
     *@return true if saving the resource succeeded.
     */
    virtual bool save() = 0;
    virtual bool saveToDevice(QIODevice* dev) const;

    /**
     * @returns a QImage thumbnail image representing this resource.
     *
     * This image could be null. The image can be in any valid format.
     */
    QImage image() const;
    void setImage(const QImage &image);

    /// @return the md5sum calculated over the contents of the resource.
    QByteArray md5() const;

    /// @return the full path to this resource
    QString filename() const;
    void setFilename(const QString& filename);

    /// @return the name of the file without the path
    QString shortFilename() const;

    /// @return the user-visible name of the resource
    QString name() const;
    void setName(const QString& name);

    /// @return true if the resource is ready for use
    bool valid() const;
    void setValid(bool valid);

    /// @return the default file extension which should be used when saving the resource
    virtual QString defaultFileExtension() const;

    /// @return true if the resource is permanent and can't be removed by the user
    bool permanent() const;
    void setPermanent(bool permanent);

    /// @return the name of the storage location of the resource
    QString storageLocation() const;

    /// Mark the preset as modified but not saved
    void setDirty(bool value);

    /// @return true if the preset has been modified, but not saved
    bool isDirty() const;

    /// store the given key, value pair in the resource
    void addMetaData(QString key, QVariant value);

    /// get a map with all the metadata
    QMap<QString, QVariant> metadata() const;

    /// Get the version of the resource
    int version() const;

private:

    friend class KisResourceModel;
    friend class KisResourceLocator;
    friend class TestResourceModel;
    friend class TestResourceLocator;
    friend class KisFolderStorage;

    void setVersion(int version);


    /// @return the unique id of the resource in the resource database
    int resourceId() const;
    void setResourceId(int id);


    void setStorageLocation(const QString &location);

protected:

    /// override generateMD5 and in your resource subclass
    virtual QByteArray generateMD5() const;

    /// call this when the contents of the resource change so the md5 needs to be recalculated
    void setMD5(const QByteArray &md5);

protected:
    KoResource(const KoResource &rhs);

private:
    struct Private;
    Private* const d;
};

static inline bool operator==(const KoResource &resource1, const KoResource &resource2)
{
    return (resource1.md5() == resource2.md5());
}

static inline uint qHash(const KoResource &resource)
{
    return qHash(resource.md5());
}

typedef QSharedPointer<KoResource> KoResourceSP;
Q_DECLARE_METATYPE(QSharedPointer<KoResource>)

#endif // KORESOURCE_H_

