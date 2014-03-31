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
#include "KoResourceBundleManager.h"
#include "KoResourceTableModel.h"

#include "KoResourceServerProvider.h"
#include "kis_resource_server_provider.h"
#include "kis_workspace_resource.h"
#include "kis_paintop_preset.h"
#include "kis_brush_server.h"

#include <QtCore/QProcessEnvironment>
#include <iostream>
using namespace std;


KoResourceManagerControl::KoResourceManagerControl(int nb):
    nbModels(nb)
{
    QString env=QProcessEnvironment::systemEnvironment().value("KDEDIRS");

    if (env.isEmpty()) {
        exit(1);
    }

    root=env.section(':',0,0);

    if (root.at(root.size()-1)!='/') {
        root.append('/');
    }

    extractor=new KoResourceBundleManager(root+"share/apps/krita/");

    for(int i=0;i<nbModels;i++) {
        modelList.append(0);
    }

    filterResourceTypes(0);
}

KoResourceManagerControl::~KoResourceManagerControl()
{
    delete meta;
    delete manifest;
    delete extractor;
    delete bundleServer;

    for (int i=0;i<nbModels;i++) {
        delete modelList.at(i);
    }
}

void KoResourceManagerControl::setMeta(QModelIndex index,QString metaType,QString metaValue,int type)
{
    KoResourceBundle *currentBundle = dynamic_cast<KoResourceBundle*>(getModel(type)->getResourceFromIndex(index));

    if (currentBundle) {
        currentBundle->addMeta(metaType,metaValue);
    }
}

void KoResourceManagerControl::saveMeta(QModelIndex index,int type)
{
    KoResourceBundle *currentBundle = dynamic_cast<KoResourceBundle*>(getModel(type)->getResourceFromIndex(index));

    if (currentBundle) {
        currentBundle->save();
    }
}

//Réutilisation d'un bout de code de addResource
//et execution double de ce code
void KoResourceManagerControl::createPack(int type)
{
    KoResourceTableModel *currentModel=getModel(type);
    QList<QString> selected = currentModel->getSelectedResource();

    if (!selected.empty()) {
        QString bundlePath=root+"share/apps/krita/bundles/newBundle.zip";
        QFileInfo fileInfo(bundlePath);

        if (fileInfo.exists()) {
            QString filename = fileInfo.path() + "/" + fileInfo.baseName() + "XXXXXX" + "." + fileInfo.suffix();
            QTemporaryFile file(filename);
            if (file.open()) {
                kDebug() << "File already exists ; now " << file.fileName();
                bundlePath=file.fileName();
            }
        }

        KoResourceBundle* newBundle=new KoResourceBundle(bundlePath);
        newBundle->load();
        newBundle->setName(bundlePath.section('/',bundlePath.count('/')));

        for (int i=0; i<selected.size(); i++) {
            newBundle->addFile(selected.at(i).section('/',selected.count("/")-2,selected.count("/")-2),selected.at(i));
        }

        newBundle->addMeta("created",QDate::currentDate().toString("dd/MM/yyyy"));

        bundleServer->addResource(newBundle);
        currentModel->clearSelected(); //TODO Voir s'il est nécessaire ou pas
    }
}

//TODO Définir les règles en matière de sélection multiple
void KoResourceManagerControl::modifySelected(int mode,int type)
{
    KoResourceBundle *currentBundle;
    KoResource *currentResource;
    QString currentFileName;
    KoResourceTableModel* currentModel=getModel(type);
    QList<QString> selected=currentModel->getSelectedResource();
    bool modified=false;

    for (int i=0;i<selected.size();i++) {
        currentFileName=selected.at(i);
        currentResource=currentModel->getResourceFromFilename(currentFileName);
        currentBundle = dynamic_cast<KoResourceBundle*>(currentResource);

        switch (mode) {
        case Install:
            if (currentBundle && type==KoResourceTableModel::Available) {
                currentBundle->install();
                currentBundle->addResourceDirs();
                currentModel->hideResource(currentResource);
                modified=true;
            }
            break;
        case Uninstall:
            if (currentBundle && type==KoResourceTableModel::Installed) {
                currentBundle->uninstall();
                currentModel->hideResource(currentResource);
                modified=true;
            }
            break;
        case Delete:
            if (currentBundle && currentBundle->isInstalled()) {
                currentBundle->uninstall();
            }
            currentModel->removeResourceFile(currentResource,currentFileName);
            modified=true;
            //TODO Décommenter pour supprimer ou rajouter déplacement vers autre dossier pour retour en arrière
            //QFile::remove(currentFileName);
            break;
        }
    }

    if (modified) {
        //TODO Voir s'il faut rajouter une condition (cas où les bundles ne sont pas affectés)
        for (int i=0;i<nbModels;i++) {
            modelList.at(i)->refreshBundles();
        }
    }
}

