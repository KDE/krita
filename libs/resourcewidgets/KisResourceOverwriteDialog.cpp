/*
 *    This file is part of the KDE project
 *    SPDX-FileCopyrightText: 2021 Agata Cacko <cacko.azh@gmail.com>
 *
 *    SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KisResourceOverwriteDialog.h"

#include <QMessageBox>
#include <QFileInfo>

#include <klocalizedstring.h>

#include <KisResourceLocator.h>

bool KisResourceOverwriteDialog::userAllowsOverwrite(QWidget* widgetParent, QString resourceFilepath)
{
    return QMessageBox::question(widgetParent, i18nc("Dialog title", "Overwrite the file?"),
                          i18nc("Question in a dialog/messagebox", "This resource file already exists in the resource folder. Do you want to overwrite it?\nResource filename: %1", QFileInfo(resourceFilepath).fileName()),
                                 QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Cancel) != QMessageBox::Cancel;
}

bool KisResourceOverwriteDialog::resourceExistsInResourceFolder(QString resourceType, QString filepath)
{
    QString resourceLocationBase = KisResourceLocator::instance()->resourceLocationBase();
    QString newFilepath = resourceLocationBase + "/" + resourceType + "/" + QFileInfo(filepath).fileName();
    QFileInfo fi(newFilepath);
    return fi.exists();
}
