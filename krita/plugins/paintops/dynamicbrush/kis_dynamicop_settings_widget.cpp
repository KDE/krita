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

#include "kis_dynamicop_settings_widget.h"
#include "kis_dynamicop_settings.h"
#include "kis_dynamicop_factory.h"

#include <kis_paintop_registry.h>

KisDynamicOpSettingsWidget::KisDynamicOpSettingsWidget(QWidget* parent)
        : KisPaintOpSettingsWidget(parent)
{
    m_uiOptions = new Ui::DynamicBrushOptions();
    m_uiOptions->setupUi(this);
    m_uiOptions->comboBoxShapes->removeItem(1);
}

KisDynamicOpSettingsWidget::~KisDynamicOpSettingsWidget()
{
}

void KisDynamicOpSettingsWidget::setConfiguration(const KisPropertiesConfiguration * config)
{

}

KisPropertiesConfiguration* KisDynamicOpSettingsWidget::configuration() const
{
    KisPaintOpRegistry* reg = KisPaintOpRegistry::instance();
    KisDynamicOpFactory* factory = dynamic_cast<KisDynamicOpFactory*>(reg->get("dynamicbrush"));
    if (!factory) return 0;

    KisDynamicOpSettings *config =
        new KisDynamicOpSettings(const_cast<KisDynamicOpSettingsWidget*>(this),
                                 factory->shapeBookmarksManager(),
                                 factory->coloringBookmarksManager());
    return config;
}

void KisDynamicOpSettingsWidget::writeConfiguration(KisPropertiesConfiguration *config) const
{
    config->setProperty("paintop", "dynamicbrush"); // XXX: make this a const id string
}

