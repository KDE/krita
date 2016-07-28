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

#ifndef __KIS_STANDARD_UNIFORM_PROPERTIES_FACTORY_H
#define __KIS_STANDARD_UNIFORM_PROPERTIES_FACTORY_H

#include <KoID.h>

#include "kis_uniform_paintop_property.h"

class KisPaintopSettingsUpdateProxy;

namespace KisStandardUniformPropertiesFactory
{
    static const KoID size("size", i18n("Size"));
    static const KoID opacity("opacity", i18n("Opacity"));
    static const KoID flow("flow", i18n("Flow"));
    static const KoID angle("angle", i18n("Angle"));
    static const KoID spacing("spacing", i18n("Spacing"));

    /**
     * Overload of createProperty(const QString &id, ...)
     */
    KisUniformPaintOpPropertySP createProperty(const KoID &id,
                                               KisPaintOpSettingsSP settings,
                                               KisPaintopSettingsUpdateProxy *updateProxy);

    /**
     * Factory for creating standard uniform properties. Right now
     * it supports only size, opacity and flow.
     */
    KisUniformPaintOpPropertySP createProperty(const QString &id,
                                               KisPaintOpSettingsSP settings,
                                               KisPaintopSettingsUpdateProxy *updateProxy);
}

#endif /* __KIS_STANDARD_UNIFORM_PROPERTIES_FACTORY_H */
