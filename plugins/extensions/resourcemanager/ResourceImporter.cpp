/*
 * SPDX-FileCopyrightText: 2021 Agata Cacko <cacko.azh@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "ResourceImporter.h"

#include <QItemSelection>
#include <QPainter>
#include <QStandardPaths>
#include <QMessageBox>

#include <KoFileDialog.h>

#include <KisResourceModel.h>
#include <kis_assert.h>
#include <KisResourceTypes.h>
#include <KisMainWindow.h>
#include <KisResourceTypeModel.h>
#include <KisResourceLoaderRegistry.h>
#include <KisMimeDatabase.h>
#include <KisStorageModel.h>
#include <KisResourceLocator.h>
#include <kis_config.h>

#include "DlgResourceTypeForFile.h"

ResourceImporter::ResourceImporter(QWidget *parent)
    : m_widgetParent(parent)
{
    initialize();
}


ResourceImporter::~ResourceImporter()
{

}

void ResourceImporter::importResources(QString startPath)
{
    // TODO: remove debug after finishing the importer
    bool debug = false;

    if (startPath.isEmpty()) {
        startPath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    }

    KoFileDialog dialog(m_widgetParent, KoFileDialog::OpenFiles, "krita_resources");
    dialog.setDefaultDir(startPath);
    dialog.setCaption(i18nc("Resource Importer file dialog title", "Import Resources and Resource Libraries"));
    dialog.setMimeTypeFilters(m_allMimetypes);
    QStringList filenames = dialog.filenames();


    if (debug) qCritical() << "All filenames: " << filenames;


    QMap<QString, QString> troublesomeFiles;
    QStringList troublesomeMimetypes;

    QMap<QString, QStringList> troublesomeFilesPerMimetype;



    QMap<QString, QString> resourceTypePerFile;

    QStringList failedImportedFiles;
    QStringList successfullyImportedFiles;



    for (int i = 0; i < filenames.count(); i++) {
        QString mimetype = KisMimeDatabase::mimeTypeForFile(filenames[i]);
        if (m_storagesMimetypes.contains(mimetype)) {
            if (debug) qCritical() << "We're loading a storage here: " << filenames[i];

            // import the bundle/asl/abr storage

            KisStorageModel::StorageImportOption importMode = KisStorageModel::None;
            if (debug) qCritical() << "checking for storage" << filenames[i] << QFileInfo(filenames[i]).fileName();
            // TODO: three options in case of the same filename: cancel; overwrite; rename;
            // but for now, let's just skip
            bool skip = false;
            if (KisResourceLocator::instance()->hasStorage(QFileInfo(filenames[i]).fileName())) {
                skip = true;
                /*
                if (QMessageBox::warning(m_widgetParent, i18nc("@ttile:window", "Warning"),
                                         i18n("There is already a resource library with this name installed. Do you want to overwrite it? Resource library name: "),
                                         QMessageBox::Ok | QMessageBox::Cancel) == QMessageBox::Cancel) {
                    importMode = KisStorageModel::Rename;
                }
                else {
                    importMode = KisStorageModel::Overwrite;
                }
                */
            }
            if (skip || !KisStorageModel::instance()->importStorage(filenames[i], importMode)) {
                failedImportedFiles << filenames[i];
            } else {
                successfullyImportedFiles << filenames[i];
            }

        } else if (m_zipMimetypes.contains(mimetype)) {
            // TODO: unpack a zip file and then proceed the same way like with others
        } else if (m_resourceTypesForMimetype.contains(mimetype)) {
            bool differentResourceTypes = m_resourceTypesForMimetype.value(mimetype).count() > 1;
            if (differentResourceTypes) {
                if (debug) qCritical() << "We have a difficult situation here!" << filenames[i];

                troublesomeFiles.insert(filenames[i], mimetype);
                if (!troublesomeMimetypes.contains(mimetype)) {
                    troublesomeMimetypes.append(mimetype);
                }

                if (!troublesomeFilesPerMimetype.contains(mimetype)) {
                    troublesomeFilesPerMimetype.insert(mimetype, QStringList());
                }
                troublesomeFilesPerMimetype[mimetype] = troublesomeFilesPerMimetype[mimetype] << filenames[i];

            } else {
                if (debug) qCritical() << "We're loading a" << mimetype << " here: " << filenames[i];
                resourceTypePerFile.insert(filenames[i], m_resourceTypesForMimetype[mimetype][0]);
            }
        } else {
            failedImportedFiles << filenames[i];
        }

    }

    QMap<QString, QStringList> troublesomeResourceTypesPerMimetype;
    for (int i = 0; i < troublesomeMimetypes.size(); i++) {
        troublesomeResourceTypesPerMimetype.insert(troublesomeMimetypes[i], m_resourceTypesForMimetype[troublesomeMimetypes[i]]);
    }

    if (troublesomeMimetypes.count() > 0) {
        DlgResourceTypeForFile dlg(m_widgetParent, troublesomeResourceTypesPerMimetype);
        if (dlg.exec() == QDialog::Accepted) {
            for (int i = 0; i < troublesomeMimetypes.count(); i++) {
                QString resourceType = dlg.getResourceTypeForMimetype(troublesomeMimetypes[i]);
                QString mimetype = troublesomeMimetypes[i];
                if (troublesomeFilesPerMimetype.contains(mimetype)) {
                    for (int j = 0; j < troublesomeFilesPerMimetype[mimetype].size(); j++) {
                        resourceTypePerFile.insert(troublesomeFilesPerMimetype[mimetype][j], resourceType);
                    }
                }
            }
        } else {
            for (int i = 0; i < troublesomeMimetypes.count(); i++) {
                for (int j = 0; j < troublesomeFilesPerMimetype[troublesomeMimetypes[i]].size(); j++) {
                    failedImportedFiles << troublesomeFilesPerMimetype[troublesomeMimetypes[i]][j];
                }
            }
        }
    }




    if (debug) qCritical() <<  "Resource types for mimetype: ";

    for (int i = 0; i < m_resourceTypesForMimetype.keys().size(); i++) {
        if (m_resourceTypesForMimetype[m_resourceTypesForMimetype.keys()[i]].size() > 1) {
            if (debug) qCritical() << m_resourceTypesForMimetype.keys()[i] << m_resourceTypesForMimetype[m_resourceTypesForMimetype.keys()[i]];
        }
    }





    QStringList resourceFiles = resourceTypePerFile.keys();
    for (int i = 0; i < resourceFiles.count(); i++) {
        QString resourceType = resourceTypePerFile[resourceFiles[i]];
        if (debug) qCritical() << "Loading " << resourceFiles[i] << "as" << resourceType;
        if (m_resourceModelsForResourceType.contains(resourceType)) {
            if (debug) qCritical() << "We do have a resource model for that!";
            KisResourceModel* model = m_resourceModelsForResourceType[resourceType];
            KoResourceSP res = model->importResourceFile(resourceFiles[i]);
            if (res.isNull()) {
                if (debug) qCritical() << "But the resource is null :( ";
                failedImportedFiles << resourceFiles[i];
            } else {
                if (debug) qCritical() << "The resource isn't null, great!";
                successfullyImportedFiles << resourceFiles[i];
            }
        } else {
            failedImportedFiles << resourceFiles[i];
        }
    }

    if (debug) qCritical() << "Failed files: " << failedImportedFiles;
    if (debug) qCritical() << "Successfully imported files: " << successfullyImportedFiles;

    if (failedImportedFiles.count() > 0) {
        QMessageBox::StandardButton button = QMessageBox::information(m_widgetParent, i18nc("Resource Importer dialog warning", "Import of some files failed"),
                                                                      i18nc("Resource Importer dialog warning", "The following files couldn't have been imported: %1",
                                                                            failedImportedFiles.join(", ")));
        Q_UNUSED(button);
    }

}

