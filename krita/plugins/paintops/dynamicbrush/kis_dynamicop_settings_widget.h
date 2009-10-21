/*
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KIS_DYNAMICOP_SETTINGS_WIDGET_H_
#define KIS_DYNAMICOP_SETTINGS_WIDGET_H_

#include <kis_paintop_settings_widget.h>
#include "ui_DynamicBrushOptions.h"

class KisPropertiesConfiguration;


class KisDynamicOpSettingsWidget : public KisPaintOpSettingsWidget
{
public:

    KisDynamicOpSettingsWidget(QWidget* parent = 0);

    virtual ~KisDynamicOpSettingsWidget();

    void setConfiguration(const KisPropertiesConfiguration * config);

    KisPropertiesConfiguration* configuration() const;

    void writeConfiguration(KisPropertiesConfiguration *config) const;

public:

    Ui_DynamicBrushOptions* m_uiOptions;


};

#endif
