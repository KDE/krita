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
#include <QDebug>

#include "KisResourceTypes.h"
#include <boost/operators.hpp>

#include <kritaresources_export.h>

class QDomDocument;
class QDomElement;

class KoResource;
typedef QSharedPointer<KoResource> KoResourceSP;

class KisResourcesInterface;
typedef QSharedPointer<KisResourcesInterface> KisResourcesInterfaceSP;

/**
 * The KoResource class provides a representation of resources.  This
 * includes, but not limited to, brushes and patterns.
 *
 * A resource knows its filename, but not the location where it's stored.
 * A new version of a resource is stored with an updated filename, the old
 * version isn't renamed.
 *
 */
class KRITARESOURCES_EXPORT KoResource : public boost::equality_comparable<KoResource>
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
    KoResource(const KoResource &rhs);
    KoResource &operator=(const KoResource &rhs) = delete;

    virtual KoResourceSP clone() const = 0;

    bool operator==(const KoResource &other) const;

public:
    /**
     * Load this resource.
     * @return true if loading the resource succeeded.
     */
    virtual bool load(KisResourcesInterfaceSP resourcesInterface);
    virtual bool loadFromDevice(QIODevice *dev, KisResourcesInterfaceSP resourcesInterface) = 0;

    /**
     * Save this resource.
     *@return true if saving the resource succeeded.
     */
    virtual bool save();
    virtual bool saveToDevice(QIODevice* dev) const;

    /**
     * @returns a QImage image representing this resource: in the case
     * of some resources, it is the actual resource.
     *
     * This image could be null. The image can be in any valid format.
     */
    QImage image() const;
    void setImage(const QImage &image);

    /**
     * @brief updateThumbnail updates the thumbnail for this resource.
     * Reimplement if your thumbnail is something else than the image
     * set with setImage.
     */
    virtual void updateThumbnail();

    /**
     * @brief thumbnail the thumbnail image to use in resource selectors
     * @return a valid qimage. All thumbnails for a given resource have the
     * same size (which is not true for image(), but that size need not
     * be square. By default it's the same as image(), but that is not guaranteed.
     */
    virtual QImage thumbnail() const;

    /// @return the md5sum calculated over the contents of the resource.
    QByteArray md5() const;

    /// @return the unique identifier of this resource within the container (folder, bundle, ...)
    QString filename() const;
    void setFilename(const QString& filename);

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

    /// @return the unique id of the resource in the resource database
    int resourceId() const;

    /// @return the resource type
    virtual QPair<QString, QString> resourceType() const = 0;

    /**
     * Loads all the linked resources either from \p globalResourcesInterface or
     * from embedded data. The preset first tries to fetch the linked resource
     * from the global source, and only if it fails, tries to load it from the
     * embedded data. One can check if the loaded resource is embedded by checking
     * its resourceId().
     */
    virtual QList<KoResourceSP> linkedResources(KisResourcesInterfaceSP globalResourcesInterface) const;

private:

    friend class KisResourceModel;
    friend class KisResourceLocator;
    friend class TestResourceModel;
    friend class TestResourceLocator;
    friend class TestFolderStorage;
    friend class KisFolderStorage;
    friend class KisMemoryStorage;

    void setVersion(int version);
    void setResourceId(int id);
    void setStorageLocation(const QString &location);

protected:

    /// override generateMD5 and in your resource subclass
    virtual QByteArray generateMD5() const;

    /// call this when the contents of the resource change so the md5 needs to be recalculated
    void setMD5(const QByteArray &md5);


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

Q_DECLARE_METATYPE(QSharedPointer<KoResource>)

inline QDebug operator<<(QDebug dbg, const KoResourceSP res)
{
    if (!res) {
        dbg.noquote() << "NULL Resource";
    }
    else {
        dbg.nospace() << "[RESOURCE] Name: " << res->name()
                      << " Version: " << res->version()
                      << " Filename: " << res->filename()
                      << " Valid: " << res->valid()
                    << " Storage: " << res->storageLocation();
    }
    return dbg.space();
}

#endif // KORESOURCE_H_

