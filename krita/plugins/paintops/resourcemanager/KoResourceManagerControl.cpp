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
#include "KoResourceTableModel.h"
#include "KoBundleCreationWidget.h"

#include "KoResourceServerProvider.h"
#include "kis_resource_server_provider.h"
#include "kis_workspace_resource.h"
#include "kis_paintop_preset.h"
#include "kis_brush_server.h"

#include <QFileDialog>
#include <QProcessEnvironment>

#include "resourcemanager.h"

KoResourceManagerControl::KoResourceManagerControl(int nb)
    : m_modelsCount(nb)
{
    for (int i = 0; i < m_modelsCount; i++) {
        m_modelList.append(0);
    }

    filterResourceTypes(0);
}

KoResourceManagerControl::~KoResourceManagerControl()
{
    delete m_meta;
    delete m_manifest;
    delete ResourceBundleServerProvider::instance()->resourceBundleServer();

    for (int i = 0; i < m_modelsCount; i++) {
        delete m_modelList.at(i);
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


void KoResourceManagerControl::configureFilters(int filterType, bool enable)
{
    if (!m_modelList.isEmpty()) {
        m_modelList.at(0)->configureFilters(filterType, enable);
    }
}

void KoResourceManagerControl::toStatus(QString text, int timeout)
{
    emit status(text, timeout);
}

//TODO Design Decision : Un paquet peut il contenir un paquet
bool KoResourceManagerControl::createPack(int type)
{
    KoResourceTableModel *currentModel = getModel(type);
    QList<QString> selected = currentModel->getSelectedResource();

    if (selected.isEmpty()) {
        emit status("No resource selected for bundle creation...", 3000);
        return false;
    }
    else {
        emit status("Creating new bundle...");

        KoXmlResourceBundleMeta* newMeta = new KoXmlResourceBundleMeta();
        KoBundleCreationWidget *bundleCreationWidget = new KoBundleCreationWidget(newMeta);
        connect(bundleCreationWidget, SIGNAL(status(QString, int)), this, SLOT(toStatus(QString, int)));

        if (bundleCreationWidget->exec() == 0 || newMeta == 0) {
            emit status("Creation cancelled", 3000);
            return false;
        }
        else {
            QString bundlePath = ResourceBundleServerProvider::instance()->resourceBundleServer()->saveLocation() + newMeta->getPackName() + ".bundle";
            KoResourceBundle* newBundle = new KoResourceBundle(bundlePath);
            bool isEmpty = true;

            newBundle->load();
            newBundle->setMeta(newMeta);

            for (int i = 0; i < selected.size(); i++) {
                QString currentFileName = selected.at(i);
                KoResource* currentResource = currentModel->getResourceFromFilename(selected.at(i));
                KoResourceBundle* currentBundle = dynamic_cast<KoResourceBundle*>(currentResource);

                if (currentFileName.contains('/') && !currentBundle) {
                    isEmpty = false;
                    newBundle->addFile(currentFileName.section('/', currentFileName.count("/") - 1, currentFileName.count("/") - 1), currentFileName,
                                       currentModel->assignedTagsList(currentResource));
                }
            }

            if (isEmpty) {
                delete newBundle;
                emit status("No valid content to be added to a new bundle...Creation Cancelled", 3000);
                return false;
            }

            newBundle->addMeta("fileName", bundlePath);
            newBundle->addMeta("created", QDate::currentDate().toString("dd/MM/yyyy"));

            ResourceBundleServerProvider::instance()->resourceBundleServer()->addResource(newBundle);

            QStringList tagsList = newBundle->getTagsList();

            for (int i = 0; i < tagsList.size(); i++) {
                ResourceBundleServerProvider::instance()->resourceBundleServer()->addTag(newBundle, tagsList.at(i));
            }

            currentModel->tagCategoryMembersChanged();
            currentModel->clearSelected();
            return true;
        }
    }
}

//TODO Trouver une solution pour que les ressources installées soient visibles
bool KoResourceManagerControl::install(int type)
{
    KoResourceTableModel* currentModel = getModel(type);
    QList<QString> selected = currentModel->getSelectedResource();

    if (selected.isEmpty()) {
        emit status("No bundle selected to be installed...", 3000);
        return false;
    } else {
        KoResourceBundle *currentBundle;
        bool modified = false;

        for (int i = 0; i < selected.size(); i++) {
            currentBundle = dynamic_cast<KoResourceBundle*>(currentModel->getResourceFromFilename(selected.at(i)));

            if (currentBundle) {
                if (!modified) {
                    emit status("Installing bundle(s)...");
                    modified = true;
                }
                currentBundle->install();
                currentBundle->addResourceDirs();
                currentModel->hideResource(currentBundle);
            }
        }

        if (!modified) {
            emit status("No bundle found in current selection : Installation failed...", 3000);
            return false;
        } else {
            currentModel->clearSelected();
            for (int i = 0; i < m_modelsCount; i++) {
                m_modelList.at(i)->refreshBundles();
            }
            return true;
        }
    }
}

bool KoResourceManagerControl::uninstall(int type)
{
    KoResourceTableModel* currentModel = getModel(type);
    QList<QString> selected = currentModel->getSelectedResource();

    if (selected.isEmpty()) {
        emit status("No bundle selected to be uninstalled...", 3000);
        return false;
    } else {
        KoResourceBundle *currentBundle;
        bool modified = false;

        for (int i = 0; i < selected.size(); i++) {
            currentBundle = dynamic_cast<KoResourceBundle*>(currentModel->getResourceFromFilename(selected.at(i)));

            if (currentBundle) {
                if (!modified) {
                    emit status("Uninstalling bundle(s)...");
                    modified = true;
                }
                currentBundle->uninstall();
                currentModel->hideResource(currentBundle);
            }
        }

        if (!modified) {
            emit status("No bundle found in current selection : Uninstallation failed...", 3000);
            return false;
        } else {
            currentModel->clearSelected();
            for (int i = 0; i < m_modelsCount; i++) {
                m_modelList.at(i)->refreshBundles();
            }

            return true;
        }
    }
}

bool KoResourceManagerControl::remove(int type)
{
    KoResourceTableModel* currentModel = getModel(type);
    QList<QString> selected = currentModel->getSelectedResource();

    if (selected.isEmpty()) {
        emit status("No resource selected to be removed...", 3000);
        return false;
    } else {
        QString currentFileName;
        KoResource* currentResource;
        KoResourceBundle* currentBundle;
        bool modified = false;
        bool bundleModified = false;

        for (int i = 0; i < selected.size(); i++) {
            currentFileName = selected.at(i);
            currentResource = currentModel->getResourceFromFilename(currentFileName);
            currentBundle = dynamic_cast<KoResourceBundle*>(currentResource);

            if (currentResource) {
                modified = true;
                QStringList tagsList = currentModel->assignedTagsList(currentResource);

                for (int j = 0; j < tagsList.size(); j++) {
                    currentModel->deleteTag(currentResource, tagsList.at(j));
                }

                if (currentBundle) {
                    bundleModified = true;
                    if (currentBundle->isInstalled()) {
                        currentBundle->uninstall();
                    }
                }

                currentModel->removeResourceFile(currentResource, currentFileName);
                QString delFilename = ResourceBundleServerProvider::instance()->resourceBundleServer()->saveLocation() + "/temp/" + currentFileName.section('/', currentFileName.count('/'));

                QFileInfo fileInfo(delFilename);
                if (fileInfo.exists()) {
                    QTemporaryFile file(fileInfo.path() + "/" + fileInfo.baseName() + "XXXXXX" + "." + fileInfo.suffix());
                    if (file.open()) {
                        delFilename = file.fileName();
                        file.close();
                    }
                }

                QFile::rename(currentFileName, delFilename);
            }
        }

        if (!modified) {
            emit status("No target available to be removed...", 3000);
            return false;
        }

        currentModel->clearSelected();
        for (int i = 0; i < m_modelsCount; i++) {
            currentModel = m_modelList.at(i);
            for (int j = 0; j < selected.size(); j++) {
                m_modelList.at(i)->removeOneSelected(selected.at(j));
            }
            if (bundleModified) {
                m_modelList.at(i)->refreshBundles();
            }
        }
        return true;
    }
}

KoResourceTableModel* KoResourceManagerControl::getModel(int type)
{
    if (type == KoResourceTableModel::Undefined) {
        return m_modelList.at(0);
    } else {
        return m_modelList.at(type);
    }
}

int KoResourceManagerControl::getNbModels()
{
    return m_modelsCount;
}

void KoResourceManagerControl::thumbnail(QModelIndex index, QString fileName, int type)
{
    KoResourceBundle *currentBundle = dynamic_cast<KoResourceBundle*>(getModel(type)->getResourceFromIndex(index));
    if (currentBundle) {
        currentBundle->setThumbnail(fileName);
    }
}

void KoResourceManagerControl::setMeta(QModelIndex index, QString metaType, QString metaValue, int type)
{
    KoResourceBundle *currentBundle = dynamic_cast<KoResourceBundle*>(getModel(type)->getResourceFromIndex(index));

    if (currentBundle) {
        currentBundle->addMeta(metaType, metaValue);
    }
}

void KoResourceManagerControl::saveMeta(QModelIndex index, int type)
{
    KoResourceBundle *currentBundle = dynamic_cast<KoResourceBundle*>(getModel(type)->getResourceFromIndex(index));

    if (currentBundle) {
        currentBundle->save();
    }
}

//TODO Bug Foreground to Background
//TODO Bug rename bouton/label
//TODO Rafraichir réellement un bundle renommé
//TODO Trouver pourquoi les tags sont préservés pour les fichiers mais pas pour les paquets
//TODO Trouver un moyen pour bien différencier et renommer fichier et nom de ressource
//TODO Bug pour les noms qui n'ont pas l'extension du fichier de base
//TODO A tester pr tous les cas possibles
//TODO Rajouter le cas où le nom est déjà utilisé
//TODO Vérifier comment on supprime un tag d'un bundle <=> est ce qu'on supprime la méta donnée dans le xml
//<=>est-ce qu'on vérifie avant de modif le xml si un fichier qu'il contient n'a pas ce tag...
bool KoResourceManagerControl::rename(QModelIndex index, QString newName, int type)
{
    KoResourceTableModel* currentModel = getModel(type);
    KoResource* currentResource = currentModel->getResourceFromIndex(index);

    if (currentResource) {
        QString oldFilename = currentResource->filename();
        QString newFilename = oldFilename.section('/', 0, oldFilename.count('/') - 1) + "/" + newName;

        if (oldFilename != newFilename) {
            KoResourceBundle* currentBundle;
            QStringList tagList = currentModel->assignedTagsList(currentResource);

            if (!currentResource->name().contains('.')) {
                newName = newName.section('.', 0, 0);
            }

            for (int i = 0; i < tagList.size(); i++) {
                currentModel->deleteTag(currentResource, tagList.at(i));
            }

            QFile::rename(oldFilename, newFilename);
            currentResource->setFilename(newFilename);
            currentResource->setName(newName);
            currentBundle = dynamic_cast<KoResourceBundle*>(currentResource);

            if (currentBundle) {
                currentBundle->rename(newFilename, newName);
            } else {
                QString bundleName = newFilename.section('/', newFilename.count('/') - 1, newFilename.count('/') - 1);
                QString fileType = newFilename.section('/', newFilename.count('/') - 2, newFilename.count('/') - 2);

                currentBundle = dynamic_cast<KoResourceBundle*>
                        (currentModel->getResourceFromFilename(bundleName.append(".bundle")));

                if (currentBundle) {
                    currentBundle->removeFile(oldFilename);
                    currentBundle->addFile(fileType, newFilename, tagList);
                }

            }

            for (int i = 0; i < tagList.size(); i++) {
                currentModel->addTag(currentResource, tagList.at(i));
            }

            currentModel->removeOneSelected(oldFilename);
            return true;
        }
    }
    return false;
}

//TODO Voir s'il est intéressant de garder la sélection
void KoResourceManagerControl::filterResourceTypes(int index)
{
    QList<QSharedPointer<KoAbstractResourceServerAdapter> > list;

    for (int i = 0; i < m_modelsCount; i++) {
        KoResourceTableModel* currentModel = m_modelList.at(i);
        if (currentModel != 0) {
            delete currentModel;
        }
    }
    m_modelList.clear();

    switch (index) {
    case 0:
        list.append(QSharedPointer<KoAbstractResourceServerAdapter>(new KoResourceServerAdapter<KoAbstractGradient>(KoResourceServerProvider::instance()->gradientServer())));
        list.append(QSharedPointer<KoAbstractResourceServerAdapter>(new KoResourceServerAdapter<KoPattern>(KoResourceServerProvider::instance()->patternServer())));
        list.append(QSharedPointer<KoAbstractResourceServerAdapter>(new KoResourceServerAdapter<KisBrush>(KisBrushServer::instance()->brushServer())));
        list.append(QSharedPointer<KoAbstractResourceServerAdapter>(new KoResourceServerAdapter<KoColorSet>(KoResourceServerProvider::instance()->paletteServer())));
        list.append(QSharedPointer<KoAbstractResourceServerAdapter>(new KoResourceServerAdapter<KisPaintOpPreset>(KisResourceServerProvider::instance()->paintOpPresetServer())));
        list.append(QSharedPointer<KoAbstractResourceServerAdapter>(new KoResourceServerAdapter<KisWorkspaceResource>(KisResourceServerProvider::instance()->workspaceServer())));
        list.append(QSharedPointer<KoAbstractResourceServerAdapter>(new KoResourceServerAdapter<KoResourceBundle>(ResourceBundleServerProvider::instance()->resourceBundleServer())));
        m_modelList.append(new KoResourceTableModel(list, KoResourceTableModel::Available));
        m_modelList.append(new KoResourceTableModel(list, KoResourceTableModel::Installed));
        break;
    case 1:
        list.append(QSharedPointer<KoAbstractResourceServerAdapter>(new KoResourceServerAdapter<KoResourceBundle>(ResourceBundleServerProvider::instance()->resourceBundleServer())));
        m_modelList.append(new KoResourceTableModel(list, KoResourceTableModel::Available));
        m_modelList.append(new KoResourceTableModel(list, KoResourceTableModel::Installed));
        break;
    case 2:
        list.append(QSharedPointer<KoAbstractResourceServerAdapter>(new KoResourceServerAdapter<KisBrush>(KisBrushServer::instance()->brushServer())));
        m_modelList.append(new KoResourceTableModel(list, KoResourceTableModel::Undefined));
        m_modelList.append(new KoResourceTableModel(list, KoResourceTableModel::Undefined));
        break;
    case 3:
        list.append(QSharedPointer<KoAbstractResourceServerAdapter>(new KoResourceServerAdapter<KoAbstractGradient>(KoResourceServerProvider::instance()->gradientServer())));
        m_modelList.append(new KoResourceTableModel(list, KoResourceTableModel::Undefined));
        m_modelList.append(new KoResourceTableModel(list, KoResourceTableModel::Undefined));
        break;
    case 4:
        list.append(QSharedPointer<KoAbstractResourceServerAdapter>(new KoResourceServerAdapter<KisPaintOpPreset>(KisResourceServerProvider::instance()->paintOpPresetServer())));
        m_modelList.append(new KoResourceTableModel(list, KoResourceTableModel::Undefined));
        m_modelList.append(new KoResourceTableModel(list, KoResourceTableModel::Undefined));
        break;
    case 5:
        list.append(QSharedPointer<KoAbstractResourceServerAdapter>(new KoResourceServerAdapter<KoColorSet>(KoResourceServerProvider::instance()->paletteServer())));
        m_modelList.append(new KoResourceTableModel(list, KoResourceTableModel::Undefined));
        m_modelList.append(new KoResourceTableModel(list, KoResourceTableModel::Undefined));
        break;
    case 6:
        list.append(QSharedPointer<KoAbstractResourceServerAdapter>(new KoResourceServerAdapter<KoPattern>(KoResourceServerProvider::instance()->patternServer())));
        m_modelList.append(new KoResourceTableModel(list, KoResourceTableModel::Undefined));
        m_modelList.append(new KoResourceTableModel(list, KoResourceTableModel::Undefined));
        break;
    case 7:
        list.append(QSharedPointer<KoAbstractResourceServerAdapter>(new KoResourceServerAdapter<KisWorkspaceResource>(KisResourceServerProvider::instance()->workspaceServer())));
        m_modelList.append(new KoResourceTableModel(list, KoResourceTableModel::Undefined));
        m_modelList.append(new KoResourceTableModel(list, KoResourceTableModel::Undefined));
        break;
    }
}


//TODO Rajouter la mise a jour des tags
//TODO Voir si ce traitement ne peut pas etre généraliser ou fait ailleurs
//TODO Lors de l'ajout d'un tag à un paquet, rajouter le tag aussi dans le meta
void KoResourceManagerControl::addFiles(QString bundleName, int type)
{
    KoResourceTableModel* currentModel = getModel(type);
    QList<QString> selected = currentModel->getSelectedResource();
    KoResourceBundle* currentBundle = dynamic_cast<KoResourceBundle*>(ResourceBundleServerProvider::instance()->resourceBundleServer()->resourceByName(bundleName));
    QString path = currentBundle->filename().section('/', 0, currentBundle->filename().count('/') - 2) + "/";

    bundleName = bundleName.section('.', 0, 0);

    if (currentBundle->isInstalled()) {
        for (int i = 0; i < selected.size(); i++) {
            QString currentSelect = selected.at(i);
            QString resourceType = currentSelect.section('/', currentSelect.count("/") - 2, currentSelect.count("/") - 2);

            currentBundle->addFile(resourceType, currentSelect, currentModel->assignedTagsList(currentModel->getResourceFromFilename(currentSelect)));
            QFile::copy(currentSelect, path + resourceType + QString("/") + bundleName + QString("/")
                        + currentSelect.section('/', currentSelect.count("/")));
        }
        currentBundle->save();
    }
}

void KoResourceManagerControl::exportBundle(int type)
{
    KoResourceTableModel* currentModel = getModel(type);
    QList<QString> selected = currentModel->getSelectedResource();

    if (selected.isEmpty()) {
        emit status("No bundle selected to be exported...", 3000);
        return;
    } else {
        QString dirPath;
        KoResourceBundle *currentBundle;
        bool modified = false;

        for (int i = 0; i < selected.size(); i++) {
            currentBundle = dynamic_cast<KoResourceBundle*>(currentModel->getResourceFromFilename(selected.at(i)));

            if (currentBundle) {
                if (!modified) {
                    emit status("Exporting bundle(s)...");
                    dirPath = QFileDialog::getExistingDirectory(0, tr("Directory"), QProcessEnvironment::systemEnvironment().value("HOME").section(':', 0, 0), QFileDialog::ShowDirsOnly);
                    dirPath.append("/");
                    modified = true;
                }
                currentBundle->save();
                QString fileName = currentBundle->filename();
                QFile::copy(fileName, dirPath + fileName.section('/', fileName.count('/')));
            }
        }

        if (!modified) {
            emit status("No bundle found in current selection : Export failed...", 3000);
        } else {
            emit status("Bundle(s) exported successfully", 3000);

            currentModel->clearSelected();
            for (int i = 0; i < m_modelsCount; i++) {
                m_modelList.at(i)->refreshBundles();
            }
        }
    }
}

bool KoResourceManagerControl::importBundle()
{
    emit status("Importing...");
    QString filePath = QFileDialog::getOpenFileName(0,
                       tr("Import Bundle"), QProcessEnvironment::systemEnvironment().value("HOME").section(':', 0, 0), tr("Archive Files (*.bundle)"));

    if (!filePath.isEmpty()) {
        QFile::copy(filePath, ResourceBundleServerProvider::instance()->resourceBundleServer()->saveLocation());
        emit status("Bundle imported successfully", 3000);
        return true;
    } else {
        emit status("Import aborted ! ", 3000);
        return false;
    }
}

//TODO Problème nb de valeurs affichées sur le précédent onglet
void KoResourceManagerControl::refreshTaggingManager()
{
    for (int i = 0; i < m_modelsCount; i++) {
        KoResourceTableModel *currentModel = m_modelList.at(i);
        currentModel->enableResourceFiltering(false);
        currentModel->setCurrentTag(QString());
        currentModel->refreshResources();
    }
}
