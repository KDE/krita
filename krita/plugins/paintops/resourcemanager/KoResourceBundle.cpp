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
#include "KoResourceBundleManager.h"
#include "KoXmlResourceBundleManifest.h"
#include "KoXmlResourceBundleMeta.h"

#include <kglobal.h>
#include <kcomponentdata.h>
#include <kstandarddirs.h>

#include <QtCore/QProcessEnvironment>
#include <QtCore/QDate>
#include <QtCore/QDir>

#include <iostream>
using namespace std;

KoResourceBundle::KoResourceBundle(QString const& bundlePath):KoResource(bundlePath)
{
    manager=new KoResourceBundleManager(bundlePath.section('/',0,bundlePath.count('/')-2));
    setName(bundlePath.section('/',bundlePath.count('/')));
}

KoResourceBundle::~KoResourceBundle()
{
    delete manager;
    delete meta;
    delete manifest;
}

QString KoResourceBundle::defaultFileExtension() const
{
    return QString(".zip");
}

QImage KoResourceBundle::image() const
{
    return thumbnail;
}

bool KoResourceBundle::load()
{
    manager->setReadPack(filename());
    if (manager->bad()) {
        manifest=new KoXmlResourceBundleManifest();
        meta=new KoXmlResourceBundleMeta();
        installed=false;
    }
    else {
        //TODO Vérifier si on peut éviter de recréer manifest et meta à chaque load
        //A optimiser si possible
        manifest=new KoXmlResourceBundleManifest(manager->getFile("manifest.xml"));
        meta=new KoXmlResourceBundleMeta(manager->getFile("meta.xml"));
        thumbnail.load(manager->getFile("thumbnail.jpg"),"JPG");
        manager->close();
        installed=manifest->isInstalled();
        setValid(true);
    }
    return true;
}

bool KoResourceBundle::save()
{
    addMeta("updated",QDate::currentDate().toString("dd/MM/yyyy"));
    manifest->checkSort();
    meta->checkSort();

    if (manager->bad()) {
        meta->addTags(manifest->getTagList());
        manager->createPack(manifest,meta,thumbnail,true);
    }
    else {
        manager->createPack(manifest,meta,thumbnail);
    }

    if (!valid()) {
        cout<<"Valid"<<endl;
        setValid(true);
    }

    return load();
}

//TODO getFilesToExtract à vérifier
//TODO exportTags à vérifier
void KoResourceBundle::install()
{
    //load();
    if (!manager->bad()) {
        manager->extractKFiles(manifest->getFilesToExtract());
        manifest->exportTags();
        installed=true;
        manifest->install();
        save();
    }
}

void KoResourceBundle::uninstall()
{
    if (!installed)
        return;

    QString dirPath = this->manager->getKritaPath();
    QList<QString> directoryList = manifest->getDirList();
    QString shortPackName = meta->getPackName();

    for (int i = 0; i < directoryList.size(); i++) {
        if (!KoResourceBundleManager::removeDir(dirPath + directoryList.at(i) + QString("/") + shortPackName)) {
            cerr<<"Error : Couldn't delete folder : "<<qPrintable(dirPath)<<endl;
        }
    }

    installed=false;
    manifest->uninstall();
    save();
}

void KoResourceBundle::addMeta(QString type,QString value)
{
    if (type=="created") {
        setValid(true);
    }
    meta->addTag(type,value);
}

void KoResourceBundle::setMeta(KoXmlResourceBundleMeta* newMeta)
{
    meta=newMeta;
}

void KoResourceBundle::addFile(QString fileType,QString filePath)
{
    manifest->addTag(fileType,filePath);
}

void KoResourceBundle::removeFile(QString fileName)
{
    QList<QString> list=manifest->removeFile(fileName);

    for (int i=0;i<list.size();i++) {
        meta->removeFirstTag("tag",list.at(i));
    }
}

void KoResourceBundle::addResourceDirs()
{
    QList<QString> listeType = manifest->getDirList();
    for(int i = 0; i < listeType.size();i++) {
        KGlobal::mainComponent().dirs()->addResourceDir(listeType.at(i).toLatin1().data(), this->manager->getKritaPath()+listeType.at(i)+"/"+this->name());
    }
}

bool KoResourceBundle::isInstalled()
{
    return installed;
}

void KoResourceBundle::rename(QString filename,QString name)
{
    QString oldName=meta->getPackName();
    QString shortName=name.section('.',0,0);

    setFilename(filename);
    setName(name);
    addMeta("filename",filename);
    addMeta("name",shortName);
    manifest->rename(shortName);
    if (isInstalled()) {
        QList<QString> directoryList = manifest->getDirList();
        QString dirPath;
        QDir dir;
        for (int i = 0; i < directoryList.size(); i++) {
            dirPath = this->manager->getKritaPath();
            dirPath.append(directoryList.at(i)).append("/");
            dir.rename(dirPath+oldName,dirPath+shortName);
        }
    }
    save();
}

void KoResourceBundle::setThumbnail(QString filename)
{
    thumbnail=QImage(filename);
    save();
}

QString KoResourceBundle::getAuthor()
{
    return meta->getValue("author");
}

QString KoResourceBundle::getLicense()
{
    return meta->getValue("license");
}

QString KoResourceBundle::getWebSite()
{
    return meta->getValue("website");
}

QString KoResourceBundle::getCreated()
{
    return meta->getValue("created");
}

QString KoResourceBundle::getUpdated()
{
    return meta->getValue("updated");
}


