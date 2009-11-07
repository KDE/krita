/*
 *  Copyright (c) 2009 Boudewijn Rempt <boud@valdyas.org>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KIS_MYPAINT_PAINTOP_SETTINGS_WIDGET_H_
#define KIS_MYPAINT_PAINTOP_SETTINGS_WIDGET_H_

#include <kis_paintop_settings_widget.h>

#include "ui_wdgmypaintoptions.h"
#include "widgets/kis_popup_button.h"

class MyPaintBrushResource;
class MyBrushResourcesListModel;

class MyPaintSettingsWidget : public KisPaintOpSettingsWidget
{

    Q_OBJECT

public:
    MyPaintSettingsWidget(QWidget* parent = 0);
    virtual ~MyPaintSettingsWidget();

    void setConfiguration( const KisPropertiesConfiguration * config);
    KisPropertiesConfiguration* configuration() const;
    void writeConfiguration( KisPropertiesConfiguration *config ) const;

    MyPaintBrushResource* brush() const;

private slots:

    void brushSelected(const QModelIndex&);

private:
    Ui::WdgMyPaintOptions* m_options;
    MyBrushResourcesListModel* m_model;
    QString m_activeBrushFilename;
};

#endif
