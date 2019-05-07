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
#ifndef KIS_IMPORT_EXPORT_ADDITIONAL_CHECKS_H
#define KIS_IMPORT_EXPORT_ADDITIONAL_CHECKS_H

#include <QString>
#include <KisImportExportErrorCode.h>

class KRITAUI_EXPORT KisImportExportAdditionalChecks
{

public:

    bool isFileWriteable(QString filepath) const;
    bool isFileReadable(QString filepath) const;
    bool doesFileExist(QString filepath) const;

    KisImportExportErrorCode checkWriteErrors(QString filepath) const;
    KisImportExportErrorCode checkReadErrors(QString filepath) const;


};




#endif // #ifndef KIS_IMPORT_EXPORT_ADDITIONAL_CHECKS_H
