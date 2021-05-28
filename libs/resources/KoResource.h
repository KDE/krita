/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2003 Patrick Julien <freak@codepimps.org>
    SPDX-FileCopyrightText: 2005 Boudewijn Rempt <boud@valdyas.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
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

namespace ResourceTestHelper {
void overrideResourceVesion(KoResourceSP resource, int version);
}

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
    bool load(KisResourcesInterfaceSP resourcesInterface);
    virtual bool loadFromDevice(QIODevice *dev, KisResourcesInterfaceSP resourcesInterface) = 0;

    /**
     * Save this resource.
     *@return true if saving the resource succeeded.
     */
    bool save();
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

    /**
     * @brief thumbnailPath returns the path to a separate thumbnail image, outside
     *        the actual resource file itself. If the path is relative, it is supposed
     *        start in the same location as the resource itself. If it's absolute,
     *        that is, it starts with "/", it is from the root of the storage.
     * @return an empty string if the thumbnail is part of the resource
     */
    virtual QString thumbnailPath() const;

    /// @return the md5sum calculated over the contents of the resource.
    QByteArray md5() const;
    void setMD5(const QByteArray &md5);

    /// @return the filename of this resource within the container (folder, bundle, ...)
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
    void setStorageLocation(const QString &location);

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
    void setVersion(int version);

    /// @return the unique id of the resource in the resource database
    int resourceId() const;
    void setResourceId(int id);

    /// @return the resource type
    virtual QPair<QString, QString> resourceType() const = 0;

    /**
     * Loads all the required resources either from \p globalResourcesInterface or
     * from embedded data. The preset first tries to fetch the required resource
     * from the global source, and only if it fails, tries to load it from the
     * embedded data. One can check if the loaded resource is embedded by checking
     * its resourceId().
     *
     * The set of resources returned is basically: linkedResources() + embeddedResources()
     */
    QList<KoResourceSP> requiredResources(KisResourcesInterfaceSP globalResourcesInterface) const;

    /**
     * @return all the resources that are needed but (*this) resource and
     * are not embedded into it. The resources are fetched from
     * \p globalResourcesInterface. If fetching of some resources is failed,
     * then (*this) resource is invalid.
     */
    virtual QList<KoResourceSP> linkedResources(KisResourcesInterfaceSP globalResourcesInterface) const;

    /**
     * @return all the resources that were embedded into (*this) resource.
     * If the resources were already added to the global database, then they
     * are fetched from \p globalResourcesInterface to save time/memory.
     */
    virtual QList<KoResourceSP> embeddedResources(KisResourcesInterfaceSP globalResourcesInterface) const;

    /**
     * A list of per-canvas active resources that are needed for this resource
     * to function properly. E.g. some gradients may require Fg/Bg colors and
     * some presets would like to know about the current gradient selection.
     *
     * @return a list of canvas resources needed for the current resource
     */
    virtual QList<int> requiredCanvasResources() const;

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

