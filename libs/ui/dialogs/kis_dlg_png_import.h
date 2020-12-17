/*
 * SPDX-FileCopyrightText: 2015 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef KIS_DLG_PNG_IMPORT_H
#define KIS_DLG_PNG_IMPORT_H

#include <KoDialog.h>
#include <QString>

#include "ui_wdgdlgpngimport.h"


class KisDlgPngImport : public KoDialog
{
    Q_OBJECT

public:
    KisDlgPngImport(const QString &path, const QString &colorModelID, const QString &colorDepthID, QWidget *parent = 0);
    QString profile() const;

private:

    Ui_WdgDlgPngImport dlgWidget;

};

#endif // KIS_DLG_PNG_IMPORT_H
