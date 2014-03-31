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
    installed=false; //TODO Vérifier l'utilité
    manager=new KoResourceBundleManager(QProcessEnvironment::systemEnvironment().value("KDEDIRS").section(':',0,0).append("/share/apps/krita/"));
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
        meta->addTag("name",filename(),true);
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

//TODO Vérifier que l'updated est bien placé
bool KoResourceBundle::save()
{
    if (manager->bad()) {
        meta->addTags(manifest->getTagList());
    }
    addMeta("updated",QDate::currentDate().toString("dd/MM/yyyy"));

    manager->createPack(manifest,meta);

    setValid(true);

    return load();
}

void KoResourceBundle::install()
{
    load(); //TODO Vérifier si ce load est nécessaire
    if (!manager->bad()) {
        manager->extractKFiles(manifest->getFilesToExtract());
        manifest->exportTags();
        //TODO Vérifier que l'export est validé et copié dans les fichiers
        //TODO Sinon, déterminer pourquoi et comment faire
        installed=true;
        manifest->install(manager->getKritaPath(),this->filename());
        save();
        //TODO Modifier les chemins des fichiers si c'est la première installation
    }
}

void KoResourceBundle::uninstall()
{
    if (!installed)
        return;

    QList<QString> directoryList = manifest->getDirList();
    QString shortPackName = meta->getShortPackName();
    QString dirPath;

    for (int i = 0; i < directoryList.size(); i++) {
        dirPath = this->manager->getKritaPath();
        dirPath.append(directoryList.at(i)).append("/").append(shortPackName);

        if (!removeDir(dirPath)) {
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
    meta->show();
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

bool KoResourceBundle::removeDir(const QString & dirName)
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

void KoResourceBundle::rename(QString filename)
{
    addMeta("name",filename);
    if (isInstalled()) {
        QList<QString> directoryList = manifest->getDirList();
        QString dirPath;
        QDir dir;
        for (int i = 0; i < directoryList.size(); i++) {
            dirPath = this->manager->getKritaPath();
            dirPath.append(directoryList.at(i)).append("/").append(filename);
            dir.rename(dirPath,dirPath.section('/',0,dirPath.count('/')-1).append("/").append(filename));
        }
    }
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
