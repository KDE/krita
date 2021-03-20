/*
 *  SPDX-FileCopyrightText: 2019 Agata Cacko <cacko.azh@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_IMPORT_EXPORT_ADDITIONAL_CHECKS_H
#define KIS_IMPORT_EXPORT_ADDITIONAL_CHECKS_H

#include <QString>
#include <KisImportExportErrorCode.h>

class KRITAUI_EXPORT KisImportExportAdditionalChecks
{

public:

    static bool isFileWritable(QString filepath);
    static bool isFileReadable(QString filepath);
    static bool doesFileExist(QString filepath);
};




#endif // #ifndef KIS_IMPORT_EXPORT_ADDITIONAL_CHECKS_H
