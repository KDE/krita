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

#include <QProcessEnvironment>
#include "KoResourceManagerControl.h"
#include "KoXmlResourceBundleManifest.h"
#include "KoXmlResourceBundleMeta.h"
#include "KoResourceBundleManager.h"
#include "KoResourceServer.h"
#include "KoResourceBundle.h"
#include <QtCore/QFile>
#include <iostream>
using namespace std;


KoResourceManagerControl::KoResourceManagerControl():currentMeta(""),currentManifest("")
{
    root=QProcessEnvironment::systemEnvironment().value("KDEDIRS").section(':',0,0);
    if (root.at(root.size()-1)!='/') {
        root.append("/");
    }
    extractor=new KoResourceBundleManager(root+"/share/apps/krita/");
    current=new KoResourceBundle(root+"/share/apps/krita/bundles/pack.zip");
    current->load();
}

KoResourceManagerControl::~KoResourceManagerControl()
{
    delete meta;
    delete manifest;
    delete extractor;
}

void KoResourceManagerControl::setMeta(QString packName,QString type,QString value)
{
    Q_UNUSED(packName);
    //if (packName!=currentMeta) {
    //currentMeta=packName; //TODO Vérifier Utilité!!!!!
    current->addMeta(type,value);
}

QIODevice* KoResourceManagerControl::getDevice(QString deviceName){
    //TODO Renvoie le fichier associé au nom passé en paramètre
    //TODO Vérifier l'utilité !!!
    Q_UNUSED(deviceName);
    return new QFile();
}

void KoResourceManagerControl::createPack(QList<QString> resources)
{

    for (int i = 0; i < resources.size(); i++) {
        current->addFile(resources.at(i).section('/',resources.count("/")-2,resources.count("/")-2),resources.at(i));
    }
    current->save();
    current->load();
}

void KoResourceManagerControl::installPack(QString packName)
{
    Q_UNUSED(packName);
    current->install();
    current->addDirs();
    //extractor.setReadPack(packName);
    //extractor.extractPack();
    //TODO Exporter les XML si ce n'est pas fait dans l'extract
}

void KoResourceManagerControl::uninstallPack(QString packName)
{
    cout<<"Uninstall"<<endl;
    Q_UNUSED(packName);
    current->uninstall();
    /*if (packName!=currentMeta) {
        meta=new KoXmlResourceBundleMeta(getDevice(packName.append("-meta.xml")));

    }
    if (packName!=currentManifest) {
        manifest=new KoXmlResourceBundleManifest(getDevice(packName.append("-manifest.xml")));
    }
    currentMeta=packName;
    currentManifest=packName;
    //TODO Renommer l'ancien paquet avec _old dans le dossier contenant les archives
    extractor.createPack(manifest,meta);
    //TODO Supprimer les dossiers
    //TODO Supprimer les fichiers Xml*/
}

void KoResourceManagerControl::deletePack(QString packName)
{
    Q_UNUSED(packName);
    cout<<"Delete"<<endl;
    if(current->isInstalled()) {
        uninstallPack("");
    }
    QFile::remove(root+"pack.zip");
    //TODO Vérifier si le paquet est installé
    //TODO Si oui, supprimer les dossiers et fichiers Xml
    //TODO Supprimer l'archive
}

//TODO Rajouter des paramètres si besoin est (exemple : valeur comboBox etc...)
void KoResourceManagerControl::refreshCurrentTable()
{
    cout<<"Refresh"<<endl;
    //TODO Rafraichir l'affichage du tableau contenu dans l'onglet courant
}

void KoResourceManagerControl::rename(QString old_name,QString new_name)
{
    Q_UNUSED(old_name);
    Q_UNUSED(new_name);
    cout<<"Rename"<<endl;
    //TODO Vérifier si c'est une ressource
    //TODO Si oui, rennomer la ressource et appliquer changement sur manifest si elle appartient à un paquet
    //TODO Si non (<=> c'est un paquet), rennomer le paquet, chacun des dossiers contenant les ressources qui lui corresponde,
    //l'archive stockée et sa mémoire si elle existe, le meta
    //On finit par un refresh de la table
    refreshCurrentTable();

}

void KoResourceManagerControl::about()
{
    cout<<"About"<<endl;
    //TODO Afficher la fenêtre "à propos"
}

void KoResourceManagerControl::launchServer()
{
    KGlobal::mainComponent().dirs()->addResourceType("ko_bundles", "data", "krita/bundles/");
    bundleServer = new KoResourceServer<KoResourceBundle>("ko_bundles", "*.zip");
    bundleServer->addResource(current);
}

KoResourceServer<KoResourceBundle>* KoResourceManagerControl::getServer()
{
    return bundleServer;
}
