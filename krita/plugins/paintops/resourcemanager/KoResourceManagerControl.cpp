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
#include "KoBundleCreationWidget.h"

#include "KoResourceServerProvider.h"
#include "kis_resource_server_provider.h"
#include "kis_workspace_resource.h"
#include "kis_paintop_preset.h"
#include "kis_brush_server.h"

#include <sys/stat.h>

#include <QFileDialog>
#include <QtCore/QProcessEnvironment>
#include <iostream>
using namespace std;


KoResourceManagerControl::KoResourceManagerControl(int nb): nbModels(nb)
{
    root=QProcessEnvironment::systemEnvironment().value("KDEDIRS");

    if (root.isEmpty()) {
        exit(1);
    }

    root=root.section(':',0,0);

    if (root.at(root.size()-1)!='/') {
        root.append('/');
    }

    extractor=new KoResourceBundleManager(root+"share/apps/krita/");

    for(int i=0;i<nbModels;i++) {
        modelList.append(0);
    }

    mkdir(QString(root+"share/apps/krita/temp/").toUtf8().constData(),S_IRWXU|S_IRGRP|S_IXGRP);

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

//Voir si on autorise la modification d'un paquet qui n'est pas installé
//Si oui, pb peut se poser pour récupérer les fichiers si leur chemin source n'est plus valable
//ou si la ressource pointée par le chemin a été modifiée à l'intérieur du paquet
//(exemple : installation modif désinstallation)
//Préférable au moins pour les fichiers de demander l'installation
//avant tout ajout, modif, suppression de fichiers

//TODO Une solution est l'extraction de tous les fichiers existants dans un dossier temporaire
//Puis on construit l'archive a partir de ses fichiers qui sont obligatoirement les plus récents


void KoResourceManagerControl::configureFilters(int filterType, bool enable) {
    if (!modelList.isEmpty()) {
        modelList.at(0)->configureFilters(filterType,enable);
    }
}

void KoResourceManagerControl::toStatus(QString text,int timeout)
{
    emit status(text,timeout);
}

void KoResourceManagerControl::createPack(int type)
{
    KoResourceTableModel *currentModel=getModel(type);
    QList<QString> selected = currentModel->getSelectedResource();

    if (selected.isEmpty()) {
        emit status("No resource selected for bundle creation...",3000);
    }
    else {
        emit status("Creating new bundle...");

        KoXmlResourceBundleMeta* newMeta=new KoXmlResourceBundleMeta();
        KoBundleCreationWidget *bundleCreationWidget = new KoBundleCreationWidget(newMeta);
        connect(bundleCreationWidget,SIGNAL(status(QString,int)),this,SLOT(toStatus(QString,int)));

        if (bundleCreationWidget->exec()==0 || newMeta==0) {
            emit status("Creation cancelled",3000);
            return;
        }
        else {
            QString bundlePath=root+"share/apps/krita/bundles/"+newMeta->getPackName()+".zip";
            KoResourceBundle* newBundle=new KoResourceBundle(bundlePath);

            newBundle->load();
            newBundle->setMeta(newMeta);

            for (int i=0; i<selected.size(); i++) {
                newBundle->addFile(selected.at(i).section('/',selected.count("/")-2,selected.count("/")-2),selected.at(i));
            }

            newBundle->addMeta("fileName",bundlePath);
            newBundle->addMeta("created",QDate::currentDate().toString("dd/MM/yyyy"));

            bundleServer->addResource(newBundle);
            currentModel->clearSelected();
            emit status("New bundle created successfully");
        }
    }
}

void KoResourceManagerControl::install(int type)
{
    KoResourceTableModel* currentModel=getModel(type);
    QList<QString> selected=currentModel->getSelectedResource();

    if (selected.isEmpty()) {
        emit status("No bundle selected to be installed...",3000);
        return;
    }
    else {
        KoResourceBundle *currentBundle;
        bool modified=false;

        for (int i=0;i<selected.size();i++) {
            currentBundle = dynamic_cast<KoResourceBundle*>(currentModel->getResourceFromFilename(selected.at(i)));

            if (currentBundle) {
                if(!modified) {
                    emit status("Installing bundle(s)...");
                    modified=true;
                }
                currentBundle->install();
                currentBundle->addResourceDirs();
                currentModel->hideResource(currentBundle);
            }
        }

        if(!modified) {
            emit status("No bundle found in current selection : Installation failed...",3000);
        }
        else {
            emit status("Bundle(s) installed successfully",3000);

            currentModel->clearSelected();
            for (int i=0;i<nbModels;i++) {
                modelList.at(i)->refreshBundles();
            }
        }
    }
}

void KoResourceManagerControl::uninstall(int type)
{
    KoResourceTableModel* currentModel=getModel(type);
    QList<QString> selected=currentModel->getSelectedResource();

    if (selected.isEmpty()) {
        emit status("No bundle selected to be uninstalled...",3000);
        return;
    }
    else {
        KoResourceBundle *currentBundle;
        bool modified=false;

        for (int i=0;i<selected.size();i++) {
            currentBundle = dynamic_cast<KoResourceBundle*>(currentModel->getResourceFromFilename(selected.at(i)));

            if (currentBundle) {
                if(!modified) {
                    emit status("Uninstalling bundle(s)...");
                    modified=true;
                }
                currentBundle->uninstall();
                currentModel->hideResource(currentBundle);
            }
        }

        if(!modified) {
            emit status("No bundle found in current selection : Uninstallation failed...",3000);
        }
        else {
            currentModel->clearSelected();
            for (int i=0;i<nbModels;i++) {
                modelList.at(i)->refreshBundles();
            }

            emit status("Bundle(s) uninstalled successfully",3000);
        }
    }
}

void KoResourceManagerControl::remove(int type)
{
    KoResourceTableModel* currentModel=getModel(type);
    QList<QString> selected=currentModel->getSelectedResource();

    if (selected.isEmpty()) {
        emit status("No resource selected to be removed...",3000);
        return;
    }
    else {
        QString currentFileName;
        KoResource* currentResource;
        KoResourceBundle* currentBundle;
        bool modified=false;

        for (int i=0;i<selected.size();i++) {
            currentFileName=selected.at(i);
            currentResource=currentModel->getResourceFromFilename(currentFileName);
            currentBundle = dynamic_cast<KoResourceBundle*>(currentResource);

            if (currentBundle) {
                modified=true;
                if (currentBundle->isInstalled()) {
                    currentBundle->uninstall();
                }
            }
            currentModel->removeResourceFile(currentResource,currentFileName);
            QString delFilename = root+"share/apps/krita/temp/"+currentFileName.section('/',currentFileName.count('/'));

            QFileInfo fileInfo(delFilename);
            if (fileInfo.exists()) {
                QTemporaryFile file(fileInfo.path() + "/" + fileInfo.baseName() + "XXXXXX" + "." + fileInfo.suffix());
                if (file.open()) {
                    delFilename=file.fileName();
                    file.close();
                }
            }

            QFile::rename(currentFileName,delFilename);
        }

        currentModel->clearSelected();
        for (int i=0;i<nbModels;i++) {
            currentModel=modelList.at(i);
            for (int j=0;j<selected.size();j++) {
                modelList.at(i)->removeOneSelected(selected.at(j));
            }
            if (modified) {
                modelList.at(i)->refreshBundles();
            }
        }
        emit status("Resource(s) removed successfully",3000);
    }
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

int KoResourceManagerControl::getNbModels()
{
    return nbModels;
}

void KoResourceManagerControl::launchServer()
{
    KGlobal::mainComponent().dirs()->addResourceType("ko_bundles", "data", "krita/bundles/");
    bundleServer = new KoResourceServer<KoResourceBundle>("ko_bundles", "*.zip");
    bundleServer->loadResources(bundleServer->fileNames());
}

void KoResourceManagerControl::thumbnail(QModelIndex index,QString fileName,int type)
{
    KoResourceBundle *currentBundle = dynamic_cast<KoResourceBundle*>(getModel(type)->getResourceFromIndex(index));
    if (currentBundle) {
        currentBundle->setThumbnail(fileName);
    }
}






void KoResourceManagerControl::setMeta(QModelIndex index,QString metaType,QString metaValue,int type)
{
    KoResourceBundle *currentBundle = dynamic_cast<KoResourceBundle*>(getModel(type)->getResourceFromIndex(index));

    if (currentBundle) {
        currentBundle->addMeta(metaType,metaValue);
    }
}

void KoResourceManagerControl::setMeta(KoResourceBundle *bundle, QString metaType,QString metaValue)
{
    if (bundle) {
        bundle->addMeta(metaType,metaValue);
    }
}

void KoResourceManagerControl::saveMeta(QModelIndex index,int type)
{
    KoResourceBundle *currentBundle = dynamic_cast<KoResourceBundle*>(getModel(type)->getResourceFromIndex(index));

    if (currentBundle) {
        currentBundle->save();
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
        QString newFilename=oldFilename.section('/',0,oldFilename.count('/')-1)+"/"+newName;
        if (oldFilename!=newFilename) {

            QFile::rename(oldFilename,newFilename);
            currentResource->setFilename(newFilename);
            currentResource->setName(newName);

            if (currentBundle) {
                currentBundle->rename(newFilename,newName);
            }
            else if (newFilename.count('/')==root.count()+5) {
                QString bundleName=newFilename.section('/',newFilename.count('/')-1,newFilename.count('/')-1);
                QString fileType=newFilename.section('/',newFilename.count('/')-2,newFilename.count('/')-2);
                currentBundle= dynamic_cast<KoResourceBundle*>
                        (currentModel->getResourceFromFilename(bundleName.append(".zip")));
                if (currentBundle) {
                    currentBundle->removeFile(oldFilename);
                    currentBundle->addFile(fileType,newFilename);
                }
            }

            currentModel->removeOneSelected(oldFilename);
        }
    }

    return true;
    //model->refresh();
    //TODO Définir dans quels cas le renommage échoue
}

//TODO Voir s'il est intéressant de garder la sélection
void KoResourceManagerControl::filterResourceTypes(int index)
{
    QList<QSharedPointer<KoAbstractResourceServerAdapter> > list;

    for (int i=0;i<nbModels;i++) {
        KoResourceTableModel* currentModel=modelList.at(i);
        if (currentModel!=0) {
            delete currentModel;
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


//TODO Rajouter la mise a jour des tags
//TODO Voir si ce traitement ne peut pas etre généraliser ou fait ailleurs

void KoResourceManagerControl::addFiles(QString bundleName,int type)
{
    KoResourceTableModel* currentModel=getModel(type);
    QList<QString> selected=currentModel->getSelectedResource();
    KoResourceBundle* currentBundle=dynamic_cast<KoResourceBundle*> (bundleServer->resourceByName(bundleName));
    QString path=currentBundle->filename().section('/',0,currentBundle->filename().count('/')-2)+"/";

    bundleName=bundleName.section('.',0,0);

    if (currentBundle->isInstalled()) {
        for (int i=0;i<selected.size();i++) {
            QString currentSelect=selected.at(i);
            QString resourceType=currentSelect.section('/',currentSelect.count("/")-2,currentSelect.count("/")-2);

            currentBundle->addFile(resourceType,currentSelect);
            QFile::copy(currentSelect,path+resourceType+QString("/")+bundleName+QString("/")
                        +currentSelect.section('/',currentSelect.count("/")));
        }
        currentBundle->save();
    }
}

void KoResourceManagerControl::exportBundle(int type)
{
    KoResourceTableModel* currentModel=getModel(type);
    QList<QString> selected=currentModel->getSelectedResource();

    if (selected.isEmpty()) {
        emit status("No bundle selected to be exported...",3000);
        return;
    }
    else {
        QString dirPath;
        KoResourceBundle *currentBundle;
        bool modified=false;

        for (int i=0;i<selected.size();i++) {
            currentBundle = dynamic_cast<KoResourceBundle*>(currentModel->getResourceFromFilename(selected.at(i)));

            if (currentBundle) {
                if(!modified) {
                    emit status("Exporting bundle(s)...");
                    dirPath= QFileDialog::getExistingDirectory (0, tr("Directory"),QProcessEnvironment::systemEnvironment().value("HOME").section(':',0,0),QFileDialog::ShowDirsOnly);
                    dirPath.append("/");
                    modified=true;
                }
                currentBundle->save();
                QString fileName = currentBundle->filename();
                QFile::copy(fileName,dirPath+fileName.section('/',fileName.count('/')));
            }
        }

        if(!modified) {
            emit status("No bundle found in current selection : Export failed...",3000);
        }
        else {
            emit status("Bundle(s) exported successfully",3000);

            currentModel->clearSelected();
            for (int i=0;i<nbModels;i++) {
                modelList.at(i)->refreshBundles();
            }
        }
    }
}

bool KoResourceManagerControl::importBundle()
{
    emit status("Importing...");
    QString filePath = QFileDialog::getOpenFileName(0,
         tr("Import Bundle"), QProcessEnvironment::systemEnvironment().value("HOME").section(':',0,0), tr("Archive Files (*.zip)"));

    if (!filePath.isEmpty()) {
        QFile::copy(filePath,root+"share/apps/krita/bundles/"+filePath.section('/',filePath.count('/')));
        emit status("Bundle imported successfully",3000);
        return true;
    }
    else {
        emit status("Import aborted ! ",3000);
        return false;
    }
}

//TODO Problème nb de valeurs affichées sur le précédent onglet
void KoResourceManagerControl::refreshTaggingManager()
{
    for(int i=0;i<nbModels;i++){
        KoResourceTableModel *currentModel = modelList.at(i);
        currentModel->enableResourceFiltering(false);
        currentModel->setCurrentTag(QString());
        currentModel->refreshResources();
    }
}
