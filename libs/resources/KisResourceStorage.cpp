/*
 * SPDX-FileCopyrightText: 2018 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KisResourceStorage.h"

#include <QFileInfo>
#include <QDebug>
#include <QApplication>

#include <quazip.h>

#include "KisFolderStorage.h"
#include "KisBundleStorage.h"
#include "KisMemoryStorage.h"

#include <cmath>
#include <QtMath>
#include "kis_debug.h"

#include <QRegularExpression>
#include <boost/optional.hpp>
#include <kis_pointer_utils.h>

const QString KisResourceStorage::s_meta_generator("meta:generator");
const QString KisResourceStorage::s_meta_author("dc:author");
const QString KisResourceStorage::s_meta_title("dc:title");
const QString KisResourceStorage::s_meta_description("dc:description");
const QString KisResourceStorage::s_meta_initial_creator("meta:initial-creator");
const QString KisResourceStorage::s_meta_creator("cd:creator");
const QString KisResourceStorage::s_meta_creation_date("meta:creation-date");
const QString KisResourceStorage::s_meta_dc_date("meta:dc-date");
const QString KisResourceStorage::s_meta_user_defined("meta:meta-userdefined");
const QString KisResourceStorage::s_meta_name("meta:name");
const QString KisResourceStorage::s_meta_value("meta:value");
const QString KisResourceStorage::s_meta_version("meta:bundle-version");
const QString KisResourceStorage::s_meta_email("meta:email");
const QString KisResourceStorage::s_meta_license("meta:license");
const QString KisResourceStorage::s_meta_website("meta:website");

Q_GLOBAL_STATIC(KisStoragePluginRegistry, s_instance);

KisStoragePluginRegistry::KisStoragePluginRegistry()
{
    m_storageFactoryMap[KisResourceStorage::StorageType::Folder] = new KisStoragePluginFactory<KisFolderStorage>();
    m_storageFactoryMap[KisResourceStorage::StorageType::Memory] = new KisStoragePluginFactory<KisMemoryStorage>();
    m_storageFactoryMap[KisResourceStorage::StorageType::Bundle] = new KisStoragePluginFactory<KisBundleStorage>();
}

KisStoragePluginRegistry::~KisStoragePluginRegistry()
{
    qDeleteAll(m_storageFactoryMap.values());
}

void KisStoragePluginRegistry::addStoragePluginFactory(KisResourceStorage::StorageType storageType, KisStoragePluginFactoryBase *factory)
{
    m_storageFactoryMap[storageType] = factory;
}

KisStoragePluginRegistry *KisStoragePluginRegistry::instance()
{
    return s_instance;
}

class KisResourceStorage::Private {
public:
    QString name;
    QString location;
    bool valid {false};
    KisResourceStorage::StorageType storageType {KisResourceStorage::StorageType::Unknown};
    QSharedPointer<KisStoragePlugin> storagePlugin;
    int storageId {-1};
};

KisResourceStorage::KisResourceStorage(const QString &location)
    : d(new Private())
{
    d->location = location;
    d->name = QFileInfo(d->location).fileName();
    QFileInfo fi(d->location);
    if (fi.isDir()) {
        d->storagePlugin.reset(KisStoragePluginRegistry::instance()->m_storageFactoryMap[StorageType::Folder]->create(location));
        d->storageType = StorageType::Folder;
        d->valid = fi.isWritable();
    }
    else if (d->location.endsWith(".bundle")) {
            d->storagePlugin.reset(KisStoragePluginRegistry::instance()->m_storageFactoryMap[StorageType::Bundle]->create(location));
            d->storageType = StorageType::Bundle;
            // XXX: should we also check whether there's a valid metadata entry? Or is this enough?
            d->valid = (fi.isReadable() && QuaZip(d->location).open(QuaZip::mdUnzip));
    } else if (d->location.endsWith(".abr")) {
            d->storagePlugin.reset(KisStoragePluginRegistry::instance()->m_storageFactoryMap[StorageType::AdobeBrushLibrary]->create(location));
            d->storageType = StorageType::AdobeBrushLibrary;
            d->valid = fi.isReadable();
    } else if (d->location.endsWith(".asl")) {
            d->storagePlugin.reset(KisStoragePluginRegistry::instance()->m_storageFactoryMap[StorageType::AdobeStyleLibrary]->create(location));
            d->storageType = StorageType::AdobeStyleLibrary;
            d->valid = fi.isReadable();
    }
    else if (!d->location.isEmpty()) {
        d->storagePlugin.reset(KisStoragePluginRegistry::instance()->m_storageFactoryMap[StorageType::Memory]->create(location));
        d->name = location;
        d->storageType = StorageType::Memory;
        d->valid = true;
    }
    else {
        d->valid = false;
    }
}

KisResourceStorage::~KisResourceStorage()
{
}

KisResourceStorage::KisResourceStorage(const KisResourceStorage &rhs)
    : d(new Private)
{
    *this = rhs;
}

KisResourceStorage &KisResourceStorage::operator=(const KisResourceStorage &rhs)
{
    if (this != &rhs) {
        d->name = rhs.d->name;
        d->location = rhs.d->location;
        d->storageType = rhs.d->storageType;
        if (d->storageType == StorageType::Memory) {
            d->storagePlugin = QSharedPointer<KisMemoryStorage>(new KisMemoryStorage(*dynamic_cast<KisMemoryStorage*>(rhs.d->storagePlugin.data())));
        }
        else {
            d->storagePlugin = rhs.d->storagePlugin;
        }
        d->valid = false;
    }
    return *this;
}

KisResourceStorageSP KisResourceStorage::clone() const
{
    return KisResourceStorageSP(new KisResourceStorage(*this));
}

QString KisResourceStorage::name() const
{
    return d->name;
}

QString KisResourceStorage::location() const
{
    return d->location;
}

KisResourceStorage::StorageType KisResourceStorage::type() const
{
    return d->storageType;
}

QImage KisResourceStorage::thumbnail() const
{
    return d->storagePlugin->thumbnail();
}

QDateTime KisResourceStorage::timestamp() const
{
    return d->storagePlugin->timestamp();
}

QDateTime KisResourceStorage::timeStampForResource(const QString &resourceType, const QString &filename) const
{
    QFileInfo li(d->location);
    if (li.suffix().toLower() == "bundle") {
        QFileInfo bf(d->location + "_modified/" + resourceType + "/" + filename);
        if (bf.exists()) {
            return bf.lastModified();
        }
    } else if (QFileInfo(d->location + "/" + resourceType + "/" + filename).exists()) {
        return QFileInfo(d->location + "/" + resourceType + "/" + filename).lastModified();
    }
    return this->timestamp();
}

KisResourceStorage::ResourceItem KisResourceStorage::resourceItem(const QString &url)
{
    return d->storagePlugin->resourceItem(url);
}

KoResourceSP KisResourceStorage::resource(const QString &url)
{
    return d->storagePlugin->resource(url);
}

QByteArray KisResourceStorage::resourceMd5(const QString &url)
{
    return d->storagePlugin->resourceMd5(url);
}

QSharedPointer<KisResourceStorage::ResourceIterator> KisResourceStorage::resources(const QString &resourceType) const
{
    return d->storagePlugin->resources(resourceType);
}

QSharedPointer<KisResourceStorage::TagIterator> KisResourceStorage::tags(const QString &resourceType) const
{
    return d->storagePlugin->tags(resourceType);
}

bool KisResourceStorage::addTag(const QString &resourceType, KisTagSP tag)
{
    return d->storagePlugin->addTag(resourceType, tag);
}

bool KisResourceStorage::saveAsNewVersion(KoResourceSP resource)
{
    return d->storagePlugin->saveAsNewVersion(resource->resourceType().first, resource);
}

bool KisResourceStorage::addResource(KoResourceSP resource)
{
    return d->storagePlugin->addResource(resource->resourceType().first, resource);
}

bool KisResourceStorage::importResourceFile(const QString &resourceType, const QString &resourceFile)
{
    return d->storagePlugin->importResourceFile(resourceType, resourceFile);
}

bool KisResourceStorage::supportsVersioning() const
{
    return d->storagePlugin->supportsVersioning();
}

bool KisResourceStorage::loadVersionedResource(KoResourceSP resource)
{
    return d->storagePlugin->loadVersionedResource(resource);
}

void KisResourceStorage::setMetaData(const QString &key, const QVariant &value)
{
    d->storagePlugin->setMetaData(key, value);
}

bool KisResourceStorage::valid() const
{
    return d->valid;
}

QStringList KisResourceStorage::metaDataKeys() const
{
    return d->storagePlugin->metaDataKeys();
}

QVariant KisResourceStorage::metaData(const QString &key) const
{
    return d->storagePlugin->metaData(key);
}

void KisResourceStorage::setStorageId(int storageId)
{
    d->storageId = storageId;
}

int KisResourceStorage::storageId()
{
    return d->storageId;
}

struct VersionedFileParts
{
    QString basename;
    int version = 0;
    QString suffix;
};

boost::optional<VersionedFileParts> guessFilenameParts(const QString &filename)
{
    QRegularExpression exp("^(.*)\\.(\\d\\d*)\\.(.+)$");

    QRegularExpressionMatch res = exp.match(filename);

    if (res.hasMatch()) {
        return VersionedFileParts({res.captured(1), res.captured(2).toInt(), res.captured(3)});
    }

    return boost::none;
}

VersionedFileParts guessFileNamePartsLazy(const QString &filename, int minVersion)
{
    boost::optional<VersionedFileParts> guess = guessFilenameParts(filename);
    if (guess) {
        guess->version = qMax(guess->version, minVersion);
    } else {
        QFileInfo info(filename);
        guess = VersionedFileParts();
        guess->basename = info.baseName();
        guess->version = minVersion;
        guess->suffix = info.completeSuffix();
    }

    return *guess;
}

QString KisStorageVersioningHelper::chooseUniqueName(KoResourceSP resource,
                                                     int minVersion,
                                                     std::function<bool(QString)> checkExists)
{
    int version = qMax(resource->version(), minVersion);

    VersionedFileParts parts = guessFileNamePartsLazy(resource->filename(), version);
    version = parts.version;

    QString newFilename;

    while (1) {
        int numPlaceholders = 4;

        if (version > 9999) {
            numPlaceholders = qFloor(std::log10(version)) + 1;
        }

        QString versionString = QString("%1").arg(version, numPlaceholders, 10, QChar('0'));

        // XXX: Temporary, until I've fixed the tests
        if (versionString == "0000" && qApp->applicationName() == "krita") {
            newFilename = resource->filename();
        }
        else {
            newFilename = parts.basename +
                    "."
                    + versionString
                    + "."
                    + parts.suffix;
        }
        if (checkExists(newFilename)) {
            version++;
            if (version == std::numeric_limits<int>::max()) {
                return QString();
            }
            continue;
        }

        break;
    }

    return newFilename;
}

void KisStorageVersioningHelper::detectFileVersions(QVector<VersionedResourceEntry> &allFiles)
{
    for (auto it = allFiles.begin(); it != allFiles.end(); ++it) {
        VersionedFileParts parts = guessFileNamePartsLazy(it->filename, -1);
        it->guessedKey = parts.basename + parts.suffix;
        it->guessedVersion = parts.version;
    }

    std::sort(allFiles.begin(), allFiles.end(), VersionedResourceEntry::KeyVersionLess());

    boost::optional<QString> lastResourceKey;
    int availableVersion = 0;
    for (auto it = allFiles.begin(); it != allFiles.end(); ++it) {
        if (!lastResourceKey || *lastResourceKey != it->guessedKey) {
            availableVersion = 0;
            lastResourceKey = it->guessedKey;
        }

        if (it->guessedVersion < availableVersion) {
            it->guessedVersion = availableVersion;
        }

        availableVersion = it->guessedVersion + 1;
    }
}

bool KisStorageVersioningHelper::addVersionedResource(const QString &saveLocation,
                                                      KoResourceSP resource,
                                                      int minVersion)
{
    int version = qMax(resource->version(), minVersion);

    VersionedFileParts parts = guessFileNamePartsLazy(resource->filename(), version);
    version = parts.version;

    QString newFilename =
        chooseUniqueName(resource, minVersion,
                         [saveLocation] (const QString &filename) {
                             return QFileInfo(saveLocation + "/" + filename).exists();
                         });

    if (newFilename.isEmpty()) return false;

    QFile file(saveLocation + "/" + newFilename);
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(!file.exists(), false);

    if (!file.open(QFile::WriteOnly)) {
        qWarning() << "Could not open resource file for writing" << newFilename;
        return false;
    }

    if (!resource->saveToDevice(&file)) {
        qWarning() << "Could not save resource file" << newFilename;
        return false;
    }

    resource->setFilename(newFilename);
    file.close();

    if (!resource->thumbnailPath().isEmpty()) {
        // hack for MyPaint brush presets thumbnails
        // note: for all versions of the preset, it will try to save in the same place
        if (!QFileInfo(saveLocation + "/" + resource->thumbnailPath()).exists()) {
            QImage thumbnail = resource->thumbnail();
            thumbnail.save(saveLocation + "/" + resource->thumbnailPath());
        }
    }

    return true;
}


QSharedPointer<KisResourceStorage::ResourceIterator> KisResourceStorage::ResourceIterator::versions() const
{
    struct DumbIterator : public ResourceIterator
    {
    public:
        DumbIterator(const ResourceIterator *parent)
            : m_parent(parent)
        {
        }

        bool hasNext() const override {
            return !m_isStarted;
        }

        void next() override {
            KIS_SAFE_ASSERT_RECOVER_NOOP(!m_isStarted);
            m_isStarted = true;
        }
        QString url() const override
        {
            return m_parent->url();
        }

        QString type() const override
        {
            return m_parent->type();
        }

        QDateTime lastModified() const override
        {
            return m_parent->lastModified();
        }

        int guessedVersion() const override
        {
            return m_parent->guessedVersion();
        }

        QSharedPointer<KisResourceStorage::ResourceIterator> versions() const override
        {
            return toQShared(new DumbIterator(m_parent));
        }

    protected:
        KoResourceSP resourceImpl() const override
        {
            return m_parent->resource();
        }

    private:
        bool m_isStarted = false;
        const ResourceIterator *m_parent;
    };

    return QSharedPointer<KisResourceStorage::ResourceIterator>(new DumbIterator(this));
}

KoResourceSP KisResourceStorage::ResourceIterator::resource() const
{
    if (m_cachedResource && m_cachedResourceUrl == url()) {
        return m_cachedResource;
    }

    m_cachedResource = resourceImpl();
    m_cachedResourceUrl = url();

    return m_cachedResource;
}

KisVersionedStorageIterator::KisVersionedStorageIterator(const QVector<VersionedResourceEntry> &entries, KisStoragePlugin *_q)
    : q(_q)
    , m_entries(entries)
    , m_begin(m_entries.constBegin())
    , m_end(m_entries.constEnd())

{
    //    ENTER_FUNCTION() << ppVar(std::distance(m_begin, m_end));
    //    for (auto it = m_begin; it != m_end; ++it) {
    //        qDebug() << ppVar(it->filename) << ppVar(it->guessedVersion);
    //    }
}

KisVersionedStorageIterator::KisVersionedStorageIterator(const QVector<VersionedResourceEntry> &entries,
                                                         QVector<VersionedResourceEntry>::const_iterator begin,
                                                         QVector<VersionedResourceEntry>::const_iterator end,
                                                         KisStoragePlugin *_q)
    : q(_q)
    , m_entries(entries)
    , m_begin(begin)
    , m_end(end)
{
//    ENTER_FUNCTION() << ppVar(std::distance(m_begin, m_end));
//    for (auto it = m_begin; it != m_end; ++it) {
//        qDebug() << ppVar(it->filename) << ppVar(it->guessedVersion);
//    }
}

bool KisVersionedStorageIterator::hasNext() const
{
    return (!m_isStarted && m_begin != m_end) ||
            (m_isStarted && std::next(m_it) != m_end);
}

void KisVersionedStorageIterator::next()
{

    if (!m_isStarted) {
        m_isStarted = true;
        m_it = m_begin;
    } else {
        ++m_it;
    }

    KIS_SAFE_ASSERT_RECOVER_RETURN(m_it != m_end);

    auto nextChunk = std::upper_bound(m_it, m_end, *m_it, VersionedResourceEntry::KeyLess());
    m_chunkStart = m_it;
    m_it = std::prev(nextChunk);
}

QString KisVersionedStorageIterator::url() const
{
    return m_it->resourceType + "/" + m_it->filename;
}

QString KisVersionedStorageIterator::type() const
{
    return m_it->resourceType;
}

QDateTime KisVersionedStorageIterator::lastModified() const
{
    return m_it->lastModified;
}

KoResourceSP KisVersionedStorageIterator::resourceImpl() const
{
    return q->resource(url());
}

int KisVersionedStorageIterator::guessedVersion() const
{
    return m_it->guessedVersion;
}

QSharedPointer<KisResourceStorage::ResourceIterator> KisVersionedStorageIterator::versions() const
{
    struct VersionsIterator : public KisVersionedStorageIterator
    {
        VersionsIterator(const QVector<VersionedResourceEntry> &entries,
                         QVector<VersionedResourceEntry>::const_iterator begin,
                         QVector<VersionedResourceEntry>::const_iterator end,
                         KisStoragePlugin *_q)
            : KisVersionedStorageIterator(entries, begin, end, _q)
        {
        }

        void next() override {
            if (!m_isStarted) {
                m_isStarted = true;
                m_it = m_begin;
            } else {
                ++m_it;
            }
        }

        QSharedPointer<KisResourceStorage::ResourceIterator> versions() const override{
            return toQShared(new VersionsIterator(m_entries, m_it, std::next(m_it), q));
        }
    };

    return toQShared(new VersionsIterator(m_entries, m_chunkStart, std::next(m_it), q));
}
