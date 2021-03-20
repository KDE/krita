/*
 *  SPDX-FileCopyrightText: 2014 Victor Lafon metabolic.ewilan @hotmail.fr
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KoResourceBundle.h"

#include <QBuffer>
#include <QByteArray>
#include <QCryptographicHash>
#include <QDate>
#include <QDir>
#include <QMessageBox>
#include <QPainter>
#include <QProcessEnvironment>
#include <QScopedPointer>
#include <QStringList>

#include <klocalizedstring.h>

#include <KisMimeDatabase.h>
#include "KoResourceBundleManifest.h"
#include <KoMD5Generator.h>
#include <KoResourcePaths.h>
#include <KoStore.h>
#include <KoXmlReader.h>
#include <KoXmlWriter.h>
#include "KisStoragePlugin.h"
#include "KisResourceLoaderRegistry.h"
#include <KisResourceModelProvider.h>
#include <KisResourceModel.h>
#include <KoMD5Generator.h>

#include <KritaVersionWrapper.h>

#include <kis_debug.h>
#include <KisGlobalResourcesInterface.h>


KoResourceBundle::KoResourceBundle(QString const& fileName)
    : m_filename(fileName),
      m_bundleVersion("1")
{
    m_metadata[KisResourceStorage::s_meta_generator] = "Krita (" + KritaVersionWrapper::versionString(true) + ")";
}

KoResourceBundle::~KoResourceBundle()
{
}

QString KoResourceBundle::defaultFileExtension() const
{
    return QString(".bundle");
}

bool KoResourceBundle::load()
{
    if (m_filename.isEmpty()) return false;
    QScopedPointer<KoStore> resourceStore(KoStore::createStore(m_filename, KoStore::Read, "application/x-krita-resourcebundle", KoStore::Zip));

    if (!resourceStore || resourceStore->bad()) {
        qWarning() << "Could not open store on bundle" << m_filename;
        return false;

    }
    else {

        m_metadata.clear();

        if (resourceStore->open("META-INF/manifest.xml")) {
            if (!m_manifest.load(resourceStore->device())) {
                qWarning() << "Could not open manifest for bundle" << m_filename;
                return false;
            }
            resourceStore->close();

            Q_FOREACH (KoResourceBundleManifest::ResourceReference ref, m_manifest.files()) {
                if (!resourceStore->open(ref.resourcePath)) {
                    qWarning() << "Bundle is broken. File" << ref.resourcePath << "is missing";
                }
                else {
                    resourceStore->close();
                }
            }

        } else {
            qWarning() << "Could not load META-INF/manifest.xml";
            return false;
        }

        bool versionFound = false;
        if (!readMetaData(resourceStore.data())) {
            qWarning() << "Could not load meta.xml";
            return false;
        }

        if (resourceStore->open("preview.png")) {
            // Workaround for some OS (Debian, Ubuntu), where loading directly from the QIODevice
            // fails with "libpng error: IDAT: CRC error"
            QByteArray data = resourceStore->device()->readAll();
            QBuffer buffer(&data);
            m_thumbnail.load(&buffer, "PNG");
            resourceStore->close();
        }
        else {
            qWarning() << "Could not open preview.png";
        }

        /*
         * If no version is found it's an old bundle with md5 hashes to fix, or if some manifest resource entry
         * doesn't not correspond to a file the bundle is "broken", in both cases we need to recreate the bundle.
         */
        if (!versionFound) {
            m_metadata.insert(KisResourceStorage::s_meta_version, "1");
        }

    }

    return true;
}

bool KoResourceBundle::loadFromDevice(QIODevice *)
{
    return false;
}

bool saveResourceToStore(KoResourceSP resource, KoStore *store, const QString &resType)
{
    if (!resource) {
        qWarning() << "No Resource";
        return false;
    }

    if (!resource->valid()) {
        qWarning() << "Resource is not valid";
        return false;
    }
    if (!store || store->bad()) {
        qWarning() << "No Store or Store is Bad";
        return false;
    }

    QByteArray ba;
    QBuffer buf;
    buf.open(QFile::ReadWrite);

    bool response = resource->saveToDevice(&buf);
    if (!response) {
        ENTER_FUNCTION() << "Cannot save to device";
        return false;
    }

    if (!store->open(resType + "/" + resource->filename())) {
        qWarning() << "Could not open file in store for resource";
        return false;
    }

    qint64 size = store->write(buf.data());
    store->close();
    buf.close();
    if (size != buf.size()) {
        ENTER_FUNCTION() << "Cannot save to the store" << size << buf.size();
    }
    return size == buf.size();
}

