/*
 *  Copyright (c) 2016 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef KIS_DUALBRUSHPAINTOP_SETTINGS_WIDGET_H_
#define KIS_DUALBRUSHPAINTOP_SETTINGS_WIDGET_H_

#include <kis_paintop_settings_widget.h>

#include "ui_WdgDualBrushOptions.h"

class KisDualBrushOpOption;

class KisDualBrushPaintOpSettingsWidget : public KisPaintOpSettingsWidget
{
    Q_OBJECT

public:
    KisDualBrushPaintOpSettingsWidget(QWidget* parent = 0);
    virtual ~KisDualBrushPaintOpSettingsWidget();

    KisPropertiesConfiguration* configuration() const;

    ///Reimplemented
    void changePaintOpSize(qreal x, qreal y);
    virtual QSizeF paintOpSize() const;

public:
    KisDualBrushOpOption* m_DualBrushOption;
};

#endif
