/*
 * SPDX-FileCopyrightText: 2021 Agata Cacko <cacko.azh@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KIS_RESOURCE_OVERWRITE_DIALOG_H
#define KIS_RESOURCE_OVERWRITE_DIALOG_H

#include <QWidget>
#include "kritaresourcewidgets_export.h"
#include <KoResource.h>
#include <KisResourceModel.h>


class KRITARESOURCEWIDGETS_EXPORT KisResourceUserOperations
{

public:
    // used when the user tries to import or create a new resource that already exists
    // for example if they import the same resource file (the filename matches)
    // or when they try to create a new resource with the same name and filename
    static bool userAllowsOverwrite(QWidget* widgetParent, QString resourceFilepath);
    static bool resourceExistsInResourceFolder(QString resourceType, QString filepath);

    // used when the user tries to rename a resource to a name that already exists
    // (it's permitted but can confuse the user later)
    static bool userAllowsRename(QWidget* widgetParent, QString name);



    static KoResourceSP importResourceFileWithUserInput(QWidget *widgetParent, KisResourceModel* resourceModel, QString storageLocation, QString resourceType, QString resourceFilepath);
    static bool renameResourceWithUserInput(QWidget* widgetParent, KisResourceModel* resourceModel, KoResourceSP resource, QString resourceName);
    static bool addResourceWithUserInput(QWidget* widgetParent, KisResourceModel* resourceModel, KoResourceSP resource, QString storageLocation = "");
    static bool updateResourceWithUserInput(QWidget* widgetParent, KisResourceModel* resourceModel, KoResourceSP resource);




};

#endif // KIS_RESOURCE_OVERWRITE_DIALOG_H
