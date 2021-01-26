/*
 * SPDX-FileCopyrightText: 2018 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KISRESOURCESTORAGE_H
#define KISRESOURCESTORAGE_H

#include <QSharedPointer>
#include <QScopedPointer>
#include <QString>
#include <QDateTime>
#include <QMap>

#include <KoResource.h>
#include <KisTag.h>

#include <klocalizedstring.h>

#include <kritaresources_export.h>

class KisStoragePlugin;

class KisStoragePluginFactoryBase
{
public:
    virtual ~KisStoragePluginFactoryBase(){}
    virtual KisStoragePlugin *create(const QString &/*location*/) { return 0; }
};

template<typename T>
class KisStoragePluginFactory : public KisStoragePluginFactoryBase
{
public:
    KisStoragePlugin *create(const QString &location) override {
        return new T(location);
    }
};

class KisResourceStorage;
typedef QSharedPointer<KisResourceStorage> KisResourceStorageSP;


/**
 * The KisResourceStorage class is the base class for
 * places where resources can be stored. Examples are
 * folders, bundles or Adobe resource libraries like
 * ABR files.
 */
class KRITARESOURCES_EXPORT KisResourceStorage
{
public:

    /// A resource item is simply an entry in the storage,
    struct ResourceItem {

        virtual ~ResourceItem() {}
        QString url;
        QString folder;
        QString resourceType;
        QDateTime lastModified;
    };

    class TagIterator
    {
    public:
        virtual ~TagIterator() {}
        virtual bool hasNext() const = 0;
        /// The iterator is only valid if next() has been called at least once.
        virtual void next() = 0;

        /// The untranslated name of the tag, to be used for making connections to resources
        virtual QString url() const = 0;
        /// The translated name of the tag, to be shown in the GUI
        virtual QString name() const = 0;
        /// An extra, optional comment for the tag
        virtual QString comment() const = 0;
        /// The resource type as defined in the tag file
        virtual QString resourceType() const = 0;


        /// A tag object on which we can set properties and which we can save
        virtual KisTagSP tag() const = 0;
    };

    class ResourceIterator
    {
    public:

        virtual ~ResourceIterator() {}

        virtual bool hasNext() const = 0;
        /// The iterator is only valid if next() has been called at least once.
        virtual void next() = 0;

        virtual QString url() const = 0;
        virtual QString type() const = 0;
        virtual QDateTime lastModified() const = 0;
        /// This only loads the resource when called
        virtual KoResourceSP resource() const = 0;
    };

    enum class StorageType : int {
        Unknown = 1,
        Folder = 2,
        Bundle = 3,
        AdobeBrushLibrary = 4,
        AdobeStyleLibrary = 5,
        Memory = 6
    };

    static QString storageTypeToString(StorageType storageType) {
        switch (storageType) {
        case StorageType::Unknown:
            return i18n("Unknown");
        case StorageType::Folder:
            return i18n("Folder");
        case StorageType::Bundle:
            return i18n("Bundle");
        case StorageType::AdobeBrushLibrary:
            return i18n("Adobe Brush Library");
        case StorageType::AdobeStyleLibrary:
            return i18n("Adobe Style Library");
        case StorageType::Memory:
            return i18n("Memory");
        default:
            return i18n("Invalid");
        }
    }


    static QString storageTypeToUntranslatedString(StorageType storageType) {
        switch (storageType) {
        case StorageType::Unknown:
            return ("Unknown");
        case StorageType::Folder:
            return ("Folder");
        case StorageType::Bundle:
            return ("Bundle");
        case StorageType::AdobeBrushLibrary:
            return ("Adobe Brush Library");
        case StorageType::AdobeStyleLibrary:
            return ("Adobe Style Library");
        case StorageType::Memory:
            return ("Memory");
        default:
            return ("Invalid");
        }
    }


    KisResourceStorage(const QString &location);
    ~KisResourceStorage();
    KisResourceStorage(const KisResourceStorage &rhs);
    KisResourceStorage &operator=(const KisResourceStorage &rhs);
    KisResourceStorageSP clone() const;

    /// The filename of the storage if it's a bundle or Adobe Library. This can
    /// also be empty (for the folder storage) or "memory" for the storage for
    /// temporary resources, a UUID for storages associated with documents.
    QString name() const;

    /// The absolute location of the storage
    QString location() const;

    /// true if the storage exists and can be used
    bool valid() const;

    /// The type of the storage
    StorageType type() const;

    /// The icond for the storage
    QImage thumbnail() const;

    /// The time and date when the storage was last modified, or created
    /// for memory storages.
    QDateTime timestamp() const;

    /// The time and date when the resource was last modified
    /// For filestorage
    QDateTime timeStampForResource(const QString &resourceType, const QString &filename) const;

    /// And entry in the storage; this is not the loaded resource
    ResourceItem resourceItem(const QString &url);

    /// The loaded resource for an entry in the storage
    KoResourceSP resource(const QString &url);

    /// An iterator over all the resources in the storage
    QSharedPointer<ResourceIterator> resources(const QString &resourceType) const;

    /// An iterator over all the tags in the resource
    QSharedPointer<TagIterator> tags(const QString &resourceType) const;

    /// Adds a tag to the storage, however, it does not store the links between
    /// tags and resources.
    bool addTag(const QString &resourceType, KisTagSP tag);

    /// Adds the given resource to the storage.
    bool addResource(KoResourceSP resource);

    static const QString s_meta_generator;
    static const QString s_meta_author;
    static const QString s_meta_title;
    static const QString s_meta_description;
    static const QString s_meta_initial_creator;
    static const QString s_meta_creator;
    static const QString s_meta_creation_date;
    static const QString s_meta_dc_date;
    static const QString s_meta_user_defined;
    static const QString s_meta_name;
    static const QString s_meta_value;
    static const QString s_meta_version;
    static const QString s_meta_license;
    static const QString s_meta_email;

    void setMetaData(const QString &key, const QVariant &value);
    QStringList metaDataKeys() const;
    QVariant metaData(const QString &key) const;

private:

    friend class KisStorageModel;
    friend class KisResourceLocator;
    friend class KisResourceCacheDb;

    void setStorageId(int storageId);
    int storageId();

    class Private;
    QScopedPointer<Private> d;
};



inline QDebug operator<<(QDebug dbg, const KisResourceStorageSP storage)
{
    if (storage.isNull()) {
        dbg.nospace() << "[RESOURCESTORAGE] NULL";
    }
    else {
        dbg.nospace() << "[RESOURCESTORAGE] Name: " << storage->name()
                      << " Version: " << storage->location()
                      << " Valid: " << storage->valid()
                      << " Storage: " << KisResourceStorage::storageTypeToString(storage->type())
                      << " Timestamp: " << storage->timestamp();
    }
    return dbg.space();
}

class KRITARESOURCES_EXPORT KisStoragePluginRegistry {
public:
    KisStoragePluginRegistry();
    void addStoragePluginFactory(KisResourceStorage::StorageType storageType, KisStoragePluginFactoryBase *factory);
    static KisStoragePluginRegistry *instance();
private:
    friend class KisResourceStorage;
    QMap<KisResourceStorage::StorageType, KisStoragePluginFactoryBase*> m_storageFactoryMap;

};

class KisStorageVersioningHelper {
public:
    static bool addVersionedResource(const QString &filename, const QString &saveLocation, KoResourceSP resource);

};


#endif // KISRESOURCESTORAGE_H