void ResourceImporter::prepareTypesMaps()
{
    m_storagesMimetypes = QStringList() << "application/x-krita-bundle"
                                        << "image/x-adobe-brushlibrary"
                                        << "application/x-photoshop-style-library";

    m_zipMimetypes = QStringList(); // << "application/zip"; // TODO: implement for zip archives

    QStringList resourceTypes;
    KisResourceTypeModel model;
    for (int i = 0; i < model.rowCount(); i++) {
        QModelIndex idx = model.index(i, 0);
        resourceTypes << model.data(idx, Qt::UserRole + KisResourceTypeModel::ResourceType).toString();
    }
    qCritical() << "resource types = " << resourceTypes;

    m_mimetypeForResourceType.clear();
    m_resourceTypesForMimetype.clear();

    QStringList mimetypes;
    for (int i = 0; i < resourceTypes.count(); i++) {
        QStringList mime = KisResourceLoaderRegistry::instance()->mimeTypes(resourceTypes[i]);
        m_mimetypeForResourceType.insert(resourceTypes[i], mime);
        mimetypes << mime;
        for (int j = 0; j < mime.count(); j++) {
            if (m_resourceTypesForMimetype.contains(mime[j])) {
                if (!m_resourceTypesForMimetype[mime[j]].contains(resourceTypes[i])) {
                    m_resourceTypesForMimetype[mime[j]].append(resourceTypes[i]);
                }
            } else {
                m_resourceTypesForMimetype.insert(mime[j], QStringList() << resourceTypes[i]);
            }
        }
    }

    m_allMimetypes << m_storagesMimetypes;
    m_allMimetypes << m_zipMimetypes;
    m_allMimetypes << mimetypes;


    m_allMimetypes.removeDuplicates();



}

void ResourceImporter::prepareModelsMap()
{
    KisResourceTypeModel model;
    for (int i = 0; i < model.rowCount(); i++) {
        QModelIndex idx = model.index(i, 0);
        QString resourceType = model.data(idx, Qt::UserRole + KisResourceTypeModel::ResourceType).toString();
        if (!m_resourceModelsForResourceType.contains(resourceType)) {
            KisResourceModel* model = new KisResourceModel(resourceType);
            if (model) {
                m_resourceModelsForResourceType.insert(resourceType, model);
            } else {
                dbgResources << "There is no KisResourceModel available for " << resourceType;
            }
        }
    }
}

void ResourceImporter::initialize()
{
    if (!m_isInitialized) {
        prepareTypesMaps();
        prepareModelsMap();
        m_isInitialized = true;
    }
}








