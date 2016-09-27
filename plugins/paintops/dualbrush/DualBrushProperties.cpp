/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
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

#include "DualBrushProperties.h"

#include <kis_resource_server_provider.h>
#include <KoCompositeOpRegistry.h>
#include <kis_properties_configuration.h>

void DualBrushProperties::writeOptionSetting(KisPropertiesConfiguration *setting) const
{
    setting->setProperty("dualbrush/preset_count", presetStack.count());
    for(int i = 0; i < presetStack.count(); ++i) {
        setting->setProperty(QString("dualbrush/preset_%1_compositeop").arg(i), presetStack[i].compositeOp);
        setting->setProperty(QString("dualbrush/preset_%1_fuzziness").arg(i), presetStack[i].fuzziness);
        setting->setProperty(QString("dualbrush/preset_%1_horizontal_offset").arg(i), presetStack[i].horizontalOffset);
        setting->setProperty(QString("dualbrush/preset_%1_name").arg(i), presetStack[i].presetName);
        setting->setProperty(QString("dualbrush/preset_%1_opacity").arg(i), presetStack[i].opacity);
        setting->setProperty(QString("dualbrush/preset_%1_spacing").arg(i), presetStack[i].spacing);
        setting->setProperty(QString("dualbrush/preset_%1_vertical_offset").arg(i), presetStack[i].verticalOffset);
    }
}

void DualBrushProperties::readOptionSetting(const KisPropertiesConfiguration *setting)
{
    presetStack.clear();

    int count = setting->getInt("dualbrush/preset_count");
    for (int i = 0; i < count; ++i) {
        StackedPreset ps;
        ps.compositeOp = setting->getString(QString("dualbrush/preset_%1_compositeop").arg(i), COMPOSITE_OVER);
        ps.fuzziness = setting->getDouble(QString("dualbrush/preset_%1_fuzziness").arg(i));
        ps.horizontalOffset = setting->getDouble(QString("dualbrush/preset_%1_horizontal_offset").arg(i));
        ps.opacity = setting->getDouble(QString("dualbrush/preset_%1_opacity").arg(i), 1.0);
        ps.spacing = setting->getDouble(QString("dualbrush/preset_%1_spacing").arg(i), 0.1);
        ps.presetName = setting->getString(QString("dualbrush/preset_%1_name").arg(i));
        KisPaintOpPresetResourceServer* server = KisResourceServerProvider::instance()->paintOpPresetServer();
        ps.paintopPreset = server->resourceByName(ps.presetName);
        ps.verticalOffset = setting->getDouble(QString("dualbrush/preset_%1_vertical_offset").arg(i));
        presetStack << ps;
    }
}
