/*
 *  Copyright (c) 2008 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef KIS_PAINTOP_SETTINGS_WIDGET_H_
#define KIS_PAINTOP_SETTINGS_WIDGET_H_

#include "krita_export.h"

#include "kis_config_widget.h"
#include "kis_image.h"
#include <kdebug.h>

/**
 * Base class for widgets that are used to edit and display paintop settings.
 */
class KRITAIMAGE_EXPORT KisPaintOpSettingsWidget : public KisConfigWidget
{

public:

    KisPaintOpSettingsWidget(QWidget * parent = 0, Qt::WFlags f = 0)
            : KisConfigWidget(parent, f) {
    }

    virtual ~KisPaintOpSettingsWidget() {
    }

    /**
     * Write the settings in this widget to the given properties
     * configuration, which is cleared first.
     */
    virtual void writeConfiguration(KisPropertiesConfiguration *config) const = 0;

    virtual void setImage(KisImageWSP image) {
        m_image = image;
    }
    
    virtual void changePaintOpSize(qreal x, qreal y) {
        Q_UNUSED(x);
        Q_UNUSED(y);
    }

protected:


    KisImageWSP m_image;

};

#endif
