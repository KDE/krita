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
#include "KoXmlResourceBundleManifest.h"
#include "KoXmlResourceBundleMeta.h"

#include <KoStore.h>

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

#include <resourcemanager.h>

KoResourceBundle::KoResourceBundle(QString const& fileName)
    : KoResource(fileName)\
    , m_manifest(new KoXmlResourceBundleManifest())
    , m_meta(new KoXmlResourceBundleMeta())
{
    setName(QFileInfo(fileName).baseName());
}

KoResourceBundle::~KoResourceBundle()
{
    delete m_meta;
    delete m_manifest;
}

QString KoResourceBundle::defaultFileExtension() const
{
    return QString(".bundle");
}

bool KoResourceBundle::load()
{
    if (filename().isEmpty()) return false;
    QScopedPointer<KoStore> resourceStore(KoStore::createStore(filename(), KoStore::Read, "application/x-krita-resourcebundle", KoStore::Zip));

    if (resourceStore->bad()) {
        m_installed = false;
        setValid(false);
        return false;

    } else {
        //TODO Vérifier si on peut éviter de recréer manifest et meta à chaque load
        //A optimiser si possible
        delete m_manifest;
        delete m_meta;

        if (resourceStore->open("manifest.xml")) {
            m_manifest = new KoXmlResourceBundleManifest(resourceStore->device());
            resourceStore->close();
        } else {
            return false;
        }

        if (resourceStore->open("meta.xml")) {
            m_meta = new KoXmlResourceBundleMeta(resourceStore->device());
            resourceStore->close();
        } else {
            return false;
        }

        if (resourceStore->open("thumbnail.png")) {
            m_thumbnail.load(resourceStore->device(), "PNG");
            resourceStore->close();
        }

        m_installed = m_manifest->isInstalled();
        setValid(true);
        setImage(m_thumbnail);
    }

    return true;
}

bool addKFile(KoStore* store, QString path)
{
    while (store->leaveDirectory());
    int pathSize = path.count('/');
    return store->addLocalFile(path, path.section('/', pathSize - 1));
}

bool addKFileBundle(KoStore* store, QString path)
{
    while (store->leaveDirectory());
    int pathSize = path.count('/');
    return store->addLocalFile(path, path.section('/', pathSize - 2, pathSize - 2).append("/").append(path.section('/', pathSize)));
}


bool KoResourceBundle::save()
{
    if (filename().isEmpty()) return false;

    addMeta("updated", QDate::currentDate().toString("dd/MM/yyyy"));
    m_manifest->checkSort();
    m_meta->checkSort();

    bool bundleExists = QFileInfo(filename()).exists();
    QDir bundleDir = KGlobal::dirs()->saveLocation("appdata", "bundles");
    bundleDir.cdUp();

    QList<QString> fileList = m_manifest->getFileList(bundleDir.absolutePath(), !bundleExists); // -- firstBuild

    if (bundleExists && !m_manifest->isInstalled()) {
        QScopedPointer<KoStore> resourceStore(KoStore::createStore(filename(), KoStore::Read, "application/x-krita-resourcebundle", KoStore::Zip));
        if (!resourceStore || resourceStore->bad()) {
            return false;
        } else {
            // Copy the contents of an uninstalled, existing bundle to a temporary directory
            QString currentPath;

            foreach (QString targetPath, fileList) {
                while (resourceStore->leaveDirectory());
                if (targetPath.contains("temp")) {
                    currentPath = targetPath.section('/', targetPath.count('/') - 1);
                    if (!resourceStore->extractFile(currentPath, targetPath)) {
                        QString dirPath = targetPath.section('/', 0, targetPath.count('/') - 1);
                        QDir dir(dirPath);
                        dir.mkdir(dirPath);
                        if (!resourceStore->extractFile(currentPath, targetPath)) {
                            qWarning() << "Failed to extract" << currentPath << "to" << targetPath;
                            continue;
                        }
                    }
                }
            }

        }
    }

    QScopedPointer<KoStore> resourceStore(KoStore::createStore(filename(), KoStore::Write, "application/x-krita-resourcebundle", KoStore::Zip));
    if (!resourceStore || resourceStore->bad()) return false;

    QString bundleName = filename().section('/', filename().count('/')).section('.', 0, 0);
    for (int i = 0; i < fileList.size(); i++) {
        QString currentFile = fileList.at(i);
        if (currentFile.contains("/" + bundleName + "/")) {
            if (!addKFileBundle(resourceStore.data(), fileList.at(i))) {
                continue;
            }
        } else {
            if (!addKFile(resourceStore.data(), fileList.at(i))) {
                continue;
            }
        }
    }


    m_manifest->updateFilePaths(bundleDir.absolutePath(), filename());

    if (!m_thumbnail.isNull()) {
        while (resourceStore->leaveDirectory());
        QByteArray byteArray;
        QBuffer buffer(&byteArray);
        m_thumbnail.save(&buffer, "PNG");
        resourceStore->open("thumbnail.png");
        resourceStore->write(byteArray);
        resourceStore->close();
    }

    resourceStore->open("manifest.xml");
    resourceStore->write(m_manifest->toByteArray());
    resourceStore->close();
    resourceStore->open("meta.xml");
    resourceStore->write(m_meta->toByteArray());
    resourceStore->close();
    resourceStore->finalize();


    if (bundleExists && !m_manifest->isInstalled()) {
        QList<QString> dirList = m_manifest->getDirList();
        for (int i = 0; i < dirList.size(); i++) {
            removeDir(bundleDir.absolutePath() + "/temp/" + dirList.at(i));
        }
    }

    setValid(true);
    return true;
}

