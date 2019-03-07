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

#include "KisResourceBundle.h"
#include "KisResourceBundleManifest.h"

#include <KoXmlReader.h>
#include <KoXmlWriter.h>
#include <KoStore.h>
#include <KoResourceServerProvider.h>
#include <KoResourcePaths.h>

#include <QScopedPointer>
#include <QProcessEnvironment>
#include <QDate>
#include <QDir>
#include <kis_debug.h>
#include <QBuffer>
#include <QCryptographicHash>
#include <QByteArray>
#include <QPainter>
#include <QStringList>
#include <QMessageBox>

#include <KoHashGeneratorProvider.h>
#include <KoHashGenerator.h>
#include <KisResourceServerProvider.h>
#include <kis_workspace_resource.h>
#include <brushengine/kis_paintop_preset.h>
#include <KisBrushServerProvider.h>
#include <resources/KoGamutMask.h>
#include <KritaVersionWrapper.h>

KisResourceBundle::KisResourceBundle(QString const& fileName)
    : KoResource(fileName),
      m_bundleVersion("1")
{
    setName(QFileInfo(fileName).baseName());
    m_metadata["generator"] = "Krita (" + KritaVersionWrapper::versionString(true) + ")";

}

KisResourceBundle::~KisResourceBundle()
{
}

QString KisResourceBundle::defaultFileExtension() const
{
    return QString(".bundle");
}

bool KisResourceBundle::load()
{
    if (filename().isEmpty()) return false;
    QScopedPointer<KoStore> resourceStore(KoStore::createStore(filename(), KoStore::Read, "application/x-krita-resourcebundle", KoStore::Zip));

    if (!resourceStore || resourceStore->bad()) {
        warnKrita << "Could not open store on bundle" << filename();
        m_installed = false;
        setValid(false);
        return false;

    } else {

        m_metadata.clear();

        bool toRecreate = false;
        if (resourceStore->open("META-INF/manifest.xml")) {
            if (!m_manifest.load(resourceStore->device())) {
                warnKrita << "Could not open manifest for bundle" << filename();
                return false;
            }
            resourceStore->close();

            Q_FOREACH (KisResourceBundleManifest::ResourceReference ref, m_manifest.files()) {
                if (!resourceStore->open(ref.resourcePath)) {
                    warnKrita << "Bundle is broken. File" << ref.resourcePath << "is missing";
                    toRecreate = true;
                }
                else {
                    resourceStore->close();
                }
            }


            if(toRecreate) {
                warnKrita << "Due to missing files and wrong entries in the manifest, " << filename() << " will be recreated.";
            }

        } else {
            warnKrita << "Could not load META-INF/manifest.xml";
            return false;
        }

        bool versionFound = false;
        if (resourceStore->open("meta.xml")) {
            KoXmlDocument doc;
            if (!doc.setContent(resourceStore->device())) {
                warnKrita << "Could not parse meta.xml for" << filename();
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
                warnKrita << "Could not find manifest node for bundle" << filename();
                return false;
            }

            const KoXmlElement  metaElement = n.toElement();
            for (n = metaElement.firstChild(); !n.isNull(); n = n.nextSibling()) {
                if (n.isElement()) {
                    KoXmlElement e = n.toElement();
                    if (e.tagName() == "meta:generator") {
                        m_metadata.insert("generator", e.firstChild().toText().data());
                    }
                    else if (e.tagName() == "dc:author") {
                        m_metadata.insert("author", e.firstChild().toText().data());
                    }
                    else if (e.tagName() == "dc:title") {
                        m_metadata.insert("title", e.firstChild().toText().data());
                    }
                    else if (e.tagName() == "dc:description") {
                        m_metadata.insert("description", e.firstChild().toText().data());
                    }
                    else if (e.tagName() == "meta:initial-creator") {
                        m_metadata.insert("author", e.firstChild().toText().data());
                    }
                    else if (e.tagName() == "dc:creator") {
                        m_metadata.insert("author", e.firstChild().toText().data());
                    }
                    else if (e.tagName() == "meta:creation-date") {
                        m_metadata.insert("created", e.firstChild().toText().data());
                    }
                    else if (e.tagName() == "meta:dc-date") {
                        m_metadata.insert("updated", e.firstChild().toText().data());
                    }
                    else if (e.tagName() == "meta:meta-userdefined") {
                        if (e.attribute("meta:name") == "tag") {
                            m_bundletags << e.attribute("meta:value");
                        }
                        else {
                            m_metadata.insert(e.attribute("meta:name"), e.attribute("meta:value"));
                        }
                    }
                    else if(e.tagName() == "meta:bundle-version") {
                        m_metadata.insert("bundle-version", e.firstChild().toText().data());
                        versionFound = true;
                    }
                }
            }
            resourceStore->close();
        }
        else {
            warnKrita << "Could not load meta.xml";
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
            warnKrita << "Could not open preview.png";
        }

        /*
         * If no version is found it's an old bundle with md5 hashes to fix, or if some manifest resource entry
         * doesn't not correspond to a file the bundle is "broken", in both cases we need to recreate the bundle.
         */
        if (!versionFound) {
            m_metadata.insert("bundle-version", "1");
            warnKrita << filename() << " has an old version and possibly wrong resources md5, so it will be recreated.";
            toRecreate = true;
        }

        if (toRecreate) {
            recreateBundle(resourceStore);
        }


        m_installed = true;
        setValid(true);
        setImage(m_thumbnail);
    }

    return true;
}