bool KoResourceBundle::save()
{
    if (m_filename.isEmpty()) return false;

    setMetaData(KisResourceStorage::s_meta_dc_date, QDate::currentDate().toString("dd/MM/yyyy"));

    QDir bundleDir = KoResourcePaths::saveLocation("data", "bundles");
    bundleDir.cdUp();

    QScopedPointer<KoStore> store(KoStore::createStore(m_filename, KoStore::Write, "application/x-krita-resourcebundle", KoStore::Zip));

    if (!store || store->bad()) return false;

    Q_FOREACH (const QString &resType, m_manifest.types()) {
        KisResourceModel model(resType);
        Q_FOREACH (const KoResourceBundleManifest::ResourceReference &ref, m_manifest.files(resType)) {
            KoResourceSP res = model.resourceForMD5(ref.md5sum);
            if (!res) res = model.resourceForFilename(QFileInfo(ref.resourcePath).fileName());
            if (!saveResourceToStore(res, store.data(), resType)) {
                if (res) {
                    qWarning() << "Could not save resource" << resType << res->name();
                }
                else {
                    qWarning() << "could not find resource for" << QFileInfo(ref.resourcePath).fileName();
                }
            }
        }
    }

    if (!m_thumbnail.isNull()) {
        QByteArray byteArray;
        QBuffer buffer(&byteArray);
        m_thumbnail.save(&buffer, "PNG");
        if (!store->open("preview.png")) qWarning() << "Could not open preview.png";
        if (store->write(byteArray) != buffer.size()) qWarning() << "Could not write preview.png";
        store->close();
    }

    saveManifest(store);

    saveMetadata(store);

    store->finalize();

    return true;
}

bool KoResourceBundle::saveToDevice(QIODevice */*dev*/) const
{
    return false;
}

void KoResourceBundle::setMetaData(const QString &key, const QString &value)
{
    m_metadata.insert(key, value);
}

const QString KoResourceBundle::metaData(const QString &key, const QString &defaultValue) const
{
    if (m_metadata.contains(key)) {
        return m_metadata[key];
    }
    else {
        return defaultValue;
    }
}

void KoResourceBundle::addResource(QString resourceType, QString filePath, QVector<KisTagSP> fileTagList, const QByteArray md5sum)
{
    QStringList tags;
    Q_FOREACH(KisTagSP tag, fileTagList) {
        tags << tag->url();
    }
    m_manifest.addResource(resourceType, filePath, tags, md5sum);
}

QList<QString> KoResourceBundle::getTagsList()
{
#if QT_VERSION >= QT_VERSION_CHECK(5,14,0)
    return QList<QString>(m_bundletags.begin(), m_bundletags.end());
#else
    return QList<QString>::fromSet(m_bundletags);
#endif
}

QStringList KoResourceBundle::resourceTypes() const
{
    return m_manifest.types();
}

