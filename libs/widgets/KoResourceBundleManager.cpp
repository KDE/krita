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
#include <QtCore/QFile>

KoResourceBundleManager::KoResourceBundleManager(QString kPath,QString pName,KoStore::Mode mode):kritaPath(kPath),packName(pName)
{
    if (packName!="") {
        resourcePack=KoStore::createStore(packName,mode,"",KoStore::Zip);
    }
}

KoResourceBundleManager::KoResourceBundleManager(KoStore* store,QString kPath):kritaPath(kPath)
{
    resourcePack=store;
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
}

bool KoResourceBundleManager::isPathSet()
{
    return kritaPath=="";
}

void KoResourceBundleManager::toRoot()
{
    while(resourcePack->leaveDirectory());
}

bool KoResourceBundleManager::addKFile(QString path)
{
    toRoot();
    int cpt=path.count('/');
    return resourcePack->addLocalFile(path,path.section('/',cpt-2,cpt-2).append
            (path.section('/',cpt)));
}

void KoResourceBundleManager::addKFiles(QString* pathList)
{
    for (int cpt=0;cpt<pathList->length();cpt++) {
        if (!addKFile(pathList[cpt])) {
            delete [] pathList;
            exit(1);
        }
    }
    delete [] pathList;
}

void KoResourceBundleManager::extractKFiles(QString* pathList)
{
    QString currentPath;
    if (isPathSet()) {
        for (int i=0;i<pathList->length();i++) {
            toRoot();
            currentPath=pathList[i];
            if (!resourcePack->extractFile(currentPath,kritaPath.append(currentPath))) {
                delete [] pathList;
                exit(1);
            }
        }
    }
    delete [] pathList;
}

void KoResourceBundleManager::extractPack(QString packName)
{
    this->packName=packName;
    toRoot();
    //TODO extractThumbnail();
    KoXmlResourceBundleManifest* manifest=new KoXmlResourceBundleManifest(getFile("manifest.xml"));
    extractKFiles(manifest->getFileList()); //TODO getFileList() doit gérer le sous-dossier portant le nom du paquet
}

void KoResourceBundleManager::createPack(KoXmlResourceBundleManifest* manifest, KoXmlResourceBundleMeta* meta)
{
    if (meta->getName()!="") {
        packName=meta->getName();
        resourcePack=KoStore::createStore(packName,KoStore::Write,"",KoStore::Zip);

        if (resourcePack!=NULL) {
            addKFiles(manifest->getFileList());
            //TODO addThumbnail();
            resourcePack->finalize();
        }
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


//File Method Shortcuts

bool KoResourceBundleManager::bad() const
{
    return resourcePack->bad();
}

bool KoResourceBundleManager::hasFile(const QString &name) const
{
    return resourcePack->hasFile(name);
}

bool KoResourceBundleManager::open(const QString &name)
{
    return resourcePack->open(name);
}

bool KoResourceBundleManager::isOpen() const
{
    return resourcePack->isOpen();
}

bool KoResourceBundleManager::close()
{
    return resourcePack->close();
}

QByteArray KoResourceBundleManager::read(qint64 max)
{
    return resourcePack->read(max);
}

qint64 KoResourceBundleManager::read(char *_buffer, qint64 _len)
{
    return resourcePack->read(_buffer,_len);
}

qint64 KoResourceBundleManager::write(const QByteArray &_data)
{
    return resourcePack->write(_data);
}

qint64 KoResourceBundleManager::write(const char *_data, qint64 _len)
{
    return resourcePack->write(_data,_len);
}

qint64 KoResourceBundleManager::size() const
{
    return resourcePack->size();
}

bool KoResourceBundleManager::atEnd() const
{
    return resourcePack->atEnd();
}

bool KoResourceBundleManager::enterDirectory(const QString &directory)
{
    return resourcePack->enterDirectory(directory);
}

bool KoResourceBundleManager::leaveDirectory()
{
    return resourcePack->leaveDirectory();
}

QIODevice* KoResourceBundleManager::device()
{
    return resourcePack->device();
}