bool KisResourceBundle::loadFromDevice(QIODevice *)
{
    return false;
}

bool saveResourceToStore(KoResourceSP resource, KoStore *store, const QString &resType)
{
    if (!resource) {
        warnKrita << "No Resource";
        return false;
    }

    if (!resource->valid()) {
        warnKrita << "Resource is not valid";
        return false;
    }
    if (!store || store->bad()) {
        warnKrita << "No Store or Store is Bad";
        return false;
    }

    QByteArray ba;
    QBuffer buf;

    QFileInfo fi(resource->filename());
    if (fi.exists() && fi.isReadable()) {

        QFile f(resource->filename());
        if (!f.open(QFile::ReadOnly)) {
            warnKrita << "Could not open resource" << resource->filename();
            return false;
        }
        ba = f.readAll();
        if (ba.size() == 0) {
            warnKrita << "Resource is empty" << resource->filename();
            return false;
        }
        f.close();
        buf.setBuffer(&ba);
    }
    else {
        warnKrita << "Could not find the resource " << resource->filename() << " or it isn't readable";
        return false;
    }

    if (!buf.open(QBuffer::ReadOnly)) {
        warnKrita << "Could not open buffer";
        return false;
    }
    Q_ASSERT(!store->hasFile(resType + "/" + resource->shortFilename()));
    if (!store->open(resType + "/" + resource->shortFilename())) {
        warnKrita << "Could not open file in store for resource";
        return false;
    }

    bool res = (store->write(buf.data()) == buf.size());
    store->close();
    return res;

}