void KoResourceBundle::setThumbnail(QString filename)
{
    if (QFileInfo(filename).exists()) {
        m_thumbnail = QImage(filename);
        m_thumbnail = m_thumbnail.scaled(256, 256, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
    else {
        m_thumbnail = QImage(256, 256, QImage::Format_ARGB32);
        QPainter gc(&m_thumbnail);
        gc.fillRect(0, 0, 256, 256, Qt::red);
        gc.end();
    }
}

void KoResourceBundle::writeMeta(const QString &metaTag, KoXmlWriter *writer)
{
    if (m_metadata.contains(metaTag)) {
        writer->startElement(metaTag.toUtf8());
        writer->addTextNode(m_metadata[metaTag].toUtf8());
        writer->endElement();
    }
}

void KoResourceBundle::writeUserDefinedMeta(const QString &metaTag, KoXmlWriter *writer)
{
    if (m_metadata.contains(metaTag)) {
        writer->startElement("meta:meta-userdefined");
        writer->addAttribute("meta:name", metaTag);
        writer->addAttribute("meta:value", m_metadata[metaTag]);
        writer->endElement();
    }
}

bool KoResourceBundle::readMetaData(KoStore *resourceStore)
{
    if (resourceStore->open("meta.xml")) {
        KoXmlDocument doc;
        if (!doc.setContent(resourceStore->device())) {
            qWarning() << "Could not parse meta.xml for" << m_filename;
            return false;
        }
        // First find the manifest:manifest node.
        KoXmlNode n = doc.firstChild();
        for (; !n.isNull(); n = n.nextSibling()) {
            if (!n.isElement()) {
                continue;
            }
            if (n.toElement().tagName() == "meta:meta") {
                break;
            }
        }

        if (n.isNull()) {
            qWarning() << "Could not find manifest node for bundle" << m_filename;
            return false;
        }

        const KoXmlElement  metaElement = n.toElement();
        for (n = metaElement.firstChild(); !n.isNull(); n = n.nextSibling()) {
            if (n.isElement()) {
                KoXmlElement e = n.toElement();
                if (e.tagName() == "meta:meta-userdefined") {
                    if (e.attribute("meta:name") == "tag") {
                        m_bundletags << e.attribute("meta:value");
                    }
                    else {
                        m_metadata.insert(e.attribute("meta:name"), e.attribute("meta:value"));
                    }
                }
                else {
                    m_metadata.insert(e.tagName(), e.firstChild().toText().data());
                }
            }
        }
        resourceStore->close();
        return true;
    }
    return false;
}

void KoResourceBundle::saveMetadata(QScopedPointer<KoStore> &store)
{
    QBuffer buf;

    store->open("meta.xml");
    buf.open(QBuffer::WriteOnly);

    KoXmlWriter metaWriter(&buf);
    metaWriter.startDocument("office:document-meta");
    metaWriter.startElement("meta:meta");

    writeMeta(KisResourceStorage::s_meta_generator, &metaWriter);

    metaWriter.startElement(KisResourceStorage::s_meta_version.toUtf8());
    metaWriter.addTextNode(m_bundleVersion.toUtf8());
    metaWriter.endElement();

    writeMeta(KisResourceStorage::s_meta_author, &metaWriter);
    writeMeta(KisResourceStorage::s_meta_title,  &metaWriter);
    writeMeta(KisResourceStorage::s_meta_description, &metaWriter);
    writeMeta(KisResourceStorage::s_meta_initial_creator,  &metaWriter);
    writeMeta(KisResourceStorage::s_meta_creator, &metaWriter);
    writeMeta(KisResourceStorage::s_meta_creation_date, &metaWriter);
    writeMeta(KisResourceStorage::s_meta_dc_date, &metaWriter);
    writeUserDefinedMeta("email", &metaWriter);
    writeUserDefinedMeta("license", &metaWriter);
    writeUserDefinedMeta("website", &metaWriter);
    Q_FOREACH (const QString &tag, m_bundletags) {
        metaWriter.startElement(KisResourceStorage::s_meta_user_defined.toUtf8());
        metaWriter.addAttribute(KisResourceStorage::s_meta_name.toUtf8(), "tag");
        metaWriter.addAttribute(KisResourceStorage::s_meta_value.toUtf8(), tag);
        metaWriter.endElement();
    }

    metaWriter.endElement(); // meta:meta
    metaWriter.endDocument();

    buf.close();
    store->write(buf.data());
    store->close();
}

void KoResourceBundle::saveManifest(QScopedPointer<KoStore> &store)
{
    store->open("META-INF/manifest.xml");
    QBuffer buf;
    buf.open(QBuffer::WriteOnly);
    m_manifest.save(&buf);
    buf.close();
    store->write(buf.data());
    store->close();
}

int KoResourceBundle::resourceCount() const
{
    return m_manifest.files().count();
}

KoResourceBundleManifest &KoResourceBundle::manifest()
{
    return m_manifest;
}

KoResourceSP KoResourceBundle::resource(const QString &resourceType, const QString &filepath)
{
    QString mime = KisMimeDatabase::mimeTypeForSuffix(filepath);
    KisResourceLoaderBase *loader = KisResourceLoaderRegistry::instance()->loader(resourceType, mime);
    if (!loader) {
        qWarning() << "Could not create loader for" << resourceType << filepath << mime;
        return 0;
    }

    QStringList parts = filepath.split('/', QString::SkipEmptyParts);
    Q_ASSERT(parts.size() == 2);

    KoResourceSP resource = loader->create(parts[1]);
    return loadResource(resource) ? resource : 0;
}

bool KoResourceBundle::loadResource(KoResourceSP resource)
{
    if (m_filename.isEmpty()) return false;

    const QString resourceType = resource->resourceType().first;

    QScopedPointer<KoStore> resourceStore(KoStore::createStore(m_filename, KoStore::Read, "application/x-krita-resourcebundle", KoStore::Zip));

    if (!resourceStore || resourceStore->bad()) {
        qWarning() << "Could not open store on bundle" << m_filename;
        return false;
    }
    const QString fileName = QString("%1/%2").arg(resourceType).arg(resource->filename());

    if (!resourceStore->open(fileName)) {
        qWarning() << "Could not open file in bundle" << fileName;
        return false;
    }

    if (!resource->loadFromDevice(resourceStore->device(),
                                  KisGlobalResourcesInterface::instance())) {
        qWarning() << "Could not reload the resource from the bundle" << resourceType << fileName << m_filename;
        return false;
    }

    if ((resource->image().isNull() || resource->thumbnail().isNull()) && !resource->thumbnailPath().isNull()) {

        if (!resourceStore->open(resourceType + '/' + resource->thumbnailPath())) {
            qWarning() << "Could not open thumbnail in bundle" << resource->thumbnailPath();
            return false;
        }

        QImage img;
        img.load(resourceStore->device(), QFileInfo(resource->thumbnailPath()).completeSuffix().toLatin1());
        resource->setImage(img);
        resource->updateThumbnail();
    }

    return true;
}

QByteArray KoResourceBundle::resourceMd5(const QString &url)
{
    QByteArray result;

    if (m_filename.isEmpty()) return result;

    QScopedPointer<KoStore> resourceStore(KoStore::createStore(m_filename, KoStore::Read, "application/x-krita-resourcebundle", KoStore::Zip));

    if (!resourceStore || resourceStore->bad()) {
        qWarning() << "Could not open store on bundle" << m_filename;
        return result;
    }
    if (!resourceStore->open(url)) {
        qWarning() << "Could not open file in bundle" << url;
        return result;
    }

    result = KoMD5Generator::generateHash(resourceStore->device()->readAll());
    resourceStore->close();

    return result;
}

QImage KoResourceBundle::image() const
{
    return m_thumbnail;
}

QString KoResourceBundle::filename() const
{
    return m_filename;
}
