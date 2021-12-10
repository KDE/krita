/*
 *    This file is part of the KDE project
 *    SPDX-FileCopyrightText: 2021 Agata Cacko <cacko.azh@gmail.com>
 *
 *    SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KisResourceUserOperations.h"

#include <QMessageBox>
#include <QFileInfo>
#include <QBuffer>

#include <klocalizedstring.h>

#include <KisResourceLocator.h>
#include <KisResourceModel.h>
#include <KisResourceCacheDb.h>
#include <kis_assert.h>
#include <KisGlobalResourcesInterface.h>


bool KisResourceUserOperations::userAllowsOverwrite(QWidget* widgetParent, QString resourceFilepath)
{
    return QMessageBox::question(widgetParent, i18nc("Dialog title", "Overwrite the file?"),
                          i18nc("Question in a dialog/messagebox", "This resource file already exists in the resource folder. "
                                                                   "Do you want to overwrite it?\nResource filename: %1", QFileInfo(resourceFilepath).fileName()),
                                 QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Cancel) != QMessageBox::Cancel;
}

bool KisResourceUserOperations::resourceNameIsAlreadyUsed(KisResourceModel *resourceModel, QString resourceName, int resourceIdToIgnore)
{
    auto sizeFilteredById = [resourceIdToIgnore] (QVector<KoResourceSP> list) {
        int sumHere = 0;
        if (resourceIdToIgnore < 0) {
            return list.size();
        }

        for (int i = 0; i < list.size(); i++) {
            if (list[i]->resourceId() != resourceIdToIgnore) {
                sumHere++;
            }
        }
        return sumHere;
    };

    QVector<KoResourceSP> resourcesWithTheSameExactName = resourceModel->resourcesForName(resourceName);
    if (sizeFilteredById(resourcesWithTheSameExactName) > 0) {
        return true;
    }

    QVector<KoResourceSP> resourcesWithSpacesReplacedByUnderlines = resourceModel->resourcesForName(resourceName.replace(" ", "_"));
    if (sizeFilteredById(resourcesWithSpacesReplacedByUnderlines) > 0) {
        return true;
    }

    return false;
}

KoResourceSP KisResourceUserOperations::importResourceFileWithUserInput(QWidget *widgetParent, QString storageLocation, QString resourceType, QString resourceFilepath)
{
    KisResourceModel resourceModel(resourceType);
    resourceModel.setResourceFilter(KisResourceModel::ShowActiveResources); // inactive don't count here

    KoResourceSP resource = resourceModel.importResourceFile(resourceFilepath, false, storageLocation);
    if (resource.isNull() && storageLocation == "" && resourceModel.importWillOverwriteResource(resourceFilepath, storageLocation)) {
        if (KisResourceUserOperations::userAllowsOverwrite(widgetParent, resourceFilepath)) {
            resource = resourceModel.importResourceFile(resourceFilepath, true, storageLocation);
        } else {
            return nullptr; // the user doesn't want to import the file anymore because they don't want to overwrite it
        }
    }
    if (!resource) {
        QMessageBox::warning(widgetParent, i18nc("@title:window", "Failed to import the resource"), i18nc("Warning message", "Failed to import the resource."));
    }
    return resource;
}

bool KisResourceUserOperations::renameResourceWithUserInput(QWidget *widgetParent, KoResourceSP resource, QString resourceName)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(resource, false);
    KisResourceModel resourceModel(resource->resourceType().first);
    resourceModel.setResourceFilter(KisResourceModel::ShowActiveResources); // inactive don't count here

    if (resourceNameIsAlreadyUsed(&resourceModel, resourceName, resource->resourceId())) {
        bool userWantsRename = QMessageBox::question(widgetParent, i18nc("@title:window", "Rename the resource?"),
                              i18nc("Question in a dialog/messagebox", "This name is already used for another resource. "
                                                                       "Do you want to use the same name for multiple resources?"
                                                                       "(If you decline now, the resource won't be renamed)."),
                                     QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Cancel) != QMessageBox::Cancel;
        if (!userWantsRename) {
            return false;
        }
    }
    bool res = resourceModel.renameResource(resource, resourceName);
    if (!res) {
        QMessageBox::warning(widgetParent, i18nc("@title:window", "Failed to rename the resource"), i18nc("Warning message", "Failed to rename the resource."));
    }
    return res;
}

bool KisResourceUserOperations::addResourceWithUserInput(QWidget *widgetParent, KoResourceSP resource, QString storageLocation)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(resource, false);
    KisResourceModel resourceModel(resource->resourceType().first);

    resourceModel.setResourceFilter(KisResourceModel::ShowAllResources); // we want to consider all resources later when searching for the same name

    // check if adding the resource is possible: it is not if there is a resource with the same filename in the storage the user want to save the resource to

    typedef enum {ADD, OVERWRITE, CANCEL} Action;
    Action action = ADD;

    int resourceWithThatFilenameId;

    if (KisResourceCacheDb::getResourceIdFromVersionedFilename(resource->filename(), resource->resourceType().first, storageLocation, resourceWithThatFilenameId)) {

        KoResourceSP resource = resourceModel.resourceForId(resourceWithThatFilenameId);
        bool userWantsOverwrite = QMessageBox::question(widgetParent, i18nc("@title:window", "Overwrite the resource?"),
                              i18nc("Question in a dialog/messagebox", "This filename is already used for another resource. "
                                                                       "Do you want to overwrite that resource?\n"
                                                                       "(If you decline now, nothing will be done)."),
                                     QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Cancel) != QMessageBox::Cancel;
        if (userWantsOverwrite) {
            action = OVERWRITE;
        } else {
            return false;
        }
    }
    // check if there are any other resources with the same name, even in different storages or with different filenames
    else if (resourceNameIsAlreadyUsed(&resourceModel, resource->name()))
    {
        bool userWantsAdd = QMessageBox::question(widgetParent, i18nc("@title:window", "Add the resource?"),
                              i18nc("Question in a dialog/messagebox", "This name is already used for another resource. "
                                                                       "Do you want to use the same name for multiple resources? "
                                                                       "(If you decline now, the resource won't be added)."),
                                     QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Cancel) != QMessageBox::Cancel;

        if (!userWantsAdd) {
            action = CANCEL;
            return false;
        } else {
            action = ADD;
        }
    }

    if (action == ADD) {
        bool res = resourceModel.addResource(resource, storageLocation);
        if (!res) {
            QMessageBox::warning(widgetParent, i18nc("@title:window", "Failed to add resource"), i18nc("Warning message", "Failed to add the resource."));
        }
        return res;
    } else if (action == OVERWRITE) {
        bool res = updateResourceWithUserInput(widgetParent, resource);
        // no error message, because it's handled in that function
        return res;
    }
    // shouldn't get here
    return false;
}

bool KisResourceUserOperations::updateResourceWithUserInput(QWidget *widgetParent, KoResourceSP resource)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(resource, false);
    KisResourceModel resourceModel(resource->resourceType().first);
    resourceModel.setResourceFilter(KisResourceModel::ShowActiveResources); // inactive don't count here

    if (resource->resourceId() < 0) {
        // that's a resource that didn't come from a database
        // we assume that filename and storageLocation are correct, though
        if (QFileInfo(resource->storageLocation()).isRelative()) {
            QString storageLocation = resource->storageLocation();
            resource->setStorageLocation(KisResourceLocator::instance()->makeStorageLocationAbsolute(storageLocation));
        }

        int outResourceId;
        // note that we need to check for any file that exists so we can't use KisResourceModel here
        // because the model only keeps the current resource filename
        bool result = KisResourceCacheDb::getResourceIdFromVersionedFilename(resource->filename(), resource->resourceType().first,
                                                               KisResourceLocator::instance()->makeStorageLocationRelative(resource->storageLocation()), outResourceId);
        KoResourceSP cachedPointer;
        if (outResourceId >= 0) {
            cachedPointer = resourceModel.resourceForId(outResourceId);
        }

        if (!cachedPointer || !resource->isSerializable() || !cachedPointer->isSerializable()) {
            QMessageBox::warning(widgetParent, i18nc("@title:window", "Failed to overwrite the resource"), i18nc("Warning message", "Failed to overwrite the resource."));
            return false;
        }
        // now we need to move data from the provided pointer to the pointer from the database
        QBuffer buffer;
        buffer.open(QIODevice::ReadWrite);

        resource->saveToDevice(&buffer);
        buffer.close();
        buffer.open(QIODevice::ReadWrite);

        cachedPointer->loadFromDevice(&buffer, KisGlobalResourcesInterface::instance());
        buffer.close();
        resource = cachedPointer;
    }

    QString oldName = resourceModel.data(resourceModel.indexForResourceId(resource->resourceId()), Qt::UserRole + KisAllResourcesModel::Name).toString();
    if (resource->name() != oldName) {
        // rename in action
        if (resourceNameIsAlreadyUsed(&resourceModel, resource->name(), resource->resourceId())) {
            bool userWantsRename = QMessageBox::question(widgetParent, i18nc("@title:window", "Rename the resource?"),
                                  i18nc("Question in a dialog/messagebox", "This name is already used for another resource. Do you want to overwrite "
                                                                           "and use the same name for multiple resources?"
                                                                           "\nIf you cancel, your changes won't be saved."),
                                         QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Cancel) != QMessageBox::Cancel;
            if (!userWantsRename) {
                return false;
            }
        }
    }

    bool res = resourceModel.updateResource(resource);
    if (!res) {
        QMessageBox::warning(widgetParent, i18nc("@title:window", "Failed to overwrite the resource"), i18nc("Warning message", "Failed to overwrite the resource."));
    }
    return res;
}