bool KisResourceBundle::save()
{
    if (filename().isEmpty()) return false;

    addMeta("updated", QDate::currentDate().toString("dd/MM/yyyy"));

    QDir bundleDir = KoResourcePaths::saveLocation("data", "bundles");
    bundleDir.cdUp();

    QScopedPointer<KoStore> store(KoStore::createStore(filename(), KoStore::Write, "application/x-krita-resourcebundle", KoStore::Zip));

    if (!store || store->bad()) return false;

    Q_FOREACH (const QString &resType, m_manifest.types()) {

        if (resType == ResourceType::Gradients) {
            KoResourceServer<KoAbstractGradient>* gradientServer = KoResourceServerProvider::instance()->gradientServer();
            Q_FOREACH (const KisResourceBundleManifest::ResourceReference &ref, m_manifest.files(resType)) {
                KoResourceSP res = gradientServer->resourceByMD5(ref.md5sum);
                if (!res) res = gradientServer->resourceByFilename(QFileInfo(ref.resourcePath).fileName());
                if (!saveResourceToStore(res, store.data(), ResourceType::Gradients)) {
                    if (res) {
                        warnKrita << "Could not save resource" << resType << res->name();
                    }
                    else {
                        warnKrita << "could not find resource for" << QFileInfo(ref.resourcePath).fileName();
                    }
                }
            }
        }
        else if (resType  == ResourceType::Patterns) {
            KoResourceServer<KoPattern>* patternServer = KoResourceServerProvider::instance()->patternServer();
            Q_FOREACH (const KisResourceBundleManifest::ResourceReference &ref, m_manifest.files(resType)) {
                KoResourceSP res = patternServer->resourceByMD5(ref.md5sum);
                if (!res) res = patternServer->resourceByFilename(QFileInfo(ref.resourcePath).fileName());
                if (!saveResourceToStore(res, store.data(), ResourceType::Patterns)) {
                    if (res) {
                        warnKrita << "Could not save resource" << resType << res->name();
                    }
                    else {
                        warnKrita << "could not find resource for" << QFileInfo(ref.resourcePath).fileName();
                    }
                }
            }
        }
        else if (resType  == ResourceType::Brushes) {
            KoResourceServer<KisBrush>* brushServer = KisBrushServerProvider::instance()->brushServer();
            Q_FOREACH (const KisResourceBundleManifest::ResourceReference &ref, m_manifest.files(resType)) {
                KisBrushSP brush = brushServer->resourceByMD5(ref.md5sum);
                if (!brush) brush = brushServer->resourceByFilename(QFileInfo(ref.resourcePath).fileName());
                KoResourceSP res = brush;
                if (!saveResourceToStore(res, store.data(), ResourceType::Brushes)) {
                    if (res) {
                        warnKrita << "Could not save resource" << resType << res->name();
                    }
                    else {
                        warnKrita << "could not find resource for" << QFileInfo(ref.resourcePath).fileName();
                    }
                }
            }
        }
        else if (resType  == ResourceType::Palettes) {
            KoResourceServer<KoColorSet>* paletteServer = KoResourceServerProvider::instance()->paletteServer();
            Q_FOREACH (const KisResourceBundleManifest::ResourceReference &ref, m_manifest.files(resType)) {
                KoResourceSP res = paletteServer->resourceByMD5(ref.md5sum);
                if (!res) res = paletteServer->resourceByFilename(QFileInfo(ref.resourcePath).fileName());
                if (!saveResourceToStore(res, store.data(), ResourceType::Palettes)) {
                    if (res) {
                        warnKrita << "Could not save resource" << resType << res->name();
                    }
                    else {
                        warnKrita << "could not find resource for" << QFileInfo(ref.resourcePath).fileName();
                    }
                }
            }
        }
        else if (resType  == ResourceType::Workspaces) {
            KoResourceServer< KisWorkspaceResource >* workspaceServer = KisResourceServerProvider::instance()->workspaceServer();
            Q_FOREACH (const KisResourceBundleManifest::ResourceReference &ref, m_manifest.files(resType)) {
                KoResourceSP res = workspaceServer->resourceByMD5(ref.md5sum);
                if (!res) res = workspaceServer->resourceByFilename(QFileInfo(ref.resourcePath).fileName());
                if (!saveResourceToStore(res, store.data(), ResourceType::Workspaces)) {
                    if (res) {
                        warnKrita << "Could not save resource" << resType << res->name();
                    }
                    else {
                        warnKrita << "could not find resource for" << QFileInfo(ref.resourcePath).fileName();
                    }
                }
            }
        }
        else if (resType  == ResourceType::PaintOpPresets) {
            KisPaintOpPresetResourceServer* paintoppresetServer = KisResourceServerProvider::instance()->paintOpPresetServer();
            Q_FOREACH (const KisResourceBundleManifest::ResourceReference &ref, m_manifest.files(resType)) {
                KisPaintOpPresetSP res = paintoppresetServer->resourceByMD5(ref.md5sum);
                if (!res) res = paintoppresetServer->resourceByFilename(QFileInfo(ref.resourcePath).fileName());
                if (!saveResourceToStore(res, store.data(), ResourceType::PaintOpPresets)) {
                    if (res) {
                        warnKrita << "Could not save resource" << resType << res->name();
                    }
                    else {
                        warnKrita << "could not find resource for" << QFileInfo(ref.resourcePath).fileName();
                    }
                }
            }
        }
        else if (resType  == ResourceType::GamutMasks) {
            KoResourceServer<KoGamutMask>* gamutMaskServer = KoResourceServerProvider::instance()->gamutMaskServer();
            Q_FOREACH (const KisResourceBundleManifest::ResourceReference &ref, m_manifest.files(resType)) {
                KoResourceSP res = gamutMaskServer->resourceByMD5(ref.md5sum);
                if (!res) res = gamutMaskServer->resourceByFilename(QFileInfo(ref.resourcePath).fileName());
                if (!saveResourceToStore(res, store.data(), ResourceType::GamutMasks)) {
                    if (res) {
                        warnKrita << "Could not save resource" << resType << res->name();
                    }
                    else {
                        warnKrita << "could not find resource for" << QFileInfo(ref.resourcePath).fileName();
                    }
                }
            }
        }
    }

    if (!m_thumbnail.isNull()) {
        QByteArray byteArray;
        QBuffer buffer(&byteArray);
        m_thumbnail.save(&buffer, "PNG");
        if (!store->open("preview.png")) warnKrita << "Could not open preview.png";
        if (store->write(byteArray) != buffer.size()) warnKrita << "Could not write preview.png";
        store->close();
    }

    saveManifest(store);

    saveMetadata(store);

    store->finalize();

    return true;
}

