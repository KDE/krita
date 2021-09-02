/*
 * SPDX-FileCopyrightText: 2021 Agata Cacko <cacko.azh@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KIS_RESOURCE_OVERWRITE_DIALOG_H
#define KIS_RESOURCE_OVERWRITE_DIALOG_H

#include <QWidget>
#include "kritaresourcewidgets_export.h"


class KRITARESOURCEWIDGETS_EXPORT KisResourceOverwriteDialog
{

public:
    static bool userAllowsOverwrite(QWidget* widgetParent, QString resourceFilepath);
    static bool resourceExistsInResourceFolder(QString resourceType, QString filepath);




};

#endif // KIS_RESOURCE_OVERWRITE_DIALOG_H
