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
                if (!resourceStore->hasFile(ref.resourcePath)) {
                    m_manifest.removeResource(ref);
                    qWarning() << "Bundle" << filename() <<  "is broken. File" << ref.resourcePath << "is missing";
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

bool saveResourceToStore(const QString &filename, KoResourceSP resource, KoStore *store, const QString &resType, KisResourceModel &model)
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

    QBuffer buf;
    buf.open(QFile::ReadWrite);

    bool response = model.exportResource(resource, &buf);
    if (!response) {
        ENTER_FUNCTION() << "Cannot save to device";
        return false;
    }

    if (!store->open(resType + "/" + filename)) {
        qWarning() << "Could not open file in store for resource";
        return false;
    }

    qint64 size = store->write(buf.data());
    store->close();
    buf.close();
    if (size != buf.size()) {
        qWarning() << "Cannot save resource to the store" << size << buf.size();
        return false;
    }

    if (!resource->thumbnailPath().isEmpty()) {
        // hack for MyPaint brush presets previews
        const QImage thumbnail = resource->thumbnail();

        // clone resource to find out the file path for its preview
        KoResourceSP clonedResource = resource->clone();
        clonedResource->setFilename(filename);

        if (!store->open(resType + "/" + clonedResource->thumbnailPath())) {
            qWarning() << "Could not open file in store for resource thumbnail";
            return false;
        }
        QBuffer buf;
        buf.open(QFile::ReadWrite);
        thumbnail.save(&buf, "PNG");

        int size2 = store->write(buf.data());
        if (size2 != buf.size()) {
            qWarning() << "Cannot save thumbnail to the store" << size << buf.size();
        }
        store->close();
        buf.close();
    }


    return size == buf.size();
}

bool KoResourceBundle::save()
{
    if (m_filename.isEmpty()) return false;

    if (metaData(KisResourceStorage::s_meta_creation_date, "").isEmpty()) {
        setMetaData(KisResourceStorage::s_meta_creation_date, QLocale::c().toString(QDate::currentDate(), QStringLiteral("dd/MM/yyyy")));
    }
    setMetaData(KisResourceStorage::s_meta_dc_date, QLocale::c().toString(QDate::currentDate(), QStringLiteral("dd/MM/yyyy")));

    QDir bundleDir = KoResourcePaths::saveLocation("data", "bundles");
    bundleDir.cdUp();

    QScopedPointer<KoStore> store(KoStore::createStore(m_filename, KoStore::Write, "application/x-krita-resourcebundle", KoStore::Zip));

    if (!store || store->bad()) return false;

    Q_FOREACH (const QString &resType, m_manifest.types()) {
        KisResourceModel model(resType);
        model.setResourceFilter(KisResourceModel::ShowAllResources);
        Q_FOREACH (const KoResourceBundleManifest::ResourceReference &ref, m_manifest.files(resType)) {
            KoResourceSP res;
            if (ref.resourceId >= 0) res = model.resourceForId(ref.resourceId);
            if (!res) res = model.resourcesForMD5(ref.md5sum).first();
            if (!res) res = model.resourcesForFilename(QFileInfo(ref.resourcePath).fileName()).first();
            if (!res) {
                qWarning() << "Could not find resource" << resType << ref.resourceId << ref.md5sum << ref.resourcePath;
                continue;
            }

            if (!saveResourceToStore(ref.filenameInBundle, res, store.data(), resType, model)) {
                qWarning() << "Could not save resource" << resType << res->name();
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

void KoResourceBundle::addResource(QString resourceType, QString filePath, QVector<KisTagSP> fileTagList, const QString md5sum, const int resourceId, const QString filenameInBundle)
{
    QStringList tags;
    Q_FOREACH(KisTagSP tag, fileTagList) {
        tags << tag->url();
    }
    m_manifest.addResource(resourceType, filePath, tags, md5sum, resourceId, filenameInBundle);
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
        QByteArray mt = metaTag.toUtf8();
        QByteArray tx = m_metadata[metaTag].toUtf8();
        writer->startElement(mt);
        writer->addTextNode(tx);
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
        QDomDocument doc;
        if (!doc.setContent(resourceStore->device())) {
            qWarning() << "Could not parse meta.xml for" << m_filename;
            return false;
        }
        // First find the manifest:manifest node.
        QDomNode n = doc.firstChild();
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

        const QDomElement  metaElement = n.toElement();
        for (n = metaElement.firstChild(); !n.isNull(); n = n.nextSibling()) {
            if (n.isElement()) {
                QDomElement e = n.toElement();
                if (e.tagName() == "meta:meta-userdefined") {
                    if (e.attribute("meta:name") == "tag") {
                        m_bundletags << e.attribute("meta:value");
                    }
                    else {
                        QString metaName = e.attribute("meta:name");
                        if (!metaName.startsWith("meta:") && !metaName.startsWith("dc:")) {
                            if (metaName == "email" || metaName == "license" || metaName == "website") { // legacy metadata options
                                if (!m_metadata.contains("meta:" + metaName)) {
                                    m_metadata.insert("meta:" + metaName, e.attribute("meta:value"));
                                }
                            } else {
                                qWarning() << "Unrecognized metadata: " << e.tagName() << e.attribute("meta:name") << e.attribute("meta:value");
                            }
                        }
                        m_metadata.insert(e.attribute("meta:name"), e.attribute("meta:value"));
                    }
                }
                else {
                    if (!m_metadata.contains(e.tagName())) {
                        m_metadata.insert(e.tagName(), e.firstChild().toText().data());
                    }
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

    QByteArray ba1 = KisResourceStorage::s_meta_version.toUtf8();
    metaWriter.startElement(ba1);
    QByteArray ba2  = m_bundleVersion.toUtf8();
    metaWriter.addTextNode(ba2);
    metaWriter.endElement();

    writeMeta(KisResourceStorage::s_meta_author, &metaWriter);
    writeMeta(KisResourceStorage::s_meta_title,  &metaWriter);
    writeMeta(KisResourceStorage::s_meta_description, &metaWriter);
    writeMeta(KisResourceStorage::s_meta_initial_creator,  &metaWriter);
    writeMeta(KisResourceStorage::s_meta_creator, &metaWriter);
    writeMeta(KisResourceStorage::s_meta_creation_date, &metaWriter);
    writeMeta(KisResourceStorage::s_meta_dc_date, &metaWriter);
    writeMeta(KisResourceStorage::s_meta_email, &metaWriter);
    writeMeta(KisResourceStorage::s_meta_license, &metaWriter);
    writeMeta(KisResourceStorage::s_meta_website, &metaWriter);

    // For compatibility
    writeUserDefinedMeta("email", &metaWriter);
    writeUserDefinedMeta("license", &metaWriter);
    writeUserDefinedMeta("website", &metaWriter);


    Q_FOREACH (const QString &tag, m_bundletags) {
        QByteArray ba1 = KisResourceStorage::s_meta_user_defined.toUtf8();
        QByteArray ba2 = KisResourceStorage::s_meta_name.toUtf8();
        QByteArray ba3 = KisResourceStorage::s_meta_value.toUtf8();
        metaWriter.startElement(ba1);
        metaWriter.addAttribute(ba2, "tag");
        metaWriter.addAttribute(ba3, tag);
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
        qWarning() << "Could not load the resource from the bundle" << resourceType << fileName << m_filename;
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

QString KoResourceBundle::resourceMd5(const QString &url)
{
    QString result;

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

    result = KoMD5Generator::generateHash(resourceStore->device());
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