bool KisResourceBundle::saveToDevice(QIODevice */*dev*/) const
{
    return false;
}

void KisResourceBundle::addMeta(const QString &type, const QString &value)
{
    m_metadata.insert(type, value);
}

const QString KisResourceBundle::getMeta(const QString &type, const QString &defaultValue) const
{
    if (m_metadata.contains(type)) {
        return m_metadata[type];
    }
    else {
        return defaultValue;
    }
}

void KisResourceBundle::addResource(QString fileType, QString filePath, QStringList fileTagList, const QByteArray md5sum)
{
    m_manifest.addResource(fileType, filePath, fileTagList, md5sum);
}

QList<QString> KisResourceBundle::getTagsList()
{
    return QList<QString>::fromSet(m_bundletags);
}

QStringList KisResourceBundle::resourceTypes() const
{
    return m_manifest.types();
}

QList<KoResourceSP> KisResourceBundle::resources(const QString &resType) const
{
    QList<KisResourceBundleManifest::ResourceReference> references = m_manifest.files(resType);

    QList<KoResourceSP> ret;
    Q_FOREACH (const KisResourceBundleManifest::ResourceReference &ref, references) {
        if (resType == ResourceType::Gradients) {
            KoResourceServer<KoAbstractGradient>* gradientServer = KoResourceServerProvider::instance()->gradientServer();
            KoResourceSP res =  gradientServer->resourceByMD5(ref.md5sum);
            if (res) ret << res;
        }
        else if (resType  == ResourceType::Patterns) {
            KoResourceServer<KoPattern>* patternServer = KoResourceServerProvider::instance()->patternServer();
            KoResourceSP res =  patternServer->resourceByMD5(ref.md5sum);
            if (res) ret << res;
        }
        else if (resType  == ResourceType::Brushes) {
            KoResourceServer<KisBrush> *brushServer = KisBrushServerProvider::instance()->brushServer();
            KoResourceSP res =  brushServer->resourceByMD5(ref.md5sum);
            if (res) ret << res;
        }
        else if (resType  == ResourceType::Palettes) {
            KoResourceServer<KoColorSet>* paletteServer = KoResourceServerProvider::instance()->paletteServer();
            KoResourceSP res =  paletteServer->resourceByMD5(ref.md5sum);
            if (res) ret << res;
        }
        else if (resType  == ResourceType::Workspaces) {
            KoResourceServer< KisWorkspaceResource >* workspaceServer = KisResourceServerProvider::instance()->workspaceServer();
            KoResourceSP res =  workspaceServer->resourceByMD5(ref.md5sum);
            if (res) ret << res;
        }
        else if (resType  == ResourceType::PaintOpPresets) {
            KisPaintOpPresetResourceServer* paintoppresetServer = KisResourceServerProvider::instance()->paintOpPresetServer();
            KisPaintOpPresetSP res =  paintoppresetServer->resourceByMD5(ref.md5sum);
            if (res) ret << res;
        }
        else if (resType  == ResourceType::GamutMasks) {
            KoResourceServer<KoGamutMask>* gamutMaskServer = KoResourceServerProvider::instance()->gamutMaskServer();
            KoResourceSP res =  gamutMaskServer->resourceByMD5(ref.md5sum);
            if (res) ret << res;
        }
    }
    return ret;
}

void KisResourceBundle::setThumbnail(QString filename)
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

    setImage(m_thumbnail);
}

void KisResourceBundle::writeMeta(const char *metaTag, const QString &metaKey, KoXmlWriter *writer)
{
    if (m_metadata.contains(metaKey)) {
        writer->startElement(metaTag);
        writer->addTextNode(m_metadata[metaKey].toUtf8());
        writer->endElement();
    }
}

