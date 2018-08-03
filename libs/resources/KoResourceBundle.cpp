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

#include "KoResourceBundleManifest.h"
#include <KoHashGenerator.h>
#include <KoHashGeneratorProvider.h>
#include <KoResourcePaths.h>
#include <KoStore.h>
#include <KoXmlReader.h>
#include <KoXmlWriter.h>
#include "KisStoragePlugin.h"

#include <KritaVersionWrapper.h>

#include <kis_debug.h>

KoResourceBundle::KoResourceBundle(QString const& fileName)
    : KoResource(fileName),
      m_bundleVersion("1")
{
    setName(QFileInfo(fileName).baseName());
    m_metadata[KisStoragePlugin::s_meta_generator] = "Krita (" + KritaVersionWrapper::versionString(true) + ")";

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
    if (filename().isEmpty()) return false;
    QScopedPointer<KoStore> resourceStore(KoStore::createStore(filename(), KoStore::Read, "application/x-krita-resourcebundle", KoStore::Zip));

    if (!resourceStore || resourceStore->bad()) {
        qWarning() << "Could not open store on bundle" << filename();
        m_installed = false;
        setValid(false);
        return false;

    }
    else {

        m_metadata.clear();

        if (resourceStore->open("META-INF/manifest.xml")) {
            if (!m_manifest.load(resourceStore->device())) {
                qWarning() << "Could not open manifest for bundle" << filename();
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
            m_metadata.insert(KisStoragePlugin::s_meta_version, "1");
        }

        m_installed = true;
        setValid(true);
        setImage(m_thumbnail);
    }

    return true;
}

bool KoResourceBundle::loadFromDevice(QIODevice *)
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
    Q_ASSERT(!store->hasFile(resType + "/" + resource->shortFilename()));
    if (!store->open(resType + "/" + resource->shortFilename())) {
        qWarning() << "Could not open file in store for resource";
        return false;
    }

    bool res = (store->write(buf.data()) == buf.size());
    store->close();
    return res;

}

