/*
 *  Copyright (c) 2014 Victor Lafon metabolic.ewilan@hotmail.fr
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
#include <KoHashGenerator.h>
#include <KoHashGeneratorProvider.h>
#include <KoResourcePaths.h>
#include <KoStore.h>
#include <KoXmlReader.h>
#include <KoXmlWriter.h>
#include "KisStoragePlugin.h"
#include "KisResourceLoaderRegistry.h"

#include <KritaVersionWrapper.h>

#include <kis_debug.h>

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

    QFileInfo fi(resource->filename());
    if (fi.exists() && fi.isReadable()) {

        QFile f(resource->filename());
        if (!f.open(QFile::ReadOnly)) {
            qWarning() << "Could not open resource" << resource->filename();
            return false;
        }
        ba = f.readAll();
        if (ba.size() == 0) {
            qWarning() << "Resource is empty" << resource->filename();
            return false;
        }
        f.close();
        buf.setBuffer(&ba);
    }
    else {
        qWarning() << "Could not find the resource " << resource->filename() << " or it isn't readable";
        return false;
    }

    if (!buf.open(QBuffer::ReadOnly)) {
        qWarning() << "Could not open buffer";
        return false;
    }
    Q_ASSERT(!store->hasFile(resType + "/" + resource->filename()));
    if (!store->open(resType + "/" + resource->filename())) {
        qWarning() << "Could not open file in store for resource";
        return false;
    }

    bool res = (store->write(buf.data()) == buf.size());
    store->close();
    return res;

}

bool KoResourceBundle::save()
{
    if (m_filename.isEmpty()) return false;

    setMetaData(KisResourceStorage::s_meta_dc_date, QDate::currentDate().toString("dd/MM/yyyy"));

    QDir bundleDir = KoResourcePaths::saveLocation("data", "bundles");
    bundleDir.cdUp();

    QScopedPointer<KoStore> store(KoStore::createStore(m_filename, KoStore::Write, "application/x-krita-resourcebundle", KoStore::Zip));

    if (!store || store->bad()) return false;

    //    Q_FOREACH (const QString &resType, m_manifest.types()) {

    //        if (resType == ResourceType::Gradients) {
    //            KoResourceServer<KoAbstractGradient>* gradientServer = KoResourceServerProvider::instance()->gradientServer();
    //            Q_FOREACH (const KoResourceBundleManifest::ResourceReference &ref, m_manifest.files(resType)) {
    //                KoResourceSP res = gradientServer->resourceByMD5(ref.md5sum);
    //                if (!res) res = gradientServer->resourceByFilename(QFileInfo(ref.resourcePath).m_filename);
    //                if (!saveResourceToStore(res, store.data(), ResourceType::Gradients)) {
    //                    if (res) {
    //                        qWarning() << "Could not save resource" << resType << res->name();
    //                    }
    //                    else {
    //                        qWarning() << "could not find resource for" << QFileInfo(ref.resourcePath).m_filename;
    //                    }
    //                }
    //            }
    //        }
    //        else if (resType  == ResourceType::Patterns) {
    //            KoResourceServer<KoPattern>* patternServer = KoResourceServerProvider::instance()->patternServer();
    //            Q_FOREACH (const KoResourceBundleManifest::ResourceReference &ref, m_manifest.files(resType)) {
    //                KoResourceSP res = patternServer->resourceByMD5(ref.md5sum);
    //                if (!res) res = patternServer->resourceByFilename(QFileInfo(ref.resourcePath).m_filename);
    //                if (!saveResourceToStore(res, store.data(), ResourceType::Patterns)) {
    //                    if (res) {
    //                        qWarning() << "Could not save resource" << resType << res->name();
    //                    }
    //                    else {
    //                        qWarning() << "could not find resource for" << QFileInfo(ref.resourcePath).fileName();
    //                    }
    //                }
    //            }
    //        }
    //        else if (resType  == ResourceType::Brushes) {
    //            KoResourceServer<KisBrush>* brushServer = KisBrushServerProvider::instance()->brushServer();
    //            Q_FOREACH (const KoResourceBundleManifest::ResourceReference &ref, m_manifest.files(resType)) {
    //                KisBrushSP brush = brushServer->resourceByMD5(ref.md5sum);
    //                if (!brush) brush = brushServer->resourceByFilename(QFileInfo(ref.resourcePath).fileName());
    //                KoResourceSP res = brush.data();
    //                if (!saveResourceToStore(res, store.data(), ResourceType::Brushes)) {
    //                    if (res) {
    //                        qWarning() << "Could not save resource" << resType << res->name();
    //                    }
    //                    else {
    //                        qWarning() << "could not find resource for" << QFileInfo(ref.resourcePath).fileName();
    //                    }
    //                }
    //            }
    //        }
    //        else if (resType  == ResourceType::Palettes) {
    //            KoResourceServer<KoColorSet>* paletteServer = KoResourceServerProvider::instance()->paletteServer();
    //            Q_FOREACH (const KoResourceBundleManifest::ResourceReference &ref, m_manifest.files(resType)) {
    //                KoResourceSP res = paletteServer->resourceByMD5(ref.md5sum);
    //                if (!res) res = paletteServer->resourceByFilename(QFileInfo(ref.resourcePath).fileName());
    //                if (!saveResourceToStore(res, store.data(), ResourceType::Palettes)) {
    //                    if (res) {
    //                        qWarning() << "Could not save resource" << resType << res->name();
    //                    }
    //                    else {
    //                        qWarning() << "could not find resource for" << QFileInfo(ref.resourcePath).fileName();
    //                    }
    //                }
    //            }
    //        }
    //        else if (resType  == ResourceType::Workspaces) {
    //            KoResourceServer< KisWorkspaceResource >* workspaceServer = KisResourceServerProvider::instance()->workspaceServer();
    //            Q_FOREACH (const KoResourceBundleManifest::ResourceReference &ref, m_manifest.files(resType)) {
    //                KoResourceSP res = workspaceServer->resourceByMD5(ref.md5sum);
    //                if (!res) res = workspaceServer->resourceByFilename(QFileInfo(ref.resourcePath).fileName());
    //                if (!saveResourceToStore(res, store.data(), ResourceType::Workspaces)) {
    //                    if (res) {
    //                        qWarning() << "Could not save resource" << resType << res->name();
    //                    }
    //                    else {
    //                        qWarning() << "could not find resource for" << QFileInfo(ref.resourcePath).fileName();
    //                    }
    //                }
    //            }
    //        }
    //        else if (resType  == ResourceType::PaintOpPresets) {
    //            KisPaintOpPresetResourceServer* paintoppresetServer = KisResourceServerProvider::instance()->paintOpPresetServer();
    //            Q_FOREACH (const KoResourceBundleManifest::ResourceReference &ref, m_manifest.files(resType)) {
    //                KisPaintOpPresetSP res = paintoppresetServer->resourceByMD5(ref.md5sum);
    //                if (!res) res = paintoppresetServer->resourceByFilename(QFileInfo(ref.resourcePath).fileName());
    //                if (!saveResourceToStore(res.data(), store.data(), ResourceType::PaintOpPresets)) {
    //                    if (res) {
    //                        qWarning() << "Could not save resource" << resType << res->name();
    //                    }
    //                    else {
    //                        qWarning() << "could not find resource for" << QFileInfo(ref.resourcePath).fileName();
    //                    }
    //                }
    //            }
    //        }
    //    }

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

void KoResourceBundle::addResource(QString fileType, QString filePath, QVector<KisTagSP> fileTagList, const QByteArray md5sum)
{
    QStringList tags;
    Q_FOREACH(KisTagSP tag, fileTagList) {
        tags << tag->url();
    }
    m_manifest.addResource(fileType, filePath, tags, md5sum);
}

QList<QString> KoResourceBundle::getTagsList()
{
    return QList<QString>::fromSet(m_bundletags);
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
    if (m_filename.isEmpty()) return 0;


    QScopedPointer<KoStore> resourceStore(KoStore::createStore(m_filename, KoStore::Read, "application/x-krita-resourcebundle", KoStore::Zip));

    if (!resourceStore || resourceStore->bad()) {
        qWarning() << "Could not open store on bundle" << m_filename;
        return 0;
    }

    if (!resourceStore->open(filepath)) {
        qWarning() << "Could not open file in bundle" << filepath;
    }

    QString mime = KisMimeDatabase::mimeTypeForSuffix(filepath);
    KisResourceLoaderBase *loader = KisResourceLoaderRegistry::instance()->loader(resourceType, mime);
    if (!loader) {
        qWarning() << "Could not create loader for" << resourceType << filepath << mime;
        return 0;
    }
    KoResourceSP res = loader->load(filepath, *resourceStore->device());
    resourceStore->close();

    return res;
}

QImage KoResourceBundle::image() const
{
    return m_thumbnail;
}

QString KoResourceBundle::filename() const
{
    return m_filename;
}
