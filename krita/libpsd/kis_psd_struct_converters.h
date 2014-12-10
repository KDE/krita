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

#ifndef __KIS_PSD_STRUCT_CONVERTERS_H
#define __KIS_PSD_STRUCT_CONVERTERS_H

#include "libkispsd_export.h"

class KisFilterConfiguration;
struct psd_layer_effects_drop_shadow;
struct psd_layer_effects_context;

namespace KisPSD
{
    void LIBKISPSD_EXPORT contextToConfig(const psd_layer_effects_context *context,
                                          KisFilterConfiguration *config);

    void LIBKISPSD_EXPORT configToContext(const KisFilterConfiguration *config,
                                          psd_layer_effects_context *context);

    void LIBKISPSD_EXPORT dropShadowToConfig(const psd_layer_effects_drop_shadow *effect,
                                             KisFilterConfiguration *config);

    void LIBKISPSD_EXPORT configToDropShadow(const KisFilterConfiguration *config,
                                             psd_layer_effects_drop_shadow *effect);
}

#endif /* __KIS_PSD_STRUCT_CONVERTERS_H */