bool KoResourceBundle::save()
{
    if (filename().isEmpty()) return false;

    setMetaData(KisStoragePlugin::s_meta_dc_date, QDate::currentDate().toString("dd/MM/yyyy"));

    QDir bundleDir = KoResourcePaths::saveLocation("data", "bundles");
    bundleDir.cdUp();

    QScopedPointer<KoStore> store(KoStore::createStore(filename(), KoStore::Write, "application/x-krita-resourcebundle", KoStore::Zip));

    if (!store || store->bad()) return false;

    //    Q_FOREACH (const QString &resType, m_manifest.types()) {

    //        if (resType == "ko_gradients") {
    //            KoResourceServer<KoAbstractGradient>* gradientServer = KoResourceServerProvider::instance()->gradientServer();
    //            Q_FOREACH (const KoResourceBundleManifest::ResourceReference &ref, m_manifest.files(resType)) {
    //                KoResource *res = gradientServer->resourceByMD5(ref.md5sum);
    //                if (!res) res = gradientServer->resourceByFilename(QFileInfo(ref.resourcePath).fileName());
    //                if (!saveResourceToStore(res, store.data(), "gradients")) {
    //                    if (res) {
    //                        qWarning() << "Could not save resource" << resType << res->name();
    //                    }
    //                    else {
    //                        qWarning() << "could not find resource for" << QFileInfo(ref.resourcePath).fileName();
    //                    }
    //                }
    //            }
    //        }
    //        else if (resType  == "ko_patterns") {
    //            KoResourceServer<KoPattern>* patternServer = KoResourceServerProvider::instance()->patternServer();
    //            Q_FOREACH (const KoResourceBundleManifest::ResourceReference &ref, m_manifest.files(resType)) {
    //                KoResource *res = patternServer->resourceByMD5(ref.md5sum);
    //                if (!res) res = patternServer->resourceByFilename(QFileInfo(ref.resourcePath).fileName());
    //                if (!saveResourceToStore(res, store.data(), "patterns")) {
    //                    if (res) {
    //                        qWarning() << "Could not save resource" << resType << res->name();
    //                    }
    //                    else {
    //                        qWarning() << "could not find resource for" << QFileInfo(ref.resourcePath).fileName();
    //                    }
    //                }
    //            }
    //        }
    //        else if (resType  == "kis_brushes") {
    //            KisBrushResourceServer* brushServer = KisBrushServer::instance()->brushServer();
    //            Q_FOREACH (const KoResourceBundleManifest::ResourceReference &ref, m_manifest.files(resType)) {
    //                KisBrushSP brush = brushServer->resourceByMD5(ref.md5sum);
    //                if (!brush) brush = brushServer->resourceByFilename(QFileInfo(ref.resourcePath).fileName());
    //                KoResource *res = brush.data();
    //                if (!saveResourceToStore(res, store.data(), "brushes")) {
    //                    if (res) {
    //                        qWarning() << "Could not save resource" << resType << res->name();
    //                    }
    //                    else {
    //                        qWarning() << "could not find resource for" << QFileInfo(ref.resourcePath).fileName();
    //                    }
    //                }
    //            }
    //        }
    //        else if (resType  == "ko_palettes") {
    //            KoResourceServer<KoColorSet>* paletteServer = KoResourceServerProvider::instance()->paletteServer();
    //            Q_FOREACH (const KoResourceBundleManifest::ResourceReference &ref, m_manifest.files(resType)) {
    //                KoResource *res = paletteServer->resourceByMD5(ref.md5sum);
    //                if (!res) res = paletteServer->resourceByFilename(QFileInfo(ref.resourcePath).fileName());
    //                if (!saveResourceToStore(res, store.data(), "palettes")) {
    //                    if (res) {
    //                        qWarning() << "Could not save resource" << resType << res->name();
    //                    }
    //                    else {
    //                        qWarning() << "could not find resource for" << QFileInfo(ref.resourcePath).fileName();
    //                    }
    //                }
    //            }
    //        }
    //        else if (resType  == "kis_workspaces") {
    //            KoResourceServer< KisWorkspaceResource >* workspaceServer = KisResourceServerProvider::instance()->workspaceServer();
    //            Q_FOREACH (const KoResourceBundleManifest::ResourceReference &ref, m_manifest.files(resType)) {
    //                KoResource *res = workspaceServer->resourceByMD5(ref.md5sum);
    //                if (!res) res = workspaceServer->resourceByFilename(QFileInfo(ref.resourcePath).fileName());
    //                if (!saveResourceToStore(res, store.data(), "workspaces")) {
    //                    if (res) {
    //                        qWarning() << "Could not save resource" << resType << res->name();
    //                    }
    //                    else {
    //                        qWarning() << "could not find resource for" << QFileInfo(ref.resourcePath).fileName();
    //                    }
    //                }
    //            }
    //        }
    //        else if (resType  == "kis_paintoppresets") {
    //            KisPaintOpPresetResourceServer* paintoppresetServer = KisResourceServerProvider::instance()->paintOpPresetServer();
    //            Q_FOREACH (const KoResourceBundleManifest::ResourceReference &ref, m_manifest.files(resType)) {
    //                KisPaintOpPresetSP res = paintoppresetServer->resourceByMD5(ref.md5sum);
    //                if (!res) res = paintoppresetServer->resourceByFilename(QFileInfo(ref.resourcePath).fileName());
    //                if (!saveResourceToStore(res.data(), store.data(), "paintoppresets")) {
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

bool KoResourceBundle::install()
{
    QStringList md5Mismatch;
    if (filename().isEmpty())  {
        qWarning() << "Cannot install bundle: no file name" << this;
        return false;
    }
    QScopedPointer<KoStore> resourceStore(KoStore::createStore(filename(), KoStore::Read, "application/x-krita-resourcebundle", KoStore::Zip));

    if (!resourceStore || resourceStore->bad()) {
        qWarning() << "Cannot open the resource bundle: invalid zip file?";
        return false;
    }

    //    Q_FOREACH (const QString &resType, m_manifest.types()) {
    //        dbgResources << "Installing resource type" << resType;
    //        if (resType == "gradients") {
    //            KoResourceServer<KoAbstractGradient>* gradientServer = KoResourceServerProvider::instance()->gradientServer();
    //            Q_FOREACH (const KoResourceBundleManifest::ResourceReference &ref, m_manifest.files(resType)) {

    //                if (resourceStore->isOpen()) resourceStore->close();

    //                dbgResources << "\tInstalling" << ref.resourcePath;
    //                KoAbstractGradient *res = gradientServer->createResource(QString("bundle://%1:%2").arg(filename()).arg(ref.resourcePath));
    //                if (!res) {
    //                    qWarning() << "Could not create resource for" << ref.resourcePath;
    //                    continue;
    //                }
    //                if (!resourceStore->open(ref.resourcePath)) {
    //                    qWarning() << "Failed to open" << ref.resourcePath << "from bundle" << filename();
    //                    continue;
    //                }
    //                if (!res->loadFromDevice(resourceStore->device())) {
    //                    qWarning() << "Failed to load" << ref.resourcePath << "from bundle" << filename();
    //                    continue;
    //                }
    //                dbgResources << "\t\tresource:" << res->name();

    //                KoAbstractGradient *res2 = gradientServer->resourceByName(res->name());
    //                if (!res2)  {//if it doesn't exist...
    //                    gradientServer->addResource(res, false);//add it!

    //                    if (!m_gradientsMd5Installed.contains(res->md5())) {
    //                        m_gradientsMd5Installed.append(res->md5());
    //                    }
    //                    if (ref.md5sum!=res->md5()) {
    //                        md5Mismatch.append(res->name());
    //                    }

    //                    Q_FOREACH (const QString &tag, ref.tagList) {
    //                        gradientServer->addTag(res, tag);
    //                    }
    //                    //gradientServer->addTag(res, name());
    //                }
    //                else {
    //                    //qWarning() << "Didn't install" << res->name()<<"It already exists on the server";
    //                }

    //            }
    //        }
    //        else if (resType  == "patterns") {
    //            KoResourceServer<KoPattern>* patternServer = KoResourceServerProvider::instance()->patternServer();
    //            Q_FOREACH (const KoResourceBundleManifest::ResourceReference &ref, m_manifest.files(resType)) {

    //                if (resourceStore->isOpen()) resourceStore->close();

    //                dbgResources << "\tInstalling" << ref.resourcePath;
    //                KoPattern *res = patternServer->createResource(QString("bundle://%1:%2").arg(filename()).arg(ref.resourcePath));
    //                if (!res) {
    //                    qWarning() << "Could not create resource for" << ref.resourcePath;
    //                    continue;
    //                }
    //                if (!resourceStore->open(ref.resourcePath)) {
    //                    qWarning() << "Failed to open" << ref.resourcePath << "from bundle" << filename();
    //                    continue;
    //                }
    //                if (!res->loadFromDevice(resourceStore->device())) {
    //                    qWarning() << "Failed to load" << ref.resourcePath << "from bundle" << filename();
    //                    continue;
    //                }
    //                dbgResources << "\t\tresource:" << res->name();

    //                KoPattern *res2 = patternServer->resourceByName(res->name());
    //                if (!res2)  {//if it doesn't exist...
    //                    patternServer->addResource(res, false);//add it!

    //                    if (!m_patternsMd5Installed.contains(res->md5())) {
    //                        m_patternsMd5Installed.append(res->md5());
    //                    }
    //                    if (ref.md5sum!=res->md5()) {
    //                        md5Mismatch.append(res->name());
    //                    }

    //                    Q_FOREACH (const QString &tag, ref.tagList) {
    //                        patternServer->addTag(res, tag);
    //                    }
    //                    //patternServer->addTag(res, name());
    //                }

    //            }
    //        }
    //        else if (resType  == "brushes") {
    //            KisBrushResourceServer *brushServer = KisBrushServer::instance()->brushServer();
    //            Q_FOREACH (const KoResourceBundleManifest::ResourceReference &ref, m_manifest.files(resType)) {

    //                if (resourceStore->isOpen()) resourceStore->close();

    //                dbgResources << "\tInstalling" << ref.resourcePath;
    //                KisBrushSP res = brushServer->createResource(QString("bundle://%1:%2").arg(filename()).arg(ref.resourcePath));
    //                if (!res) {
    //                    qWarning() << "Could not create resource for" << ref.resourcePath;
    //                    continue;
    //                }
    //                if (!resourceStore->open(ref.resourcePath)) {
    //                    qWarning() << "Failed to open" << ref.resourcePath << "from bundle" << filename();
    //                    continue;
    //                }
    //                if (!res->loadFromDevice(resourceStore->device())) {
    //                    qWarning() << "Failed to load" << ref.resourcePath << "from bundle" << filename();
    //                    continue;
    //                }
    //                dbgResources << "\t\tresource:" << res->name();

    //                //find the resource on the server
    //                KisBrushSP res2 = brushServer->resourceByName(res->name());
    //                if (res2) {
    //                    res->setName(res->name()+"("+res->shortFilename()+")");
    //                }
    //                // file name is more important than the regular name because the
    //                // it is the way how it is called up from the brushpreset settings.
    //                // Therefore just adjust the resource name and only refuse to load
    //                // when the filename is different.
    //                res2 = brushServer->resourceByFilename(res->shortFilename());
    //                if (!res2)  {//if it doesn't exist...
    //                    brushServer->addResource(res, false);//add it!

    //                    if (!m_brushesMd5Installed.contains(res->md5())) {
    //                        m_brushesMd5Installed.append(res->md5());
    //                    }
    //                    if (ref.md5sum!=res->md5()) {
    //                        md5Mismatch.append(res->name());
    //                    }

    //                    Q_FOREACH (const QString &tag, ref.tagList) {
    //                        brushServer->addTag(res.data(), tag);
    //                    }
    //                    //brushServer->addTag(res.data(), name());
    //                }
    //                else {
    //                    //qWarning() << "Didn't install" << res->name()<<"It already exists on the server";
    //                }
    //            }
    //        }
    //        else if (resType  == "palettes") {
    //            KoResourceServer<KoColorSet>* paletteServer = KoResourceServerProvider::instance()->paletteServer();
    //            Q_FOREACH (const KoResourceBundleManifest::ResourceReference &ref, m_manifest.files(resType)) {

    //                if (resourceStore->isOpen()) resourceStore->close();

    //                dbgResources << "\tInstalling" << ref.resourcePath;
    //                KoColorSet *res = paletteServer->createResource(QString("bundle://%1:%2").arg(filename()).arg(ref.resourcePath));

    //                if (!res) {
    //                    qWarning() << "Could not create resource for" << ref.resourcePath;
    //                    continue;
    //                }
    //                if (!resourceStore->open(ref.resourcePath)) {
    //                    qWarning() << "Failed to open" << ref.resourcePath << "from bundle" << filename();
    //                    continue;
    //                }
    //                if (!res->loadFromDevice(resourceStore->device())) {
    //                    qWarning() << "Failed to load" << ref.resourcePath << "from bundle" << filename();
    //                    continue;
    //                }
    //                dbgResources << "\t\tresource:" << res->name();

    //                //find the resource on the server
    //                KoColorSet *res2 = paletteServer->resourceByName(res->name());
    //                if (!res2)  {//if it doesn't exist...
    //                    paletteServer->addResource(res, false);//add it!

    //                    if (!m_palettesMd5Installed.contains(res->md5())) {
    //                        m_palettesMd5Installed.append(res->md5());
    //                    }
    //                    if (ref.md5sum!=res->md5()) {
    //                        md5Mismatch.append(res->name());
    //                    }

    //                    Q_FOREACH (const QString &tag, ref.tagList) {
    //                        paletteServer->addTag(res, tag);
    //                    }
    //                    //paletteServer->addTag(res, name());
    //                }
    //                else {
    //                    //qWarning() << "Didn't install" << res->name()<<"It already exists on the server";
    //                }
    //            }
    //        }
    //        else if (resType  == "workspaces") {
    //            KoResourceServer< KisWorkspaceResource >* workspaceServer = KisResourceServerProvider::instance()->workspaceServer();
    //            Q_FOREACH (const KoResourceBundleManifest::ResourceReference &ref, m_manifest.files(resType)) {

    //                if (resourceStore->isOpen()) resourceStore->close();

    //                dbgResources << "\tInstalling" << ref.resourcePath;
    //                KisWorkspaceResource *res = workspaceServer->createResource(QString("bundle://%1:%2").arg(filename()).arg(ref.resourcePath));
    //                if (!res) {
    //                    qWarning() << "Could not create resource for" << ref.resourcePath;
    //                    continue;
    //                }
    //                if (!resourceStore->open(ref.resourcePath)) {
    //                    qWarning() << "Failed to open" << ref.resourcePath << "from bundle" << filename();
    //                    continue;
    //                }
    //                if (!res->loadFromDevice(resourceStore->device())) {
    //                    qWarning() << "Failed to load" << ref.resourcePath << "from bundle" << filename();
    //                    continue;
    //                }
    //                dbgResources << "\t\tresource:" << res->name();

    //                //the following tries to find the resource by name.
    //                KisWorkspaceResource *res2 = workspaceServer->resourceByName(res->name());
    //                if (!res2)  {//if it doesn't exist...
    //                    workspaceServer->addResource(res, false);//add it!

    //                    if (!m_workspacesMd5Installed.contains(res->md5())) {
    //                        m_workspacesMd5Installed.append(res->md5());
    //                    }
    //                    if (ref.md5sum!=res->md5()) {
    //                        md5Mismatch.append(res->name());
    //                    }

    //                    Q_FOREACH (const QString &tag, ref.tagList) {
    //                        workspaceServer->addTag(res, tag);
    //                    }
    //                    //workspaceServer->addTag(res, name());
    //                }
    //                else {
    //                    //qWarning() << "Didn't install" << res->name()<<"It already exists on the server";
    //                }

    //            }
    //        }
    //        else if (resType  == "paintoppresets") {
    //            KisPaintOpPresetResourceServer*  paintoppresetServer = KisResourceServerProvider::instance()->paintOpPresetServer();
    //            Q_FOREACH (const KoResourceBundleManifest::ResourceReference &ref, m_manifest.files(resType)) {

    //                if (resourceStore->isOpen()) resourceStore->close();

    //                dbgResources << "\tInstalling" << ref.resourcePath;
    //                KisPaintOpPresetSP res = paintoppresetServer->createResource(QString("bundle://%1:%2").arg(filename()).arg(ref.resourcePath));

    //                if (!res) {
    //                    qWarning() << "Could not create resource for" << ref.resourcePath;
    //                    continue;
    //                }
    //                if (!resourceStore->open(ref.resourcePath)) {
    //                    qWarning() << "Failed to open" << ref.resourcePath << "from bundle" << filename();
    //                    continue;
    //                }
    //                // Workaround for some OS (Debian, Ubuntu), where loading directly from the QIODevice
    //                // fails with "libpng error: IDAT: CRC error"
    //                QByteArray data = resourceStore->device()->readAll();
    //                QBuffer buffer(&data);
    //                if (!res->loadFromDevice(&buffer)) {
    //                    qWarning() << "Failed to load" << ref.resourcePath << "from bundle" << filename();
    //                    continue;
    //                }
    //                dbgResources << "\t\tresource:" << res->name() << "File:" << res->filename();

    //                //the following tries to find the resource by name.
    //                KisPaintOpPresetSP res2 = paintoppresetServer->resourceByName(res->name());
    //                if (!res2)  {//if it doesn't exist...
    //                    paintoppresetServer->addResource(res, false);//add it!
    //                    if (!m_presetsMd5Installed.contains(res->md5())){
    //                        m_presetsMd5Installed.append(res->md5());
    //                    }
    //                    if (ref.md5sum!=res->md5()) {
    //                        md5Mismatch.append(res->name());
    //                    }

    //                    Q_FOREACH (const QString &tag, ref.tagList) {
    //                        paintoppresetServer->addTag(res.data(), tag);
    //                    }
    //                    //paintoppresetServer->addTag(res.data(), name());
    //                }
    //                else {
    //                    //qWarning() << "Didn't install" << res->name()<<"It already exists on the server";
    //                }

    //            }
    //        }
    //    }
    m_installed = true;
    if(!md5Mismatch.isEmpty()){
        QString message = i18n("The following resources had mismatching MD5 sums. They may have gotten corrupted, for example, during download.");
        QMessageBox bundleFeedback;
        bundleFeedback.setIcon(QMessageBox::Warning);
        Q_FOREACH (QString name, md5Mismatch) {
            message.append("\n");
            message.append(name);
        }
        bundleFeedback.setText(message);
        bundleFeedback.exec();
    }
    return true;
}

bool KoResourceBundle::uninstall()
{

    m_installed = false;
    QStringList tags = getTagsList();
    tags << m_manifest.tags();
    //tags << name();

    //    KoResourceServer<KoAbstractGradient>* gradientServer = KoResourceServerProvider::instance()->gradientServer();
    //    //Q_FOREACH (const KoResourceBundleManifest::ResourceReference &ref, m_manifest.files("gradients")) {
    //    Q_FOREACH (const QByteArray md5, m_gradientsMd5Installed) {
    //        KoAbstractGradient *res = gradientServer->resourceByMD5(md5);
    //        if (res) {
    //            gradientServer->removeResourceFromServer(res);
    //        }
    //    }

    //    KoResourceServer<KoPattern>* patternServer = KoResourceServerProvider::instance()->patternServer();
    //    //Q_FOREACH (const KoResourceBundleManifest::ResourceReference &ref, m_manifest.files("patterns")) {
    //    Q_FOREACH (const QByteArray md5, m_patternsMd5Installed) {
    //        KoPattern *res = patternServer->resourceByMD5(md5);
    //        if (res) {
    //            patternServer->removeResourceFromServer(res);
    //        }
    //    }

    //    KisBrushResourceServer *brushServer = KisBrushServer::instance()->brushServer();
    //    //Q_FOREACH (const KoResourceBundleManifest::ResourceReference &ref, m_manifest.files("brushes")) {
    //    Q_FOREACH (const QByteArray md5, m_brushesMd5Installed) {
    //        KisBrushSP res = brushServer->resourceByMD5(md5);
    //        if (res) {
    //            brushServer->removeResourceFromServer(res);
    //        }
    //    }

    //    KoResourceServer<KoColorSet>* paletteServer = KoResourceServerProvider::instance()->paletteServer();
    //    //Q_FOREACH (const KoResourceBundleManifest::ResourceReference &ref, m_manifest.files("palettes")) {
    //    Q_FOREACH (const QByteArray md5, m_palettesMd5Installed) {
    //        KoColorSet *res = paletteServer->resourceByMD5(md5);
    //        if (res) {
    //            paletteServer->removeResourceFromServer(res);
    //        }
    //    }

    //    KoResourceServer< KisWorkspaceResource >* workspaceServer = KisResourceServerProvider::instance()->workspaceServer();
    //    //Q_FOREACH (const KoResourceBundleManifest::ResourceReference &ref, m_manifest.files("workspaces")) {
    //    Q_FOREACH (const QByteArray md5, m_workspacesMd5Installed) {
    //        KisWorkspaceResource *res = workspaceServer->resourceByMD5(md5);
    //        if (res) {
    //            workspaceServer->removeResourceFromServer(res);
    //        }
    //    }
    //    KisPaintOpPresetResourceServer* paintoppresetServer = KisResourceServerProvider::instance()->paintOpPresetServer();
    //    //Q_FOREACH (const KoResourceBundleManifest::ResourceReference &ref, m_manifest.files("paintoppresets")) {
    //    Q_FOREACH (const QByteArray md5, m_presetsMd5Installed) {
    //        KisPaintOpPresetSP res = paintoppresetServer->resourceByMD5(md5);
    //        if (res) {
    //            paintoppresetServer->removeResourceFromServer(res);
    //        }
    //    }

    //    Q_FOREACH(const QString &tag, tags) {
    //        paintoppresetServer->tagCategoryRemoved(tag);
    //        workspaceServer->tagCategoryRemoved(tag);
    //        paletteServer->tagCategoryRemoved(tag);
    //        brushServer->tagCategoryRemoved(tag);
    //        patternServer->tagCategoryRemoved(tag);
    //        gradientServer->tagCategoryRemoved(tag);
    //    }


    return true;
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

void KoResourceBundle::addResource(QString fileType, QString filePath, QStringList fileTagList, const QByteArray md5sum)
{
    m_manifest.addResource(fileType, filePath, fileTagList, md5sum);
}

QList<QString> KoResourceBundle::getTagsList()
{
    return QList<QString>::fromSet(m_bundletags);
}


bool KoResourceBundle::isInstalled()
{
    return m_installed;
}


QStringList KoResourceBundle::resourceTypes() const
{
    return m_manifest.types();
}

QList<KoResource*> KoResourceBundle::resources(const QString &resType) const
{
    QList<KoResourceBundleManifest::ResourceReference> references = m_manifest.files(resType);

    QList<KoResource*> ret;
    //    Q_FOREACH (const KoResourceBundleManifest::ResourceReference &ref, references) {
    //        if (resType == "gradients") {
    //            KoResourceServer<KoAbstractGradient>* gradientServer = KoResourceServerProvider::instance()->gradientServer();
    //            KoResource *res =  gradientServer->resourceByMD5(ref.md5sum);
    //            if (res) ret << res;
    //        }
    //        else if (resType  == "patterns") {
    //            KoResourceServer<KoPattern>* patternServer = KoResourceServerProvider::instance()->patternServer();
    //            KoResource *res =  patternServer->resourceByMD5(ref.md5sum);
    //            if (res) ret << res;
    //        }
    //        else if (resType  == "brushes") {
    //            KisBrushResourceServer *brushServer = KisBrushServer::instance()->brushServer();
    //            KoResource *res =  brushServer->resourceByMD5(ref.md5sum).data();
    //            if (res) ret << res;
    //        }
    //        else if (resType  == "palettes") {
    //            KoResourceServer<KoColorSet>* paletteServer = KoResourceServerProvider::instance()->paletteServer();
    //            KoResource *res =  paletteServer->resourceByMD5(ref.md5sum);
    //            if (res) ret << res;
    //        }
    //        else if (resType  == "workspaces") {
    //            KoResourceServer< KisWorkspaceResource >* workspaceServer = KisResourceServerProvider::instance()->workspaceServer();
    //            KoResource *res =  workspaceServer->resourceByMD5(ref.md5sum);
    //            if (res) ret << res;
    //        }
    //        else if (resType  == "paintoppresets") {
    //            KisPaintOpPresetResourceServer* paintoppresetServer = KisResourceServerProvider::instance()->paintOpPresetServer();
    //            KisPaintOpPresetSP res =  paintoppresetServer->resourceByMD5(ref.md5sum);
    //            if (res) ret << res.data();
    //        }
    //    }
    return ret;
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

    setImage(m_thumbnail);
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

void KoResourceBundle::setInstalled(bool install)
{
    m_installed = install;
}

void KoResourceBundle::saveMetadata(QScopedPointer<KoStore> &store)
{
    QBuffer buf;

    store->open("meta.xml");
    buf.open(QBuffer::WriteOnly);

    KoXmlWriter metaWriter(&buf);
    metaWriter.startDocument("office:document-meta");
    metaWriter.startElement("meta:meta");

    writeMeta(KisStoragePlugin::s_meta_generator, &metaWriter);

    metaWriter.startElement(KisStoragePlugin::s_meta_version.toUtf8());
    metaWriter.addTextNode(m_bundleVersion.toUtf8());
    metaWriter.endElement();

    writeMeta(KisStoragePlugin::s_meta_author, &metaWriter);
    writeMeta(KisStoragePlugin::s_meta_title,  &metaWriter);
    writeMeta(KisStoragePlugin::s_meta_description, &metaWriter);
    writeMeta(KisStoragePlugin::s_meta_initial_creator,  &metaWriter);
    writeMeta(KisStoragePlugin::s_meta_creator, &metaWriter);
    writeMeta(KisStoragePlugin::s_meta_creation_date, &metaWriter);
    writeMeta(KisStoragePlugin::s_meta_dc_date, &metaWriter);
    writeUserDefinedMeta("email", &metaWriter);
    writeUserDefinedMeta("license", &metaWriter);
    writeUserDefinedMeta("website", &metaWriter);
    Q_FOREACH (const QString &tag, m_bundletags) {
        metaWriter.startElement(KisStoragePlugin::s_meta_user_defined.toUtf8());
        metaWriter.addAttribute(KisStoragePlugin::s_meta_name.toUtf8(), "tag");
        metaWriter.addAttribute(KisStoragePlugin::s_meta_value.toUtf8(), tag);
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
