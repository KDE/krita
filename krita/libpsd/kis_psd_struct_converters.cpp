/*
 *  Copyright (c) 2014 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_psd_struct_converters.h"

#include <filter/kis_filter_configuration.h>

#include "psd.h"


namespace KisPSD {

void contextToConfig(const psd_layer_effects_context *context,
                     KisFilterConfiguration *config)
{
    config->setProperty("context/global_angle", context->global_angle);
    config->setProperty("context/keep_original", context->keep_original);
}

void configToContext(const KisFilterConfiguration *config,
                     psd_layer_effects_context *context)
{
    context->global_angle = config->getPropertyLazy("context/global_angle", 120);
    context->keep_original = config->getPropertyLazy("context/keep_original", true);
}

void dropShadowToConfig(const psd_layer_effects_drop_shadow *effect,
                        KisFilterConfiguration *config)
{
    config->setProperty("drop_shadow/effect_enable", effect->effect_enable);

    config->setProperty("drop_shadow/blend_mode", effect->blend_mode);
    config->setProperty("drop_shadow/color", effect->color);
    config->setProperty("drop_shadow/native_color", effect->native_color);
    config->setProperty("drop_shadow/opacity", effect->opacity);
    config->setProperty("drop_shadow/angle", effect->angle);
    config->setProperty("drop_shadow/use_global_light", effect->use_global_light);
    config->setProperty("drop_shadow/distance", effect->distance);
    config->setProperty("drop_shadow/spread", effect->spread);
    config->setProperty("drop_shadow/size", effect->size);

    for (int i = 0; i < PSD_LOOKUP_TABLE_SIZE; i++) {
        QString property = QString("drop_shadow/contour_lookup_table/value_%1").arg(i);
        config->setProperty(property, effect->contour_lookup_table[i]);
    }

    config->setProperty("drop_shadow/anti_aliased", effect->anti_aliased);
    config->setProperty("drop_shadow/noise", effect->noise);
    config->setProperty("drop_shadow/knocks_out", effect->knocks_out);
}

void configToDropShadow(const KisFilterConfiguration *config,
                        psd_layer_effects_drop_shadow *effect)
{
    effect->effect_enable = config->getPropertyLazy("drop_shadow/effect_enable", false);

    effect->blend_mode = config->getPropertyLazy("drop_shadow/blend_mode", COMPOSITE_MULT);
    effect->color = config->getPropertyLazy("drop_shadow/color", QColor(Qt::black));
    effect->native_color = config->getPropertyLazy("drop_shadow/native_color", QColor(Qt::black));
    effect->opacity = config->getPropertyLazy("drop_shadow/opacity", 75);
    effect->angle = config->getPropertyLazy("drop_shadow/angle", 120);
    effect->use_global_light = config->getPropertyLazy("drop_shadow/use_global_light", true);
    effect->distance = config->getPropertyLazy("drop_shadow/distance", 21);
    effect->spread = config->getPropertyLazy("drop_shadow/spread", 0);
    effect->size = config->getPropertyLazy("drop_shadow/size", 21);

    for (int i = 0; i < PSD_LOOKUP_TABLE_SIZE; i++) {
        QString property = QString("drop_shadow/contour_lookup_table/value_%1").arg(i);
        effect->contour_lookup_table[i] = config->getPropertyLazy(property, i);
    }

    effect->anti_aliased = config->getPropertyLazy("drop_shadow/anti_aliased", false);
    effect->noise = config->getPropertyLazy("drop_shadow/noise", 0);
    effect->knocks_out = config->getPropertyLazy("drop_shadow/knocks_out", true);
}

}
