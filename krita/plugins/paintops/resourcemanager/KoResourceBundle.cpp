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

#include <QtCore/QDir>
#include "KoResourceBundle.h"
#include "KoXmlResourceBundleManifest.h"
#include "KoXmlResourceBundleMeta.h"
#include "KoResourceBundleManager.h"
#include <iostream>
using namespace std;

KoResourceBundle::KoResourceBundle(QString const& bundlePath, QString kritaPath):KoResource(bundlePath)
{
    manager=new KoResourceBundleManager(kritaPath);
    isInstalled=false; //TODO A vérifier
}

KoResourceBundle::~KoResourceBundle()
{
    delete manager;
    delete meta;
    delete manifest;
}

bool KoResourceBundle::load()
{
    manager->setReadPack(filename());
    if (manager->bad()) {
        manifest=new KoXmlResourceBundleManifest();
        meta=new KoXmlResourceBundleMeta();
        meta->addTag("name",filename(),true);
    }
    else {
        //TODO Vérifier si on peut éviter de recréer manifest et meta à chaque load
        manifest=new KoXmlResourceBundleManifest(manager->getFile("manifest.xml"));
        meta=new KoXmlResourceBundleMeta(manager->getFile("meta.xml"));
        thumbnail.load(manager->getFile("thumbnail.jpg"),"jpg");
        manager->close();
        setValid(true);
    }
    return true;
}

bool KoResourceBundle::save()
{
    if (manager->bad()) {
        meta->addTags(manifest->getTagList());
    }
    manager->createPack(manifest,meta);
    setValid(true);
    load();
    return true;
}

void KoResourceBundle::addMeta(QString type,QString value){
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

QString KoResourceBundle::defaultFileExtension() const
{
    return QString(".zip");
}

QImage KoResourceBundle::image() const
{
	return thumbnail;
}

void KoResourceBundle::install()
{
    load();
    if (!manager->bad()) {
        manager->extractKFiles(manifest->getFilesToExtract());
        manifest->exportTags();
        //TODO Vérifier que l'export est validé et copié dans les fichiers
        //TODO Sinon, déterminer pourquoi et comment faire
        isInstalled=true;
        //Modifier les chemins des fichiers si c'est la première installation
    }
}

void KoResourceBundle::uninstall()
{
    if(!isInstalled)
        return;

    QList<QString> directoryList = manifest->getDirList();
    QString shortPackName = meta->getShortPackName();
    QString dirPath;

    for (int i = 0; i < directoryList.size(); i++) {
        dirPath = this->manager->kritaPath;
        dirPath.append(directoryList.at(i)).append("/").append(shortPackName);

        if (!removeDir(dirPath)) {
            cerr<<"Error : Couldn't delete folder : "<<qPrintable(dirPath)<<endl;
        }
    }    
    isInstalled=false;
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
