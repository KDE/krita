/*
 *  SPDX-FileCopyrightText: 2018 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISRESOURCEMODELPROVIDER_H
#define KISRESOURCEMODELPROVIDER_H

#include <qglobal.h>

#include "kritaresources_export.h"

class KisAllResourcesModel;
class KisAllTagsModel;
class KisAllTagResourceModel;

/**
 * KisResourceModelProvider should be used to retrieve resource models.
 * For every resource type, there is only one instance of the resource model,
 * so all views on these models show the same state.
 */ 
class KRITARESOURCES_EXPORT KisResourceModelProvider
{
public:
    KisResourceModelProvider();
    ~KisResourceModelProvider();

    static KisAllResourcesModel *resourceModel(const QString &resourceType);
    static KisAllTagsModel *tagModel(const QString& resourceType);
    static KisAllTagResourceModel *tagResourceModel(const QString& resourceType);

    static void testingResetAllModels();

private:

    struct Private;
    Private *const d;

    Q_DISABLE_COPY(KisResourceModelProvider)
};

#endif // KISRESOURCEMODELPROVIDER_H