//TODO Bug Foreground to Background
//TODO A tester pr tous les cas possibles
//TODO Rajouter le cas où le nom est déjà utilisé
bool KoResourceManagerControl::rename(QModelIndex index,QString newName,int type)
{
    KoResourceTableModel* currentModel=getModel(type);
    KoResource* currentResource=currentModel->getResourceFromIndex(index);
    if (currentResource!=0 && currentResource->valid()) {
        KoResourceBundle* currentBundle= dynamic_cast<KoResourceBundle*>(currentResource);

        QString oldFilename=currentResource->filename();
        QString newFilename=oldFilename.section('/',0,oldFilename.count('/')-1).append("/").append(newName);
        if (oldFilename!=newFilename) {

            cout<<"Rename"<<endl;

            QFile::rename(currentResource->filename(),newFilename);

            if (currentBundle) {//TODO A vérifier si param nécessaire
                currentBundle->rename(newFilename);
            }
            else if (newFilename.count('/')==root.count()+5) {
                QString bundleName=newFilename.section('/',newFilename.count('/')-1,newFilename.count('/')-1);
                QString fileType=newFilename.section('/',newFilename.count('/')-2,newFilename.count('/')-2);
                currentBundle= dynamic_cast<KoResourceBundle*>
                        (currentModel->getResourceFromFilename(bundleName.append(".zip")));
                if (currentBundle) {
                    currentBundle->removeFile(currentResource->filename());
                    currentBundle->addFile(fileType,newFilename);
                }
            }
            currentResource->setFilename(newFilename);
            currentResource->setName(newName);
            currentModel->removeOneSelected(oldFilename);
            //currentModel->getResourceAdapter(currentResource)->updateServer(); //TODO A Rajouter si utile
        }
    }

    return true;
    //TODO Poser question différence entre filename et name
    //TODO Si besoin rajouter un currentResource->setFilename(newName);

    //cout<<qPrintable(newFilename)<<endl;
    /*currentResource->setName(newName);*/
    //currentResource->save();

    //TODO Rajouter le cas où c'est une ressource appartenant à un paquet
    //model->refresh();
    //TODO Définir dans quels cas le renommage échoue
}

void KoResourceManagerControl::about()
{
    cout<<"About"<<endl;
    //TODO Afficher la fenêtre "à propos"
}

KoResourceTableModel* KoResourceManagerControl::getModel(int type)
{
    if (type==KoResourceTableModel::Undefined) {
        return modelList.at(0);
    }
    else {
        return modelList.at(type);
    }
}

void KoResourceManagerControl::launchServer()
{
    KGlobal::mainComponent().dirs()->addResourceType("ko_bundles", "data", "krita/bundles/");
    bundleServer = new KoResourceServer<KoResourceBundle>("ko_bundles", "*.zip");
    bundleServer->loadResources(bundleServer->fileNames());
}

