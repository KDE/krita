/*
 *  Copyright (c) 2008 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2008 Lukáš Tvrdý <lukast.dev@gmail.com>
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
#include "kis_sumi_paintop_settings_widget.h"
#include "kis_sumi_paintop_settings.h"

#include "kis_sumi_shape_option.h"
#include "kis_sumi_ink_option.h"

#include <kis_paintop_options_widget.h>

KisSumiPaintOpSettingsWidget:: KisSumiPaintOpSettingsWidget(QWidget* parent)
        : KisPaintOpOptionsWidget(parent)
{
    m_sumiShapeOption = new KisSumiShapeOption();
    m_sumiInkOption = new KisSumiInkOption();
    addPaintOpOption(m_sumiShapeOption);
    addPaintOpOption(m_sumiInkOption);
}

KisSumiPaintOpSettingsWidget::~ KisSumiPaintOpSettingsWidget()
{
    delete m_sumiShapeOption;
    delete m_sumiInkOption;
}


KisPropertiesConfiguration*  KisSumiPaintOpSettingsWidget::configuration() const
{
    KisSumiPaintOpSettings* config = new KisSumiPaintOpSettings();
    config->setOptionsWidget(const_cast<KisSumiPaintOpSettingsWidget*>(this));
    config->setProperty("paintop", "sumibrush"); // XXX: make this a const id string
    writeConfiguration(config);
    return config;
}

void KisSumiPaintOpSettingsWidget::changePaintOpSize(qreal x, qreal y)
{
    // if the movement is more left<->right then up<->down
    if (qAbs(x) > qAbs(y)){
        m_sumiShapeOption->setRadius( m_sumiShapeOption->radius() + qRound(x) );
    }
    else 
    {
    }
}
