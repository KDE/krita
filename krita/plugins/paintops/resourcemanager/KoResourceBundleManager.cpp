/* This file is part of the KDE project
   Copyright (C) 2014, Victor Lafon <metabolic.ewilan@hotmail.fr>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "KoResourceBundleManager.h"
#include "KoXmlResourceBundleManifest.h"
#include "KoXmlResourceBundleMeta.h"
#include <QImage>
#include <QBuffer>
#include <QDir>

#include <sys/stat.h>

#include <iostream>
using namespace std;


KoResourceBundleManager::KoResourceBundleManager(QString kPath,QString pName,KoStore::Mode mode):kritaPath(kPath),packName(pName)
{
    if (!packName.isEmpty()) {
        resourcePack=KoStore::createStore(packName,mode,"",KoStore::Zip);
    }
    else {
        resourcePack=0;
    }

    if (!kritaPath.isEmpty() && kritaPath.at(kritaPath.size()-1)!='/') {
        this->kritaPath.append("/");
    }
}

void KoResourceBundleManager::setReadPack(QString packName)
{
    if (!packName.isEmpty()) {
        resourcePack=KoStore::createStore(packName,KoStore::Read,"",KoStore::Zip);
        this->packName=packName;
    }
}

void KoResourceBundleManager::setWritePack(QString packName)
{
    if (!packName.isEmpty()) {
        resourcePack=KoStore::createStore(packName,KoStore::Write,"",KoStore::Zip);
        this->packName=packName;
    }
}

void KoResourceBundleManager::setKritaPath(QString kritaPath)
{
    this->kritaPath=kritaPath;

    if (!kritaPath.isEmpty() && kritaPath.at(kritaPath.size()-1)!='/') {
        this->kritaPath.append("/");
    }
}

bool KoResourceBundleManager::isPathSet()
{
    return !kritaPath.isEmpty();
}

void KoResourceBundleManager::toRoot()
{
    while(resourcePack->leaveDirectory());
}

bool KoResourceBundleManager::addKFile(QString path)
{
    toRoot();
    int pathSize=path.count('/');
    return resourcePack->addLocalFile(path,path.section('/',pathSize-1));
}

//TODO Réfléchir à fusionner addKFile et addKFileBundle
//TODO Trouver un moyen de détecter si bundle ou pas
bool KoResourceBundleManager::addKFileBundle(QString path)
{
    toRoot();
    int pathSize=path.count('/');
    return resourcePack->addLocalFile(path,path.section('/',pathSize-2,pathSize-2)
                        .append("/").append(path.section('/',pathSize)));
}

void KoResourceBundleManager::addKFiles(QList<QString> pathList)
{
    QString bundleName=packName.section('/',packName.count('/')).section('.',0,0);
    for (int i=0;i<pathList.size();i++) {
        QString currentFile=pathList.at(i);
        if(currentFile.contains("/"+bundleName+"/")) {
            if (!addKFileBundle(pathList.at(i))) {
                exit(2);
            }
        }
        else {
            if (!addKFile(pathList.at(i))) {
                exit(3);
            }
        }
    }
}

void KoResourceBundleManager::extractKFiles(QMap<QString,QString> pathList)
{
    QString currentPath;
    QString targetPath;
    QString dirPath;

    if (isPathSet()) {
        for (int i=0;i<pathList.size();i++) {
            toRoot();
            currentPath=pathList.keys().at(i);
            targetPath=pathList.values().at(i);
            if (!resourcePack->extractFile(currentPath,targetPath)) {
                dirPath = targetPath.section('/',0,targetPath.count('/')-1);
                mkdir(dirPath.toUtf8().constData(),S_IRWXU|S_IRGRP|S_IXGRP);
                if(!resourcePack->extractFile(currentPath,targetPath)){
                    //TODO Supprimer le dossier créé
                    exit(1);
                }
            }
        }
    }
}

void KoResourceBundleManager::extractTempFiles(QList<QString> pathList)
{
    QString currentPath;
    QString targetPath;

    for (int i=0;i<pathList.size();i++) {
        toRoot();
        targetPath=pathList.at(i);
        if (targetPath.contains("temp")) {
            currentPath=targetPath.section('/',targetPath.count('/')-1);
            if (!resourcePack->extractFile(currentPath,targetPath)) {
                QString dirPath = targetPath.section('/',0,targetPath.count('/')-1);
                mkdir(dirPath.toUtf8().constData(),S_IRWXU|S_IRGRP|S_IXGRP);
                if(!resourcePack->extractFile(currentPath,targetPath)){
                    exit(5);
                }
            }
        }
    }
}

void KoResourceBundleManager::createPack(KoXmlResourceBundleManifest* manifest, KoXmlResourceBundleMeta* meta, QImage thumbnail, bool firstBuild)
{
    packName=meta->getPackFileName();
    if (!packName.isEmpty()) {
        QList<QString> fileList = manifest->getFileList(kritaPath,firstBuild);

        if (!firstBuild && !manifest->isInstalled()) {
            resourcePack=KoStore::createStore(packName,KoStore::Read,"",KoStore::Zip);
            if (resourcePack==NULL || resourcePack->bad()) {
                exit(4);
            }
            else {
                extractTempFiles(fileList);
            }
        }

        resourcePack=KoStore::createStore(packName,KoStore::Write,"",KoStore::Zip);
        if (resourcePack!=NULL && !resourcePack->bad()) {
            addKFiles(fileList);
            manifest->updateFilePaths(kritaPath,packName);
            addThumbnail(thumbnail);
            addManiMeta(manifest,meta);
            resourcePack->finalize();
        }

        if (!firstBuild && !manifest->isInstalled()) {
            QList<QString> dirList=manifest->getDirList();
            for (int i=0;i<dirList.size();i++) {
                removeDir(kritaPath+"temp/"+dirList.at(i));
            }
        }
    }
}

void KoResourceBundleManager::addManiMeta(KoXmlResourceBundleManifest* manifest, KoXmlResourceBundleMeta* meta)
{
    toRoot();
    open("manifest.xml");
    write(manifest->toByteArray());
    close();
    open("meta.xml");
    write(meta->toByteArray());
    close();
}

//TODO Voir pour importer d'autres types d'images
void KoResourceBundleManager::addThumbnail(QImage thumbnail)
{
    if(!thumbnail.isNull()) {
        toRoot();
        QByteArray byteArray;
        QBuffer buffer(&byteArray);
        thumbnail.save(&buffer,"JPG");
        open("thumbnail.jpg");
        write(byteArray);
        close();
    }
}


QByteArray KoResourceBundleManager::getFileData(const QString &fileName)
{
    QByteArray result;

    if (hasFile(fileName)) {
        if (isOpen()) {
            close();
        }
        open(fileName);
        while (!atEnd()) {
            result+=read(size());
        }
        close();
    }

    return result;
}

QIODevice* KoResourceBundleManager::getFile(const QString &fileName)
{
    if (hasFile(fileName)) {
        if (isOpen()) {
            close();
        }
        open(fileName);
        return resourcePack->device();
    }

    return 0;
}

bool KoResourceBundleManager::removeDir(const QString & dirName)
{
    bool result = true;
    QDir dir(dirName);

    if (dir.exists(dirName)) {
        Q_FOREACH(QFileInfo info, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System
                    | QDir::Hidden  | QDir::AllDirs | QDir::Files, QDir::DirsFirst)) {
            if (info.isDir()) {
                result = removeDir(info.absoluteFilePath());
            }
            else {
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


QString KoResourceBundleManager::getKritaPath()
{
    return kritaPath;
}

QString KoResourceBundleManager::getPackName()
{
    return packName;
}

//File Method Shortcuts

bool KoResourceBundleManager::atEnd() const
{
    return resourcePack->atEnd();
}

bool KoResourceBundleManager::bad() const
{
    return resourcePack->bad();
}

bool KoResourceBundleManager::close()
{
    return resourcePack->close();
}

bool KoResourceBundleManager::finalize(){
    return resourcePack->finalize();
}

bool KoResourceBundleManager::hasFile(const QString &name) const
{
    return resourcePack->hasFile(name);
}

bool KoResourceBundleManager::isOpen() const
{
    return resourcePack->isOpen();
}

bool KoResourceBundleManager::open(const QString &name)
{
    return resourcePack->open(name);
}

QByteArray KoResourceBundleManager::read(qint64 max)
{
    return resourcePack->read(max);
}

qint64 KoResourceBundleManager::size() const
{
    return resourcePack->size();
}

qint64 KoResourceBundleManager::write(const QByteArray &_data)
{
    return resourcePack->write(_data);
}
