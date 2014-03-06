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

#include <QtCore/QFile>
#include "KoResourceBundleManager.h"
#include "KoXmlResourceBundleManifest.h"
#include "KoXmlResourceBundleMeta.h"
#include "KoResourceServer.h"

#include <kglobal.h>
#include <kstandarddirs.h>
#include <kcomponentdata.h>

#include <sys/stat.h>
#include <iostream>
using namespace std;

KoResourceBundleManager::KoResourceBundleManager(QString kPath,QString pName,KoStore::Mode mode):kritaPath(kPath),packName(pName)
{
    if (packName!="") {
        resourcePack=KoStore::createStore(packName,mode,"",KoStore::Zip);

    }

    if (kritaPath!="" && kritaPath.at(kritaPath.size()-1)!='/') {
        this->kritaPath.append("/");
    }
}

void KoResourceBundleManager::setReadPack(QString packName)
{
    if (packName!="") {
        resourcePack=KoStore::createStore(packName,KoStore::Read,"",KoStore::Zip);
        this->packName=packName;
    }
}

void KoResourceBundleManager::setWritePack(QString packName)
{
    if (packName!="") {
        resourcePack=KoStore::createStore(packName,KoStore::Write,"",KoStore::Zip);
        this->packName=packName;
    }
}

void KoResourceBundleManager::setKritaPath(QString kritaPath)
{
    this->kritaPath=kritaPath;

    if (kritaPath!="" && kritaPath.at(kritaPath.size()-1)!='/') {
        this->kritaPath.append("/");
    }
}

bool KoResourceBundleManager::isPathSet()
{
    return kritaPath!="";
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
    for (int i=0;i<pathList.size();i++) {
        if (!addKFile(pathList.at(i))) {
            exit(1);
        }
    }
}

void KoResourceBundleManager::extractKFiles(QList<QString> pathList)
{
    QString currentPath;
    QString targetPath;
    QString dirPath;
    int pathSize;
    QString name = packName.section('/',packName.count('/')).remove(".zip");

    if (isPathSet()) {
        for (int i=0;i<pathList.size();i++) {
            toRoot();
            currentPath=pathList.at(i);
            pathSize=currentPath.count('/');
            targetPath = kritaPath;
            targetPath.append(currentPath.section('/',0,0));
            if (!resourcePack->extractFile(currentPath,targetPath.append("/").append(name)
                        .append("/").append(currentPath.section('/',pathSize)))) {
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

void KoResourceBundleManager::createPack(KoXmlResourceBundleManifest* manifest, KoXmlResourceBundleMeta* meta)
{
    if (meta->getPackName()!="") {
        packName=meta->getPackName();
        resourcePack=KoStore::createStore(packName,KoStore::Write,"",KoStore::Zip);

        if (resourcePack!=NULL) {
            addKFiles(manifest->getFileList());
            addManiMeta(manifest,meta);
            resourcePack->finalize();
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
