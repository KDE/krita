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


KoResourceManagerControl::KoResourceManagerControl()
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

    model=0;

    filterResourceTypes(0);
}

KoResourceManagerControl::~KoResourceManagerControl()
{
    delete meta;
    delete manifest;
    delete extractor;
    delete model;
    delete bundleServer;
}

void KoResourceManagerControl::setMeta(QModelIndex index,QString type,QString value)
{
    KoResourceBundle *currentBundle = dynamic_cast<KoResourceBundle*>(model->getResourceFromIndex(index));

    if (currentBundle) {
        currentBundle->addMeta(type,value);
    }
}

void KoResourceManagerControl::createPack()
{
    QList<QString> selected = model->getSelectedResource();

    if (!selected.empty()) {
        KoResourceBundle* newBundle=new KoResourceBundle(root+"share/apps/krita/bundles/newBundle.zip");
        newBundle->load();
        newBundle->setName("newBundle");

        for (int i=0; i<selected.size(); i++) {
            newBundle->addFile(selected.at(i).section('/',selected.count("/")-2,selected.count("/")-2),selected.at(i));
        }

        newBundle->addMeta("created",QDate::currentDate().toString("dd/MM/yyyy"));

        bundleServer->addResource(newBundle);
    }
}

//TODO Définir les règles en matière de sélection multiple
void KoResourceManagerControl::modifySelected(int mode)
{
    KoResourceBundle *currentBundle;
    KoResource *currentResource;
    QString currentFileName;
    QList<QString> selected=model->getSelectedResource();

    for (int i=0;i<selected.size();i++) {
        currentFileName=selected.at(i);
        currentResource=model->getResourceFromFilename(currentFileName);
        currentBundle = dynamic_cast<KoResourceBundle*>(currentResource);

        switch (mode) {
        case Install:
            if (currentBundle) {
                currentBundle->install();
                currentBundle->addResourceDirs();
            }
            break;
        case Uninstall:
            if (currentBundle) {
                currentBundle->uninstall();
            }
            break;
        case Delete:
            if (!currentBundle) {
                model->getResourceAdapter(currentResource)->removeResourceFile(currentFileName);
            }
            else {
                if (currentBundle->isInstalled()) {
                    currentBundle->uninstall();
                }
                model->getResourceAdapter(currentBundle)->removeResourceFile(currentFileName);
            }
            model->clearSelected();
            //TODO Décommenter pour supprimer ou rajouter déplacement vers autre dossier pour retour en arrière
            //QFile::remove(currentFileName);
            break;
        }
    }
}

//TODO Bug Foreground to Background
//TODO A tester pr tous les cas possibles
bool KoResourceManagerControl::rename(QModelIndex index,QString newName)
{
    //TODO Résoudre le problème d'appels multiples du rename
    cout<<"Rename"<<endl;
    KoResource* currentResource=model->getResourceFromIndex(index);
    if (currentResource!=0 && currentResource->valid()) {
        KoResourceBundle* currentBundle= dynamic_cast<KoResourceBundle*>(currentResource);

        QString newFilename=currentResource->filename();
        newFilename=newFilename.section('/',0,newFilename.count('/')-1).append("/").append(newName);
        QFile::rename(currentResource->filename(),newFilename);

        if (currentBundle) {//TODO A vérifier si param nécessaire
            currentBundle->rename(newFilename);
        }
        else if (newFilename.count('/')==root.count()+5) {
            QString bundleName=newFilename.section('/',newFilename.count('/')-1,newFilename.count('/')-1);
            QString fileType=newFilename.section('/',newFilename.count('/')-2,newFilename.count('/')-2);
            currentBundle= dynamic_cast<KoResourceBundle*>
                    (model->getResourceFromFilename(bundleName.append(".zip")));
            if (currentBundle) {
                currentBundle->removeFile(currentResource->filename());
                currentBundle->addFile(fileType,newFilename);
            }
        }

        currentResource->setFilename(newFilename);
        currentResource->setName(newName);
        model->getResourceAdapter(currentResource)->updateServer();
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

KoResourceTableModel* KoResourceManagerControl::getModel()
{
    return model;
}

void KoResourceManagerControl::launchServer()
{
    KGlobal::mainComponent().dirs()->addResourceType("ko_bundles", "data", "krita/bundles/");
    bundleServer = new KoResourceServer<KoResourceBundle>("ko_bundles", "*.zip");
    bundleServer->loadResources(bundleServer->fileNames());
}

void KoResourceManagerControl::filterResourceTypes(int index)
{
    QList<QSharedPointer<KoAbstractResourceServerAdapter>> list;

    if(model!=0)
        delete model;

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
        model=new KoResourceTableModel(list);
        break;
    case 1:
        launchServer();
        list.append(QSharedPointer<KoAbstractResourceServerAdapter>(new KoResourceServerAdapter<KoResourceBundle>(bundleServer)));
        model=new KoResourceTableModel(list);
        break;
    case 2:
        list.append(QSharedPointer<KoAbstractResourceServerAdapter>(new KoResourceServerAdapter<KisBrush>(KisBrushServer::instance()->brushServer())));
        model=new KoResourceTableModel(list);
        break;
    case 3:
        list.append(QSharedPointer<KoAbstractResourceServerAdapter>(new KoResourceServerAdapter<KoAbstractGradient>(KoResourceServerProvider::instance()->gradientServer())));
        model=new KoResourceTableModel(list);
        break;
    case 4:
        list.append(QSharedPointer<KoAbstractResourceServerAdapter>(new KoResourceServerAdapter<KisPaintOpPreset>(KisResourceServerProvider::instance()->paintOpPresetServer())));
        model=new KoResourceTableModel(list);
        break;
    case 5:
        list.append(QSharedPointer<KoAbstractResourceServerAdapter>(new KoResourceServerAdapter<KoColorSet>(KoResourceServerProvider::instance()->paletteServer())));
        model=new KoResourceTableModel(list);
        break;
    case 6:
        list.append(QSharedPointer<KoAbstractResourceServerAdapter>(new KoResourceServerAdapter<KoPattern>(KoResourceServerProvider::instance()->patternServer())));
        model=new KoResourceTableModel(list);
        break;
    case 7:
        list.append(QSharedPointer<KoAbstractResourceServerAdapter>(new KoResourceServerAdapter<KisWorkspaceResource>(KisResourceServerProvider::instance()->workspaceServer())));
        model=new KoResourceTableModel(list);
        break;
    }
}
