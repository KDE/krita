/* This file is part of the KDE project
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2008
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
#ifndef KIS_DLG_GENERATORLAYER_H
#define KIS_DLG_GENERATORLAYER_H

#include <kdialog.h>
#include <QString>

class KisFilter;
class QListWidgetItem;
class QLabel;
class KisPreviewWidget;
class KisFilterConfiguration;
class QGroupBox;
class KisFilterConfigWidget;
class KLineEdit;

#include "ui_wdgdlggeneratorlayer.h"

/**
 * Create a new generator layer
 */
class KisDlgGeneratorLayer : public KDialog
{
public:

    Q_OBJECT

public:

    /**
     * Create a new generator layer dialog
     *
     * @param parent the widget parent of this dialog
     */
    KisDlgGeneratorLayer( QWidget *parent = 0 );

    KisFilterConfiguration * configuration() const;
    QString layerName() const;

protected slots:

    void slotNameChanged( const QString & );

private:

    Ui_WdgDlgGeneratorLayer dlgWidget;
    KisFilterConfigWidget * m_currentConfigWidget;
    KisGeneratorSP m_currentGenerator;
    bool m_customName;
    bool m_freezeName;
};

#endif
