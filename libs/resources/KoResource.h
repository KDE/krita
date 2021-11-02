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

#include "KoResourceSignature.h"

#include "KisResourceTypes.h"
#include <boost/operators.hpp>

#include <kritaresources_export.h>

class QDomDocument;
class QDomElement;

class KoResource;
typedef QSharedPointer<KoResource> KoResourceSP;

class KisResourcesInterface;
typedef QSharedPointer<KisResourcesInterface> KisResourcesInterfaceSP;

class KoResourceLoadResult;

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
    KoResource();
    explicit KoResource(const QString &filename);
    virtual ~KoResource();
    KoResource(const KoResource &rhs);
    KoResource &operator=(const KoResource &rhs) = delete;

    virtual KoResourceSP clone() const = 0;



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
     * Requests the resource to update its linked-resources
     * metadata stored in metaData()["dependent_resources_filenames"].
     *
     * This request comes from KisResourceLocator every time a
     * new version of the resource is added to the database.
     */
    virtual void updateLinkedResourcesMetaData(KisResourcesInterfaceSP resourcesInterface);

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

    /**
     * @param generateIfEmpty: if the resource does not have an md5sum set,
     * if this is true, the resource saves itself into a buffer and calculates
     * the md5sum over that. This may be different from the actual md5sum you
     * would get calculated over the actual resource file.
     * @return the md5sum calculated over the contents of the resource. This
     * is in hex-encoded string format.
     */
    QString md5Sum(bool generateIfEmpty = true) const;

    /// Set the md5sum of this resource. It must be in hex-encoded string format
    void setMD5Sum(const QString &md5sum);

    /// @return the filename of this resource within the container (folder, bundle, ...)
    QString filename() const;
    void setFilename(const QString& filename);

    /// @return the user-visible name of the resource
    virtual QString name() const;
    void setName(const QString& name);

    /// @return true if the resource is ready for use
    bool valid() const;
    void setValid(bool valid);

    /// @return true if the resource is marked as active in the database (inactive means "deleted")
    bool active() const;
    void setActive(bool active);

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
     * @return the signature of the resource which is enough for referencing
     * the resource in somewhat unique way
     */
    KoResourceSignature signature() const;

    /**
     * Ephemeral resource is a type of resource that has no physical
     * representation on disk. Therefore, its load()/save() calls do
     * nothing. Such resources will always have empty md5Sum() and,
     * therefore, will never be stored in the resources database.
     *
     * This type of resources is created directly by the corresponding
     * factory or other object (e.g. KisAutoBrushFactory).
     *
     * Ephemeral resource cannot be serializable.
     */
    virtual bool isEphemeral() const;

    /**
     * Serializable resource is the one which can be saved/loaded into a
     * specific storage via saveToDevice()/loadFromDevice() methods. Some
     * resources, like KisAbrBrush or KisPsdLayerStyle, are stored in
     * specific libraries in "batches". Such resources cannot be saved
     * individually. They are created by the corresponding factories.
     *
     * In contrast to ephemeral resource, non-serializable resource
     * will always have a correct md5Sum() and may be stored in the
     * resources database.
     */
    virtual bool isSerializable() const;

    /**
     * Loads all the required resources either from \p globalResourcesInterface or
     * from embedded data. The preset first tries to fetch the required resource
     * from the global source, and only if it fails, tries to load it from the
     * embedded data. One can check if the loaded resource is embedded by checking
     * its resourceId().
     *
     * The set of resources returned is basically: linkedResources() + embeddedResources()
     */
    QList<KoResourceLoadResult> requiredResources(KisResourcesInterfaceSP globalResourcesInterface) const;

    /**
     * @return all the resources that are needed but (*this) resource and
     * are not embedded into it. The resources are fetched from
     * \p globalResourcesInterface. If fetching of some resources is failed,
     * then (*this) resource is invalid.
     */
    virtual QList<KoResourceLoadResult> linkedResources(KisResourcesInterfaceSP globalResourcesInterface) const;

    /**
     * @return all the resources that were embedded into (*this) resource.
     * If the resources were already added to the global database, then they
     * are fetched from \p globalResourcesInterface to save time/memory.
     */
    virtual QList<KoResourceLoadResult> embeddedResources(KisResourcesInterfaceSP globalResourcesInterface) const;

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
    return (resource1.md5Sum() == resource2.md5Sum());
}

static inline uint qHash(const KoResource &resource)
{
    return qHash(resource.md5Sum());
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
                      << " MD5: " << res->md5Sum(false)
                      << " Type: " << res->resourceType()
                      << " Valid: " << res->valid()
                      << " Storage: " << res->storageLocation();
    }
    return dbg.space();
}

#endif // KORESOURCE_H_

