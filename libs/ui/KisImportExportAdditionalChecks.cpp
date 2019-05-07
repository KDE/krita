/*
 *  Copyright (c) 2019 Agata Cacko <cacko.azh@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "KisImportExportAdditionalChecks.h"



bool KisImportExportAdditionalChecks::isFileWriteable(QString filepath) const
{
    QFile file(filepath);
    bool ret = file.open(QIODevice::WriteOnly);
    if (ret) {
        file.close();
    }
    return ret;
}

bool KisImportExportAdditionalChecks::isFileReadable(QString filepath) const
{
    QFile file(filepath);
    bool ret = file.open(QIODevice::ReadOnly);
    if (ret) {
        file.close();
    }
    return ret;
}

bool KisImportExportAdditionalChecks::doesFileExist(QString filepath) const
{
    return QFile::exists(filepath);
}