void KisResourceBundle::writeUserDefinedMeta(const QString &metaKey, KoXmlWriter *writer)
{
    if (m_metadata.contains(metaKey)) {
        writer->startElement("meta:meta-userdefined");
        writer->addAttribute("meta:name", metaKey);
        writer->addAttribute("meta:value", m_metadata[metaKey]);
        writer->endElement();
    }
}

void KisResourceBundle::saveMetadata(QScopedPointer<KoStore> &store)
{
    QBuffer buf;

    store->open("meta.xml");
    buf.open(QBuffer::WriteOnly);

    KoXmlWriter metaWriter(&buf);
    metaWriter.startDocument("office:document-meta");
    metaWriter.startElement("meta:meta");

    writeMeta("meta:generator", "generator", &metaWriter);

    metaWriter.startElement("meta:bundle-version");
    metaWriter.addTextNode(m_bundleVersion.toUtf8());
    metaWriter.endElement();

    writeMeta("dc:author", "author", &metaWriter);
    writeMeta("dc:title", "filename", &metaWriter);
    writeMeta("dc:description", "description", &metaWriter);
    writeMeta("meta:initial-creator", "author", &metaWriter);
    writeMeta("dc:creator", "author", &metaWriter);
    writeMeta("meta:creation-date", "created", &metaWriter);
    writeMeta("meta:dc-date", "updated", &metaWriter);
    writeUserDefinedMeta("email", &metaWriter);
    writeUserDefinedMeta("license", &metaWriter);
    writeUserDefinedMeta("website", &metaWriter);
    Q_FOREACH (const QString &tag, m_bundletags) {
        metaWriter.startElement("meta:meta-userdefined");
        metaWriter.addAttribute("meta:name", "tag");
        metaWriter.addAttribute("meta:value", tag);
        metaWriter.endElement();
    }

    metaWriter.endElement(); // meta:meta
    metaWriter.endDocument();

    buf.close();
    store->write(buf.data());
    store->close();
}

void KisResourceBundle::saveManifest(QScopedPointer<KoStore> &store)
{
    store->open("META-INF/manifest.xml");
    QBuffer buf;
    buf.open(QBuffer::WriteOnly);
    m_manifest.save(&buf);
    buf.close();
    store->write(buf.data());
    store->close();
}

void KisResourceBundle::recreateBundle(QScopedPointer<KoStore> &oldStore)
{
    // Save a copy of the unmodified bundle, so that if anything goes bad the user doesn't lose it
    QFile file(filename());
    file.copy(filename() + ".old");

    QString newStoreName = filename() + ".tmp";
    {
        QScopedPointer<KoStore> store(KoStore::createStore(newStoreName, KoStore::Write, "application/x-krita-resourcebundle", KoStore::Zip));
        KoHashGenerator *generator = KoHashGeneratorProvider::instance()->getGenerator("MD5");
        KisResourceBundleManifest newManifest;

        addMeta("updated", QDate::currentDate().toString("dd/MM/yyyy"));

        Q_FOREACH (KisResourceBundleManifest::ResourceReference ref, m_manifest.files()) {
            // Wrong manifest entry found, skip it
            if(!oldStore->open(ref.resourcePath))
                continue;

            store->open(ref.resourcePath);

            QByteArray data = oldStore->device()->readAll();
            oldStore->close();
            store->write(data);
            store->close();
            QByteArray result = generator->generateHash(data);
            newManifest.addResource(ref.fileTypeName, ref.resourcePath, ref.tagList, result);
        }

        m_manifest = newManifest;

        if (!m_thumbnail.isNull()) {
            QByteArray byteArray;
            QBuffer buffer(&byteArray);
            m_thumbnail.save(&buffer, "PNG");
            if (!store->open("preview.png")) warnKrita << "Could not open preview.png";
            if (store->write(byteArray) != buffer.size()) warnKrita << "Could not write preview.png";
            store->close();
        }

        saveManifest(store);
        saveMetadata(store);

        store->finalize();
    }
    // Remove the current bundle and then move the tmp one to be the correct one
    file.setFileName(filename());
    if (!file.remove()) {
        qWarning() << "Could not remove" << filename() << file.errorString();
    }
    QFile f(newStoreName);
    Q_ASSERT(f.exists());
    if (!f.copy(filename())) {
        qWarning() << "Could not copy the tmp file to the store" << filename() << newStoreName << QFile(newStoreName).exists() << f.errorString();
    }
}


int KisResourceBundle::resourceCount() const
{
    return m_manifest.files().count();
}
