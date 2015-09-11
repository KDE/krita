/*
 * Copyright (C) 2015 Boudewijn Rempt <boud@valdyas.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
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
