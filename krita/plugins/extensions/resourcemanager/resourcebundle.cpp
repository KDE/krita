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

#include "resourcebundle.h"
#include "resourcebundle_manifest.h"

#include <KoXmlReader.h>
#include <KoXmlWriter.h>
#include <KoStore.h>
#include <KoResourceServerProvider.h>
#include <KoResourceTagStore.h>

#include <kglobal.h>
#include <kcomponentdata.h>
#include <kstandarddirs.h>

#include <QScopedPointer>
#include <QProcessEnvironment>
#include <QDate>
#include <QDir>
#include <QDebug>
#include <QBuffer>
#include <QCryptographicHash>
#include <QByteArray>
#include <QPainter>

#include <kis_resource_server_provider.h>
#include <kis_workspace_resource.h>
#include <kis_paintop_preset.h>
#include <kis_brush_server.h>
#include <kis_debug.h>

#include <calligraversion.h>
#include <calligragitversion.h>

#include "resourcemanager.h"


ResourceBundle::ResourceBundle(QString const& fileName)
    : KoResource(fileName)
{
    setName(QFileInfo(fileName).baseName());

    QString calligraVersion(CALLIGRA_VERSION_STRING);
    QString version;


#ifdef CALLIGRA_GIT_SHA1_STRING
    QString gitVersion(CALLIGRA_GIT_SHA1_STRING);
    version = QString("%1 (git %2)").arg(calligraVersion).arg(gitVersion).toLatin1();
#else
    version = calligraVersion;
#endif
    m_metadata["generator"] = "Krita (" + version + ")";
}

ResourceBundle::~ResourceBundle()
{
}

QString ResourceBundle::defaultFileExtension() const
{
    return QString(".bundle");
}

bool ResourceBundle::load()
{
    if (filename().isEmpty()) return false;
    QScopedPointer<KoStore> resourceStore(KoStore::createStore(filename(), KoStore::Read, "application/x-krita-resourcebundle", KoStore::Zip));

    if (!resourceStore || resourceStore->bad()) {
        qWarning() << "Could not open store on bundle" << filename();
        m_installed = false;
        setValid(false);
        return false;

    } else {

        m_metadata.clear();

        if (resourceStore->open("META-INF/manifest.xml")) {
            if (!m_manifest.load(resourceStore->device())) {
                qWarning() << "Could not open manifest for bundle" << filename();
                return false;
            }
            resourceStore->close();

            foreach(ResourceBundleManifest::ResourceReference ref, m_manifest.files()) {
                if (!resourceStore->open(ref.resourcePath)) {
                    qWarning() << "Bundle is broken. File" << ref.resourcePath << "is missing";
                    return false;
                }
                resourceStore->close();
            }
        } else {
            qWarning() << "Could not load META-INF/manifest.xml";
            return false;
        }

        if (resourceStore->open("meta.xml")) {
            KoXmlDocument doc;
            if (!doc.setContent(resourceStore->device())) {
                qWarning() << "Could not parse meta.xml for" << filename();
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
                qWarning() << "Could not find manifest node for bundle" << filename();
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
                }
            }
            resourceStore->close();
        }
        else {
            qWarning() << "Could not load meta.xml";
            return false;
        }

        if (resourceStore->open("preview.png")) {
            m_thumbnail.load(resourceStore->device(), "PNG");
            resourceStore->close();
        }
        else {
            qWarning() << "Could not open preview.png";
        }

        m_installed = true;
        setValid(true);
        setImage(m_thumbnail);
    }

    return true;
}

bool ResourceBundle::loadFromDevice(QIODevice *)
{
    return false;
}

bool saveResourceToStore(KoResource *resource, KoStore *store, const QString &resType)
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
    if (fi.exists() && fi.isReadable() && !fi.isWritable()) {

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

        if (!buf.open(QBuffer::WriteOnly)) {
            qWarning() << "Could not open buffer";
            return false;
        }
        if (!resource->saveToDevice(&buf)) {
            qWarning() << "Could not save resource to buffer";
            return false;
        }
        buf.close();
    }

    if (!buf.open(QBuffer::ReadOnly)) {
        qWarning() << "Could not open buffer";
        return false;
    }
    Q_ASSERT(!store->hasFile(resType + "/" + resource->shortFilename()));
    if (!store->open(resType + "/" + resource->shortFilename())) {
        qWarning() << "Could not open file in store for resource";
        return false;
    }

    bool res = (store->write(buf.data()) == buf.size());
    store->close();
    return res;

}

