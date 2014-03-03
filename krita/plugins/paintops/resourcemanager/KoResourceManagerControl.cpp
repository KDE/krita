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

#include "KoResourceManagerControl.h"
#include "KoXmlResourceBundleManifest.h"
#include "KoXmlResourceBundleMeta.h"
#include <QtCore/QFile>
#include <iostream>
using namespace std;


ManagerControl::ManagerControl():currentMeta(""),currentManifest("")
{
    //extractor=new KoResourceBundleManager("path/vers/krita");
}

ManagerControl::~ManagerControl()
{
    delete meta;
    delete manifest;
    //delete extractor;
}

void ManagerControl::setMeta(QString packName,QString type,QString value)
{
    Q_UNUSED(value);
    if (packName!=currentMeta) {
        //meta=new KoXmlResourceBundleMeta(getDevice(packName.append("-meta.xml")));
        currentMeta=packName;
        //meta->addTag(type,value);
        cout<<qPrintable(type)<<endl;
    }
}

QIODevice* ManagerControl::getDevice(QString deviceName){
    //TODO Renvoie le fichier associé au nom passé en paramètre
    Q_UNUSED(deviceName);
    return new QFile();
}

void ManagerControl::installPack(QString packName)
{
    Q_UNUSED(packName);
    cout<<"Install"<<endl;
    //extractor.setReadPack(packName);
    //extractor.extractPack();
    //TODO Exporter les XML si ce n'est pas fait dans l'extract
}

void ManagerControl::uninstallPack(QString packName)
{
    cout<<"Uninstall"<<endl;
    Q_UNUSED(packName);
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

void ManagerControl::deletePack(QString packName)
{
    cout<<"Delete"<<endl;
    Q_UNUSED(packName);
    //TODO Vérifier si le paquet est installé
    //TODO Si oui, supprimer les dossiers et fichiers Xml
    //TODO Supprimer l'archive
}


//TODO Rajouter des paramètres si besoin est (exemple : valeur comboBox etc...)
void ManagerControl::refreshCurrentTable()
{
    cout<<"Refresh"<<endl;
    //TODO Rafraichir l'affichage du tableau contenu dans l'onglet courant
}

void ManagerControl::rename(QString old_name,QString new_name)
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

void ManagerControl::about()
{
    cout<<"About"<<endl;
    //TODO Afficher la fenêtre "à propos"
}

