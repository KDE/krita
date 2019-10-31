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

#ifndef KISRESOURCESTORAGE_H
#define KISRESOURCESTORAGE_H

#include <QSharedPointer>
#include <QScopedPointer>
#include <QString>
#include <QDateTime>
#include <QMap>

#include <KoResource.h>
#include <KisTag.h>

#include <kritaresources_export.h>

class KisStoragePlugin;

class KRITARESOURCES_EXPORT KisStoragePluginFactoryBase
{
public:
    virtual ~KisStoragePluginFactoryBase(){}
    virtual KisStoragePlugin *create(const QString &/*location*/) { return 0; }
};

template<typename T>
class KRITARESOURCES_EXPORT KisStoragePluginFactory : public KisStoragePluginFactoryBase
{
public:
    KisStoragePlugin *create(const QString &location) override {
        return new T(location);
    }
};


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
        virtual void next() const = 0;

        /// The untranslated name of the tag, to be used for making connections to resources
        virtual QString url() const = 0;
        /// The translated name of the tag, to be shown in the GUI
        virtual QString name() const = 0;
        /// An extra, optional comment for the tag
        virtual QString comment() const = 0;

        /// A tag object on which we can set properties and which we can save
        virtual KisTagSP tag() const = 0;
    };

    class ResourceIterator
    {
    public:

        virtual ~ResourceIterator() {}

        virtual bool hasNext() const = 0;
        /// The iterator is only valid if next() has been called at least once.
        virtual void next() const = 0;

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
            return "Unknown";
        case StorageType::Folder:
            return "Folder";
        case StorageType::Bundle:
            return "Bundle";
        case StorageType::AdobeBrushLibrary:
            return "Adobe Brush Library";
        case StorageType::AdobeStyleLibrary:
            return "Adobe Style Library";
        case StorageType::Memory:
            return "Memory";
        default:
            return "Invalid";
        }
    }


    KisResourceStorage(const QString &location);
    ~KisResourceStorage();

    QString name() const;
    QString location() const;
    bool valid() const;
    StorageType type() const;
    QDateTime timestamp() const;

    ResourceItem resourceItem(const QString &url);
    KoResourceSP resource(const QString &url);
    QSharedPointer<ResourceIterator> resources(const QString &resourceType) const;
    QSharedPointer<TagIterator> tags(const QString &resourceType) const;

    bool addTag(const QString &resourceType, KisTagSP tag);
    bool addResource(const QString &resourceType, KoResourceSP resource);

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

    QStringList metaDataKeys() const;
    QVariant metaData(const QString &key) const;

private:

    class Private;
    QScopedPointer<Private> d;
};


typedef QSharedPointer<KisResourceStorage> KisResourceStorageSP;

inline QDebug operator<<(QDebug dbg, const KisResourceStorageSP storage)
{
    dbg.nospace() << "[RESOURCESTORAGE] Name: " << storage->name()
                  << " Version: " << storage->location()
                  << " Valid: " << storage->valid()
                  << " Storage: " << KisResourceStorage::storageTypeToString(storage->type())
                  << " Timestamp: " << storage->timestamp();

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



#endif // KISRESOURCESTORAGE_H
