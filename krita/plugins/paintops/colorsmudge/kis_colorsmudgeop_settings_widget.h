/*
 *  Copyright (C) 2011 Silvio Heinrich <plassy@web.de>
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

#ifndef KIS_COLORSMUDGEOP_SETTINGS_WIDGET_H_
#define KIS_COLORSMUDGEOP_SETTINGS_WIDGET_H_

#include <kis_brush_based_paintop_options_widget.h>

class KisBrushOptionWidget;

class KisColorSmudgeOpSettingsWidget : public KisBrushBasedPaintopOptionWidget
{
    Q_OBJECT

public:
     KisColorSmudgeOpSettingsWidget(QWidget* parent = 0);
    ~KisColorSmudgeOpSettingsWidget();

    KisPropertiesConfiguration* configuration() const;
};



#endif // KIS_COLORSMUDGEOP_SETTINGS_WIDGET_H_
