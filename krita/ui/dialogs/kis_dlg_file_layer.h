/* This file is part of the KDE project
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2013
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
#ifndef KIS_DLG_FILE_LAYER_H
#define KIS_DLG_FILE_LAYER_H

#include <kdialog.h>
#include <QString>

class QLabel;
class QGroupBox;
class KisConfigWidget;
class KLineEdit;

#include "ui_wdgdlgfilelayer.h"

/**
 * Create a new file layer
 */
class KisDlgFileLayer : public KDialog
{
public:

    Q_OBJECT

public:

    /**
     * Create a new file layer
     * @param name the proposed name for this layer
     * @param parent the widget parent of this dialog
     */
    KisDlgFileLayer(const QString &basePath, const QString &name, QWidget *parent = 0);
    QString fileName() const;
    QString layerName() const;
    bool scaleToImageResolution() const;

protected slots:

    void slotNameChanged(const QString &);
    void slotSelectFile();

private:

    Ui_WdgDlgFileLayer dlgWidget;
    QString m_basePath;
    bool m_customName;
    bool m_freezeName;
};

#endif
