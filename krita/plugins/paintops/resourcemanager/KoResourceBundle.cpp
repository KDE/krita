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

#include <QProcessEnvironment>
#include <QDate>
#include <QDir>
#include <QDebug>
#include <QBuffer>
#include <QCryptographicHash>
#include <QByteArray>

//TODO Voir s'il ne vaut pas mieux faire un constructeur avec un xmlmeta plutot qu'un setmeta (cf control createPack)
KoResourceBundle::KoResourceBundle(QString const& fileName)
    : KoResource(fileName)
{
    m_kritaPath = fileName.section('/', 0, fileName.count('/') - 2);
    if (!m_kritaPath.isEmpty() && m_kritaPath.at(m_kritaPath.size() - 1) != '/') {
        this->m_kritaPath.append("/");
    }

    m_packName = QString();
    m_resourceStore = 0;

    setName(fileName.section('/', fileName.count('/')));
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

QImage KoResourceBundle::image() const
{
    return m_thumbnail;
}

bool KoResourceBundle::load()
{
    if (filename().isEmpty()) return false;

    m_resourceStore = KoStore::createStore(filename(), KoStore::Read, "", KoStore::Zip);
    m_packName = filename();

    if (m_resourceStore->bad()) {
        m_manifest = new KoXmlResourceBundleManifest();
        m_meta = new KoXmlResourceBundleMeta();
        m_installed = false;
    } else {
        //TODO Vérifier si on peut éviter de recréer manifest et meta à chaque load
        //A optimiser si possible
        m_manifest = new KoXmlResourceBundleManifest(getFile("manifest.xml"));
        m_meta = new KoXmlResourceBundleMeta(getFile("meta.xml"));
        m_thumbnail.load(getFile("thumbnail.jpg"), "JPG");
        m_resourceStore->close();
        m_installed = m_manifest->isInstalled();
        setValid(true);
    }
    return true;
}

bool KoResourceBundle::save()
{
    addMeta("updated", QDate::currentDate().toString("dd/MM/yyyy"));
    m_manifest->checkSort();
    m_meta->checkSort();

    if (m_resourceStore->bad()) {
        //meta->addTags(manifest->getTagsList());
        createPack(m_manifest, m_meta, m_thumbnail, true);
    } else {
        createPack(m_manifest, m_meta, m_thumbnail);
    }

    if (!valid()) {
        setValid(true);
    }

    return load();
}

//TODO getFilesToExtract à vérifier
//TODO exportTags à vérifier
void KoResourceBundle::install()
{
    //load();
    if (!m_resourceStore->bad()) {
        extractKFiles(m_manifest->getFilesToExtract());
        m_manifest->exportTags();
        m_installed = true;
        m_manifest->install();
        save();
    }
}

void KoResourceBundle::uninstall()
{
    if (!m_installed)
        return;

    QString dirPath = m_kritaPath;
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
    QList<QString> listeType = m_manifest->getDirList();
    for (int i = 0; i < listeType.size(); i++) {
        KGlobal::mainComponent().dirs()->addResourceDir(listeType.at(i).toLatin1().data(), this->m_kritaPath + listeType.at(i) + "/" + this->name());
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

    if (isInstalled()) {
        QList<QString> directoryList = m_manifest->getDirList();
        QString dirPath;
        QDir dir;
        for (int i = 0; i < directoryList.size(); i++) {
            dirPath = m_kritaPath;
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

void KoResourceBundle::setKritaPath(QString kritaPath)
{
    this->m_kritaPath = kritaPath;

    if (!kritaPath.isEmpty() && kritaPath.at(kritaPath.size() - 1) != '/') {
        this->m_kritaPath.append("/");
    }
}

bool KoResourceBundle::isPathSet()
{
    return !m_kritaPath.isEmpty();
}

void KoResourceBundle::toRoot()
{
    while (m_resourceStore->leaveDirectory());
}

bool KoResourceBundle::addKFile(QString path)
{
    toRoot();
    int pathSize = path.count('/');
    return m_resourceStore->addLocalFile(path, path.section('/', pathSize - 1));
}

//TODO Réfléchir à fusionner addKFile et addKFileBundle
//TODO Trouver un moyen de détecter si bundle ou pas
bool KoResourceBundle::addKFileBundle(QString path)
{
    toRoot();
    int pathSize = path.count('/');
    return m_resourceStore->addLocalFile(path, path.section('/', pathSize - 2, pathSize - 2)
                                         .append("/").append(path.section('/', pathSize)));
}

void KoResourceBundle::addKFiles(QList<QString> pathList)
{
    QString bundleName = m_packName.section('/', m_packName.count('/')).section('.', 0, 0);
    for (int i = 0; i < pathList.size(); i++) {
        QString currentFile = pathList.at(i);
        if (currentFile.contains("/" + bundleName + "/")) {
            if (!addKFileBundle(pathList.at(i))) {
                continue;
            }
        } else {
            if (!addKFile(pathList.at(i))) {
                continue;
            }
        }
    }
}

void KoResourceBundle::extractKFiles(QMap<QString, QString> pathList)
{
    QString currentPath;
    QString targetPath;
    QString dirPath;

    if (isPathSet()) {
        for (int i = 0; i < pathList.size(); i++) {
            toRoot();
            currentPath = pathList.keys().at(i);
            targetPath = pathList.values().at(i);
            if (!m_resourceStore->extractFile(currentPath, targetPath)) {
                dirPath = targetPath.section('/', 0, targetPath.count('/') - 1);
                QDir dir(dirPath);
                dir.mkdir(dirPath);
                if (!m_resourceStore->extractFile(currentPath, targetPath)) {
                    qDebug() << currentPath << targetPath;
                    //TODO Supprimer le dossier créé
                    continue;
                }
            }
        }
    }
}

void KoResourceBundle::extractTempFiles(QList<QString> pathList)
{
    QString currentPath;
    QString targetPath;

    for (int i = 0; i < pathList.size(); i++) {
        toRoot();
        targetPath = pathList.at(i);
        if (targetPath.contains("temp")) {
            currentPath = targetPath.section('/', targetPath.count('/') - 1);
            if (!m_resourceStore->extractFile(currentPath, targetPath)) {
                QString dirPath = targetPath.section('/', 0, targetPath.count('/') - 1);
                QDir dir(dirPath);
                dir.mkdir(dirPath);

                if (!m_resourceStore->extractFile(currentPath, targetPath)) {
                    continue;
                }
            }
        }
    }
}

void KoResourceBundle::createPack(KoXmlResourceBundleManifest* manifest, KoXmlResourceBundleMeta* meta, QImage thumbnail, bool firstBuild)
{
    m_packName = meta->getPackFileName();
    if (!m_packName.isEmpty()) {
        QList<QString> fileList = manifest->getFileList(m_kritaPath, firstBuild);

        if (!firstBuild && !manifest->isInstalled()) {
            m_resourceStore = KoStore::createStore(m_packName, KoStore::Read, "", KoStore::Zip);
            if (m_resourceStore == NULL || m_resourceStore->bad()) {
                return;
            } else {
                extractTempFiles(fileList);
            }
        }

        m_resourceStore = KoStore::createStore(m_packName, KoStore::Write, "", KoStore::Zip);
        if (m_resourceStore != NULL && !m_resourceStore->bad()) {
            addKFiles(fileList);
            manifest->updateFilePaths(m_kritaPath, m_packName);
            addThumbnail(thumbnail);
            addManiMeta(manifest, meta);
            m_resourceStore->finalize();
        }

        if (!firstBuild && !manifest->isInstalled()) {
            QList<QString> dirList = manifest->getDirList();
            for (int i = 0; i < dirList.size(); i++) {
                removeDir(m_kritaPath + "temp/" + dirList.at(i));
            }
        }
    }
}

void KoResourceBundle::addManiMeta(KoXmlResourceBundleManifest* manifest, KoXmlResourceBundleMeta* meta)
{
    toRoot();
    m_resourceStore->open("manifest.xml");
    m_resourceStore->write(manifest->toByteArray());
    m_resourceStore->close();
    m_resourceStore->open("meta.xml");
    m_resourceStore->write(meta->toByteArray());
    m_resourceStore->close();
}

//TODO Voir pour importer d'autres types d'images
void KoResourceBundle::addThumbnail(QImage thumbnail)
{
    if (!thumbnail.isNull()) {
        toRoot();
        QByteArray byteArray;
        QBuffer buffer(&byteArray);
        thumbnail.save(&buffer, "JPG");
        m_resourceStore->open("thumbnail.jpg");
        m_resourceStore->write(byteArray);
        m_resourceStore->close();
    }
}


QByteArray KoResourceBundle::getFileData(const QString &fileName)
{
    QByteArray result;

    if (m_resourceStore->hasFile(fileName)) {
        if (m_resourceStore->isOpen()) {
            m_resourceStore->close();
        }
        m_resourceStore->open(fileName);
        while (!m_resourceStore->atEnd()) {
            result += m_resourceStore->read(m_resourceStore->size());
        }
        m_resourceStore->close();
    }

    return result;
}

QIODevice* KoResourceBundle::getFile(const QString &fileName)
{
    if (m_resourceStore->hasFile(fileName)) {
        if (m_resourceStore->isOpen()) {
            m_resourceStore->close();
        }
        m_resourceStore->open(fileName);
        return m_resourceStore->device();
    }

    return 0;
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
