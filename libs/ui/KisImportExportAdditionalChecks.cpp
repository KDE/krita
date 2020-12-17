/*
 *  SPDX-FileCopyrightText: 2019 Agata Cacko <cacko.azh@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisImportExportAdditionalChecks.h"
#include <QFileInfo>


bool KisImportExportAdditionalChecks::isFileWritable(QString filepath)
{
    QFileInfo finfo(filepath);
    return finfo.isWritable();
}

bool KisImportExportAdditionalChecks::isFileReadable(QString filepath)
{
    QFileInfo finfo(filepath);
    return finfo.isReadable();
}

bool KisImportExportAdditionalChecks::doesFileExist(QString filepath)
{
    return QFile::exists(filepath);
}