//TODO getFilesToExtract à vérifier
//TODO exportTags à vérifier
void KoResourceBundle::install()
{
    if (filename().isEmpty()) return;
    QScopedPointer<KoStore> resourceStore(KoStore::createStore(filename(), KoStore::Read, "application/x-krita-resourcebundle", KoStore::Zip));
    if (!resourceStore || resourceStore->bad()) return;

    QMap<QString, QString> pathList = m_manifest->getFilesToExtract();

    QDir bundleDir = KGlobal::dirs()->saveLocation("appdata", "bundles");
    bundleDir.cdUp();

    QString currentPath;
    QString targetPath;
    QString dirPath;

    for (int i = 0; i < pathList.size(); i++) {
        while (resourceStore->leaveDirectory());
        currentPath = pathList.keys().at(i);
        targetPath = pathList.values().at(i);
        if (!resourceStore->extractFile(currentPath, targetPath)) {
            dirPath = targetPath.section('/', 0, targetPath.count('/') - 1);
            QDir dir(dirPath);
            dir.mkdir(dirPath);
            if (!resourceStore->extractFile(currentPath, targetPath)) {
                qWarning() << "Could not install" << currentPath << "to" << targetPath;
                //TODO Supprimer le dossier créé
                continue;
            }
        }
    }

    m_manifest->exportTags();
    m_installed = true;
    m_manifest->install();
    save();
}

void KoResourceBundle::uninstall()
{
    if (!m_installed)
        return;

    QDir bundleDir = KGlobal::dirs()->saveLocation("appdata", "bundles");
    bundleDir.cdUp();
    QString dirPath = bundleDir.absolutePath();
    QList<QString> directoryList = m_manifest->getDirList();
    QString shortPackName = m_meta->getPackName();

    for (int i = 0; i < directoryList.size(); i++) {
        if (!removeDir(dirPath + directoryList.at(i) + QString("/") + shortPackName)) {
            qWarning() << "Error : Couldn't delete folder : " << dirPath;
        }
    }

    m_installed = false;
    m_manifest->uninstall();
    save();
}

void KoResourceBundle::addMeta(QString type, QString value)
{
    if (type == "created") {
        setValid(true);
    }
    m_meta->addTag(type, value);
}

void KoResourceBundle::setMeta(KoXmlResourceBundleMeta* newMeta)
{
    m_meta = newMeta;
}

//TODO Voir s'il faut aussi rajouter les tags dans le meta
void KoResourceBundle::addFile(QString fileType, QString filePath, QStringList fileTagList)
{
    m_manifest->addManiTag(fileType, filePath, fileTagList);
    m_meta->addTags(fileTagList);
}

//On rappelle que les tags d'un bundle ne sont stockés que dans le meta
//Les tags du manifest sont ajoutés au fur et à mesure de l'ajout des fichiers
QList<QString> KoResourceBundle::getTagsList()
{
    return m_meta->getTagsList();
}


void KoResourceBundle::removeFile(QString fileName)
{
    QList<QString> list = m_manifest->removeFile(fileName);

    for (int i = 0; i < list.size(); i++) {
        m_meta->removeFirstTag("tag", list.at(i));
    }
}

void KoResourceBundle::addResourceDirs()
{
    QDir bundleDir = KGlobal::dirs()->saveLocation("appdata", "bundles");
    bundleDir.cdUp();
    QString localSavePath = bundleDir.absolutePath();
    foreach(const QString& resourceType,  m_manifest->getDirList())  {
        KGlobal::mainComponent().dirs()->addResourceDir(resourceType.toLatin1().data(), localSavePath + "/" + resourceType + "/" + this->name());
    }
}

bool KoResourceBundle::isInstalled()
{
    return m_installed;
}

void KoResourceBundle::rename(QString filename, QString name)
{
    QString oldName = m_meta->getPackName();
    QString shortName = name.section('.', 0, 0);

    addMeta("filename", filename);
    addMeta("name", shortName);
    m_manifest->rename(shortName);

    QDir bundleDir = KGlobal::dirs()->saveLocation("appdata", "bundles");
    bundleDir.cdUp();
    QString localSavePath = bundleDir.absolutePath();

    if (isInstalled()) {
        QList<QString> directoryList = m_manifest->getDirList();
        QString dirPath;
        QDir dir;
        for (int i = 0; i < directoryList.size(); i++) {
            dirPath = localSavePath;
            dirPath.append(directoryList.at(i)).append("/");
            dir.rename(dirPath + oldName, dirPath + shortName);
        }
    }
    save();
}

void KoResourceBundle::removeTag(QString tagName)
{
    m_meta->removeFirstTag("tag", tagName);
}

void KoResourceBundle::setThumbnail(QString filename)
{
    m_thumbnail = QImage(filename);
    save();
}

QByteArray KoResourceBundle::generateMD5() const
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

QString KoResourceBundle::getAuthor()
{
    return m_meta->getValue("author");
}

QString KoResourceBundle::getLicense()
{
    return m_meta->getValue("license");
}

QString KoResourceBundle::getWebSite()
{
    return m_meta->getValue("website");
}

QString KoResourceBundle::getCreated()
{
    return m_meta->getValue("created");
}

QString KoResourceBundle::getUpdated()
{
    return m_meta->getValue("updated");
}


bool KoResourceBundle::removeDir(const QString & dirName)
{
    bool result = true;
    QDir dir(dirName);

    if (dir.exists(dirName)) {
        Q_FOREACH(QFileInfo info, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System
                                                    | QDir::Hidden  | QDir::AllDirs | QDir::Files, QDir::DirsFirst)) {
            if (info.isDir()) {
                result = removeDir(info.absoluteFilePath());
            } else {
                result = QFile::remove(info.absoluteFilePath());
            }

            if (!result) {
                return result;
            }
        }
        result = dir.rmdir(dirName);
    }
    return result;
}
