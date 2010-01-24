/*
 *  Copyright (c) 2009-2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#ifndef KIS_DYNAPAINTOP_SETTINGS_WIDGET_H_
#define KIS_DYNAPAINTOP_SETTINGS_WIDGET_H_

#include <kis_paintop_options_widget.h>

class KisPaintActionTypeOption;
class KisDynaOpOption;

class KisDynaPaintOpSettingsWidget : public KisPaintOpOptionsWidget
{
    Q_OBJECT

public:
    KisDynaPaintOpSettingsWidget(QWidget* parent = 0);
    virtual ~KisDynaPaintOpSettingsWidget();

    KisPropertiesConfiguration* configuration() const;

public:
    KisPaintActionTypeOption* m_paintActionTypeOption;
    KisDynaOpOption* m_dynaOption;

};

#endif
