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

KoResourceBundle::KoResourceBundle(QString const& file):KoResource(file)
{

}

KoResourceBundle::~KoResourceBundle()
{
    delete man;
    delete meta;
    delete manifest;
}

QImage KoResourceBundle::image() const
{
    return thumbnail;
}

bool KoResourceBundle::load()
{
    man.setReadPack(fileName());
    if(man.bad()){
        //Le fichier n'existe pas
        manifest=new KoXmlResourceBundleManifest();
        meta=new KoXmlResourceBundleManifest();
    }
    else{
        //Le fichier existe
        //TODO Tester si getfile suffit au lieu de getfiledata
        //TODO Vérifier si on peut éviter de recréer manifest et meta à chaque load
        manifest=new KoXmlResourceBundleManifest(man->getFileData("manifest.xml"));
        meta=new KoXmlResourceBundleManifest(man->getFileData("meta.xml"));
        thumbnail.load(man->getFile("thumbnail.jpg");
        setValid(true);
    }
    return true;
}

bool KoResourceBundle::save()
{
    if(man.bad()){
        //Le fichier n'existe pas
        meta.addTags(manifest.getTags());
    }
    man.setWritePack(fileName());
    man.createPack(manifest,meta);
    setValid(true);
    return true;
}

void KoResourceBundle::addFile(QString fileType,QString filePath)
{
    manifest->addTag(fileType,filePath);
    //TODO Voir s'il faut copier ou pas le fichier tout de suite
    //TODO Cas où le paquet n'est pas installé...
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