//TODO Voir s'il est intéressant de garder la sélection
void KoResourceManagerControl::filterResourceTypes(int index)
{
    QList<QSharedPointer<KoAbstractResourceServerAdapter> > list;

    for (int i=0;i<nbModels;i++) {
        KoResourceTableModel* currentModel=modelList.at(i);
        if (currentModel!=0 && currentModel->getDataType()!=KoResourceTableModel::Undefined && i!=1) {
            delete modelList.at(i);
        }
    }
    modelList.clear();

    switch (index) {
    case 0:
        launchServer();
        list.append(QSharedPointer<KoAbstractResourceServerAdapter>(new KoResourceServerAdapter<KoAbstractGradient>(KoResourceServerProvider::instance()->gradientServer())));
        list.append(QSharedPointer<KoAbstractResourceServerAdapter>(new KoResourceServerAdapter<KoPattern>(KoResourceServerProvider::instance()->patternServer())));
        list.append(QSharedPointer<KoAbstractResourceServerAdapter>(new KoResourceServerAdapter<KisBrush>(KisBrushServer::instance()->brushServer())));
        list.append(QSharedPointer<KoAbstractResourceServerAdapter>(new KoResourceServerAdapter<KoColorSet>(KoResourceServerProvider::instance()->paletteServer())));
        list.append(QSharedPointer<KoAbstractResourceServerAdapter>(new KoResourceServerAdapter<KisPaintOpPreset>(KisResourceServerProvider::instance()->paintOpPresetServer())));
        list.append(QSharedPointer<KoAbstractResourceServerAdapter>(new KoResourceServerAdapter<KisWorkspaceResource>(KisResourceServerProvider::instance()->workspaceServer())));
        list.append(QSharedPointer<KoAbstractResourceServerAdapter>(new KoResourceServerAdapter<KoResourceBundle>(bundleServer)));
        modelList.append(new KoResourceTableModel(list,KoResourceTableModel::Available));
        modelList.append(new KoResourceTableModel(list,KoResourceTableModel::Installed));
        break;
    case 1:
        launchServer();
        list.append(QSharedPointer<KoAbstractResourceServerAdapter>(new KoResourceServerAdapter<KoResourceBundle>(bundleServer)));
        modelList.append(new KoResourceTableModel(list,KoResourceTableModel::Available));
        modelList.append(new KoResourceTableModel(list,KoResourceTableModel::Installed));
        break;
    case 2:
        list.append(QSharedPointer<KoAbstractResourceServerAdapter>(new KoResourceServerAdapter<KisBrush>(KisBrushServer::instance()->brushServer())));
        modelList.append(new KoResourceTableModel(list,KoResourceTableModel::Undefined));
        modelList.append(new KoResourceTableModel(list,KoResourceTableModel::Undefined));
        break;
    case 3:
        list.append(QSharedPointer<KoAbstractResourceServerAdapter>(new KoResourceServerAdapter<KoAbstractGradient>(KoResourceServerProvider::instance()->gradientServer())));
        modelList.append(new KoResourceTableModel(list,KoResourceTableModel::Undefined));
        modelList.append(new KoResourceTableModel(list,KoResourceTableModel::Undefined));
        break;
    case 4:
        list.append(QSharedPointer<KoAbstractResourceServerAdapter>(new KoResourceServerAdapter<KisPaintOpPreset>(KisResourceServerProvider::instance()->paintOpPresetServer())));
        modelList.append(new KoResourceTableModel(list,KoResourceTableModel::Undefined));
        modelList.append(new KoResourceTableModel(list,KoResourceTableModel::Undefined));
        break;
    case 5:
        list.append(QSharedPointer<KoAbstractResourceServerAdapter>(new KoResourceServerAdapter<KoColorSet>(KoResourceServerProvider::instance()->paletteServer())));
        modelList.append(new KoResourceTableModel(list,KoResourceTableModel::Undefined));
        modelList.append(new KoResourceTableModel(list,KoResourceTableModel::Undefined));
        break;
    case 6:
        list.append(QSharedPointer<KoAbstractResourceServerAdapter>(new KoResourceServerAdapter<KoPattern>(KoResourceServerProvider::instance()->patternServer())));
        modelList.append(new KoResourceTableModel(list,KoResourceTableModel::Undefined));
        modelList.append(new KoResourceTableModel(list,KoResourceTableModel::Undefined));
        break;
    case 7:
        list.append(QSharedPointer<KoAbstractResourceServerAdapter>(new KoResourceServerAdapter<KisWorkspaceResource>(KisResourceServerProvider::instance()->workspaceServer())));
        modelList.append(new KoResourceTableModel(list,KoResourceTableModel::Undefined));
        modelList.append(new KoResourceTableModel(list,KoResourceTableModel::Undefined));
        break;
    }
}

int KoResourceManagerControl::getNbModels()
{
    return nbModels;
}