bool ResourceBundle::save()
{
    if (filename().isEmpty()) return false;

    addMeta("updated", QDate::currentDate().toString("dd/MM/yyyy"));

    QDir bundleDir = KGlobal::dirs()->saveLocation("appdata", "bundles");
    bundleDir.cdUp();

    QScopedPointer<KoStore> store(KoStore::createStore(filename(), KoStore::Write, "application/x-krita-resourcebundle", KoStore::Zip));

    if (!store || store->bad()) return false;

    foreach(const QString &resType, m_manifest.types()) {

        if (resType == "ko_gradients") {
            KoResourceServer<KoAbstractGradient>* gradientServer = KoResourceServerProvider::instance()->gradientServer();
            foreach(const ResourceBundleManifest::ResourceReference &ref, m_manifest.files(resType)) {
                KoResource *res = gradientServer->resourceByMD5(ref.md5sum);
                if (!res) res = gradientServer->resourceByFilename(QFileInfo(ref.resourcePath).fileName());
                if (!saveResourceToStore(res, store.data(), "gradients")) {
                    if (res) {
                        qWarning() << "Could not save resource" << resType << res->name();
                    }
                    else {
                        qWarning() << "could not find resource for" << QFileInfo(ref.resourcePath).fileName();
                    }
                }
            }
        }
        else if (resType  == "ko_patterns") {
            KoResourceServer<KoPattern>* patternServer = KoResourceServerProvider::instance()->patternServer();
            foreach(const ResourceBundleManifest::ResourceReference &ref, m_manifest.files(resType)) {
                KoResource *res = patternServer->resourceByMD5(ref.md5sum);
                if (!res) res = patternServer->resourceByFilename(QFileInfo(ref.resourcePath).fileName());
                if (!saveResourceToStore(res, store.data(), "patterns")) {
                    if (res) {
                        qWarning() << "Could not save resource" << resType << res->name();
                    }
                    else {
                        qWarning() << "could not find resource for" << QFileInfo(ref.resourcePath).fileName();
                    }
                }
            }
        }
        else if (resType  == "kis_brushes") {
            KisBrushResourceServer* brushServer = KisBrushServer::instance()->brushServer();
            foreach(const ResourceBundleManifest::ResourceReference &ref, m_manifest.files(resType)) {
                KisBrushSP brush = brushServer->resourceByMD5(ref.md5sum);
                if (!brush) brush = brushServer->resourceByFilename(QFileInfo(ref.resourcePath).fileName());
                KoResource *res = brush.data();
                if (!saveResourceToStore(res, store.data(), "brushes")) {
                    if (res) {
                        qWarning() << "Could not save resource" << resType << res->name();
                    }
                    else {
                        qWarning() << "could not find resource for" << QFileInfo(ref.resourcePath).fileName();
                    }
                }
            }
        }
        else if (resType  == "ko_palettes") {
            KoResourceServer<KoColorSet>* paletteServer = KoResourceServerProvider::instance()->paletteServer();
            foreach(const ResourceBundleManifest::ResourceReference &ref, m_manifest.files(resType)) {
                KoResource *res = paletteServer->resourceByMD5(ref.md5sum);
                if (!res) res = paletteServer->resourceByFilename(QFileInfo(ref.resourcePath).fileName());
                if (!saveResourceToStore(res, store.data(), "palettes")) {
                    if (res) {
                        qWarning() << "Could not save resource" << resType << res->name();
                    }
                    else {
                        qWarning() << "could not find resource for" << QFileInfo(ref.resourcePath).fileName();
                    }
                }
            }
        }
        else if (resType  == "kis_workspaces") {
            KoResourceServer< KisWorkspaceResource >* workspaceServer = KisResourceServerProvider::instance()->workspaceServer();
            foreach(const ResourceBundleManifest::ResourceReference &ref, m_manifest.files(resType)) {
                KoResource *res = workspaceServer->resourceByMD5(ref.md5sum);
                if (!res) res = workspaceServer->resourceByFilename(QFileInfo(ref.resourcePath).fileName());
                if (!saveResourceToStore(res, store.data(), "workspaces")) {
                    if (res) {
                        qWarning() << "Could not save resource" << resType << res->name();
                    }
                    else {
                        qWarning() << "could not find resource for" << QFileInfo(ref.resourcePath).fileName();
                    }
                }
            }
        }
        else if (resType  == "kis_paintoppresets") {
            KisPaintOpPresetResourceServer* paintoppresetServer = KisResourceServerProvider::instance()->paintOpPresetServer();
            foreach(const ResourceBundleManifest::ResourceReference &ref, m_manifest.files(resType)) {
                KisPaintOpPresetSP res = paintoppresetServer->resourceByMD5(ref.md5sum);
                if (!res) res = paintoppresetServer->resourceByFilename(QFileInfo(ref.resourcePath).fileName());
                if (!saveResourceToStore(res.data(), store.data(), "paintoppresets")) {
                    if (res) {
                        qWarning() << "Could not save resource" << resType << res->name();
                    }
                    else {
                        qWarning() << "could not find resource for" << QFileInfo(ref.resourcePath).fileName();
                    }
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

    {
        store->open("META-INF/manifest.xml");
        QBuffer buf;
        buf.open(QBuffer::WriteOnly);
        m_manifest.save(&buf);
        buf.close();
        store->write(buf.data());
        store->close();
    }

    {
        QBuffer buf;

        store->open("meta.xml");
        buf.open(QBuffer::WriteOnly);

        KoXmlWriter metaWriter(&buf);
        metaWriter.startDocument("office:document-meta");
        metaWriter.startElement("meta:meta");

        writeMeta("meta:generator", "generator", &metaWriter);
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
        foreach (const QString &tag, m_bundletags) {
            metaWriter.startElement("meta:meta-userdefined");
            metaWriter.addAttribute("meta:name", "tag");
            metaWriter.addAttribute("meta:value", tag);
            metaWriter.endElement();
        }

        metaWriter.endElement(); // meta:meta
        metaWriter.endDocument();

        buf.close();
        store->write(buf.data());
    }
    store->close();
    store->finalize();

    return true;
}

bool ResourceBundle::saveToDevice(QIODevice */*dev*/) const
{
    return false;
}

bool ResourceBundle::install()
{
    if (filename().isEmpty())  {
        qWarning() << "Cannot install bundle: no file name" << this;
        return false;
    }
    QScopedPointer<KoStore> resourceStore(KoStore::createStore(filename(), KoStore::Read, "application/x-krita-resourcebundle", KoStore::Zip));

    if (!resourceStore || resourceStore->bad()) {
        qWarning() << "Cannot open the resource bundle: invalid zip file?";
        return false;
    }

    foreach(const QString &resType, m_manifest.types()) {
        dbgResources << "Installing resource type" << resType;
        if (resType == "gradients") {
            KoResourceServer<KoAbstractGradient>* gradientServer = KoResourceServerProvider::instance()->gradientServer();
            foreach(const ResourceBundleManifest::ResourceReference &ref, m_manifest.files(resType)) {

                if (resourceStore->isOpen()) resourceStore->close();

                dbgResources << "\tInstalling" << ref.resourcePath;
                KoAbstractGradient *res = gradientServer->createResource(ref.resourcePath);
                if (!res) {
                    qWarning() << "Could not create resource for" << ref.resourcePath;
                    continue;
                }
                if (!resourceStore->open(ref.resourcePath)) {
                    qWarning() << "Failed to open" << ref.resourcePath << "from bundle" << filename();
                    continue;
                }
                if (!res->loadFromDevice(resourceStore->device())) {
                    qWarning() << "Failed to load" << ref.resourcePath << "from bundle" << filename();
                    continue;
                }
                dbgResources << "\t\tresource:" << res->name();
                gradientServer->addResource(res, false);
                foreach(const QString &tag, ref.tagList) {
                    gradientServer->addTag(res, tag);
                }
                gradientServer->addTag(res, name());
            }
        }
        else if (resType  == "patterns") {
            KoResourceServer<KoPattern>* patternServer = KoResourceServerProvider::instance()->patternServer();
            foreach(const ResourceBundleManifest::ResourceReference &ref, m_manifest.files(resType)) {

                if (resourceStore->isOpen()) resourceStore->close();

                dbgResources << "\tInstalling" << ref.resourcePath;
                KoPattern *res = patternServer->createResource(ref.resourcePath);
                if (!res) {
                    qWarning() << "Could not create resource for" << ref.resourcePath;
                    continue;
                }
                if (!resourceStore->open(ref.resourcePath)) {
                    qWarning() << "Failed to open" << ref.resourcePath << "from bundle" << filename();
                    continue;
                }
                if (!res->loadFromDevice(resourceStore->device())) {
                    qWarning() << "Failed to load" << ref.resourcePath << "from bundle" << filename();
                    continue;
                }
                patternServer->addResource(res, false);
                foreach(const QString &tag, ref.tagList) {
                    patternServer->addTag(res, tag);
                }
                patternServer->addTag(res, name());
            }
        }
        else if (resType  == "brushes") {
            KisBrushResourceServer *brushServer = KisBrushServer::instance()->brushServer();
            foreach(const ResourceBundleManifest::ResourceReference &ref, m_manifest.files(resType)) {

                if (resourceStore->isOpen()) resourceStore->close();

                dbgResources << "\tInstalling" << ref.resourcePath;
                KisBrushSP res = brushServer->createResource(ref.resourcePath);
                if (!res) {
                    qWarning() << "Could not create resource for" << ref.resourcePath;
                    continue;
                }
                if (!resourceStore->open(ref.resourcePath)) {
                    qWarning() << "Failed to open" << ref.resourcePath << "from bundle" << filename();
                    continue;
                }
                if (!res->loadFromDevice(resourceStore->device())) {
                    qWarning() << "Failed to load" << ref.resourcePath << "from bundle" << filename();
                    continue;
                }
                dbgResources << "\t\tresource:" << res->name();
                brushServer->addResource(res, false);
                foreach(const QString &tag, ref.tagList) {
                    brushServer->addTag(res.data(), tag);
                }
                brushServer->addTag(res.data(), name());
            }
        }
        else if (resType  == "palettes") {
            KoResourceServer<KoColorSet>* paletteServer = KoResourceServerProvider::instance()->paletteServer();
            foreach(const ResourceBundleManifest::ResourceReference &ref, m_manifest.files(resType)) {

                if (resourceStore->isOpen()) resourceStore->close();

                dbgResources << "\tInstalling" << ref.resourcePath;
                KoColorSet *res = paletteServer->createResource(ref.resourcePath);

                if (!res) {
                    qWarning() << "Could not create resource for" << ref.resourcePath;
                    continue;
                }
                if (!resourceStore->open(ref.resourcePath)) {
                    qWarning() << "Failed to open" << ref.resourcePath << "from bundle" << filename();
                    continue;
                }
                if (!res->loadFromDevice(resourceStore->device())) {
                    qWarning() << "Failed to load" << ref.resourcePath << "from bundle" << filename();
                    continue;
                }
                dbgResources << "\t\tresource:" << res->name();
                paletteServer->addResource(res, false);
                foreach(const QString &tag, ref.tagList) {
                    paletteServer->addTag(res, tag);
                }
                paletteServer->addTag(res, name());
            }
        }
        else if (resType  == "workspaces") {
            KoResourceServer< KisWorkspaceResource >* workspaceServer = KisResourceServerProvider::instance()->workspaceServer();
            foreach(const ResourceBundleManifest::ResourceReference &ref, m_manifest.files(resType)) {

                if (resourceStore->isOpen()) resourceStore->close();

                dbgResources << "\tInstalling" << ref.resourcePath;
                KisWorkspaceResource *res = workspaceServer->createResource(ref.resourcePath);
                if (!res) {
                    qWarning() << "Could not create resource for" << ref.resourcePath;
                    continue;
                }
                if (!resourceStore->open(ref.resourcePath)) {
                    qWarning() << "Failed to open" << ref.resourcePath << "from bundle" << filename();
                    continue;
                }
                if (!res->loadFromDevice(resourceStore->device())) {
                    qWarning() << "Failed to load" << ref.resourcePath << "from bundle" << filename();
                    continue;
                }
                dbgResources << "\t\tresource:" << res->name();
                workspaceServer->addResource(res, false);
                foreach(const QString &tag, ref.tagList) {
                    workspaceServer->addTag(res, tag);
                }
                workspaceServer->addTag(res, name());
            }
        }
        else if (resType  == "paintoppresets") {
            KisPaintOpPresetResourceServer*  paintoppresetServer = KisResourceServerProvider::instance()->paintOpPresetServer();
            foreach(const ResourceBundleManifest::ResourceReference &ref, m_manifest.files(resType)) {

                if (resourceStore->isOpen()) resourceStore->close();

                dbgResources << "\tInstalling" << ref.resourcePath;
                KisPaintOpPresetSP res = paintoppresetServer->createResource(ref.resourcePath);
                if (!res) {
                    qWarning() << "Could not create resource for" << ref.resourcePath;
                    continue;
                }
                if (!resourceStore->open(ref.resourcePath)) {
                    qWarning() << "Failed to open" << ref.resourcePath << "from bundle" << filename();
                    continue;
                }
                if (!res->loadFromDevice(resourceStore->device())) {
                    qWarning() << "Failed to load" << ref.resourcePath << "from bundle" << filename();
                    continue;
                }
                dbgResources << "\t\tresource:" << res->name();
                paintoppresetServer->addResource(res, false);
                foreach(const QString &tag, ref.tagList) {
                    paintoppresetServer->addTag(res.data(), tag);
                }
                paintoppresetServer->addTag(res.data(), name());
            }
        }
    }
    m_installed = true;
    return true;
}

bool ResourceBundle::uninstall()
{
    m_installed = false;


    KoResourceServer<KoAbstractGradient>* gradientServer = KoResourceServerProvider::instance()->gradientServer();
    foreach(const ResourceBundleManifest::ResourceReference &ref, m_manifest.files("gradients")) {
        KoAbstractGradient *res = gradientServer->resourceByMD5(ref.md5sum);
        if (res) {
            gradientServer->removeResourceFromServer(res);
        }
    }

    KoResourceServer<KoPattern>* patternServer = KoResourceServerProvider::instance()->patternServer();
    foreach(const ResourceBundleManifest::ResourceReference &ref, m_manifest.files("patterns")) {
        KoPattern *res = patternServer->resourceByMD5(ref.md5sum);
        if (res) {
            patternServer->removeResourceFromServer(res);
        }
    }

    KisBrushResourceServer *brushServer = KisBrushServer::instance()->brushServer();
    foreach(const ResourceBundleManifest::ResourceReference &ref, m_manifest.files("brushes")) {
        KisBrushSP res = brushServer->resourceByMD5(ref.md5sum);
        if (res) {
            brushServer->removeResourceFromServer(res);
        }
    }

    KoResourceServer<KoColorSet>* paletteServer = KoResourceServerProvider::instance()->paletteServer();
    foreach(const ResourceBundleManifest::ResourceReference &ref, m_manifest.files("palettes")) {
        KoColorSet *res = paletteServer->resourceByMD5(ref.md5sum);
        if (res) {
            paletteServer->removeResourceFromServer(res);
        }
    }
    KoResourceServer< KisWorkspaceResource >* workspaceServer = KisResourceServerProvider::instance()->workspaceServer();
    foreach(const ResourceBundleManifest::ResourceReference &ref, m_manifest.files("workspaces")) {
        KisWorkspaceResource *res = workspaceServer->resourceByMD5(ref.md5sum);
        if (res) {
            workspaceServer->removeResourceFromServer(res);
        }
    }

    KisPaintOpPresetResourceServer* paintoppresetServer = KisResourceServerProvider::instance()->paintOpPresetServer();
    foreach(const ResourceBundleManifest::ResourceReference &ref, m_manifest.files("workspaces")) {
        KisPaintOpPresetSP res = paintoppresetServer->resourceByMD5(ref.md5sum);
        if (res) {
            paintoppresetServer->removeResourceFromServer(res);
        }
    }

    return true;
}

void ResourceBundle::addMeta(const QString &type, const QString &value)
{
    m_metadata.insert(type, value);
}

const QString ResourceBundle::getMeta(const QString &type, const QString &defaultValue) const
{
    if (m_metadata.contains(type)) {
        return m_metadata[type];
    }
    else {
       return defaultValue;
    }
}


void ResourceBundle::addResource(QString fileType, QString filePath, QStringList fileTagList, const QByteArray md5sum)
{
    m_manifest.addResource(fileType, filePath, fileTagList, md5sum);
}

QList<QString> ResourceBundle::getTagsList()
{
    return QList<QString>::fromSet(m_bundletags);
}


bool ResourceBundle::isInstalled()
{
    return m_installed;
}


QStringList ResourceBundle::resourceTypes()
{
    return m_manifest.types();
}

QList<KoResource*> ResourceBundle::resources(const QString &resType)
{
    QList<ResourceBundleManifest::ResourceReference> references = m_manifest.files(resType);

    QList<KoResource*> ret;
    foreach(const ResourceBundleManifest::ResourceReference &ref, references) {
        if (resType == "gradients") {
            KoResourceServer<KoAbstractGradient>* gradientServer = KoResourceServerProvider::instance()->gradientServer();
            KoResource *res =  gradientServer->resourceByMD5(ref.md5sum);
            if (res) ret << res;
        }
        else if (resType  == "patterns") {
            KoResourceServer<KoPattern>* patternServer = KoResourceServerProvider::instance()->patternServer();
            KoResource *res =  patternServer->resourceByMD5(ref.md5sum);
            if (res) ret << res;
        }
        else if (resType  == "brushes") {
            KisBrushResourceServer *brushServer = KisBrushServer::instance()->brushServer();
            KoResource *res =  brushServer->resourceByMD5(ref.md5sum).data();
            if (res) ret << res;
        }
        else if (resType  == "palettes") {
            KoResourceServer<KoColorSet>* paletteServer = KoResourceServerProvider::instance()->paletteServer();
            KoResource *res =  paletteServer->resourceByMD5(ref.md5sum);
            if (res) ret << res;
        }
        else if (resType  == "workspaces") {
            KoResourceServer< KisWorkspaceResource >* workspaceServer = KisResourceServerProvider::instance()->workspaceServer();
            KoResource *res =  workspaceServer->resourceByMD5(ref.md5sum);
            if (res) ret << res;
        }
        else if (resType  == "paintoppresets") {
            KisPaintOpPresetResourceServer* paintoppresetServer = KisResourceServerProvider::instance()->paintOpPresetServer();
            KisPaintOpPresetSP res =  paintoppresetServer->resourceByMD5(ref.md5sum);
            if (res) ret << res.data();
        }
    }
    return ret;
}

void ResourceBundle::setThumbnail(QString filename)
{
    if (QFileInfo(filename).exists()) {
        m_thumbnail = QImage(filename);
        m_thumbnail = m_thumbnail.scaled(256, 256, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        Q_ASSERT(!m_thumbnail.isNull());
    }
    else {
        m_thumbnail = QImage(256, 256, QImage::Format_ARGB32);
        QPainter gc(&m_thumbnail);
        gc.fillRect(0, 0, 256, 256, Qt::red);
        gc.end();
    }
}


QByteArray ResourceBundle::generateMD5() const
{
    QFile f(filename());
    if (f.exists()) {
        f.open(QFile::ReadOnly);
        QByteArray ba = f.readAll();
        QCryptographicHash md5(QCryptographicHash::Md5);
        md5.addData(ba);
        return md5.result();
    }
    return QByteArray();
}

void ResourceBundle::writeMeta(const char *metaTag, const QString &metaKey, KoXmlWriter *writer)
{
    if (m_metadata.contains(metaKey)) {
        writer->startElement(metaTag);
        writer->addTextNode(m_metadata[metaKey].toUtf8());
        writer->endElement();
    }
}

void ResourceBundle::writeUserDefinedMeta(const QString &metaKey, KoXmlWriter *writer)
{
    if (m_metadata.contains(metaKey)) {
        writer->startElement("meta:meta-userdefined");
        writer->addAttribute("meta:name", metaKey);
        writer->addAttribute("meta:value", m_metadata[metaKey]);
        writer->endElement();
    }
}
