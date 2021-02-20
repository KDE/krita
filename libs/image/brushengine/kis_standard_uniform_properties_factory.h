/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_STANDARD_UNIFORM_PROPERTIES_FACTORY_H
#define __KIS_STANDARD_UNIFORM_PROPERTIES_FACTORY_H

#include <KoID.h>

#include "kis_uniform_paintop_property.h"

class KisPaintopSettingsUpdateProxy;

namespace KisStandardUniformPropertiesFactory
{
static const KoID size("size", ki18n("Size"));
static const KoID opacity("opacity", ki18n("Opacity"));
static const KoID flow("flow", ki18n("Flow"));
static const KoID angle("angle", ki18n("Angle"));
static const KoID spacing("spacing", ki18n("Spacing"));


/**
     * Overload of createProperty(const QString &id, ...)
     */
KisUniformPaintOpPropertySP createProperty(const KoID &id,
                                           KisPaintOpSettingsRestrictedSP settings,
                                           KisPaintopSettingsUpdateProxy *updateProxy);

/**
     * Factory for creating standard uniform properties. Right now
     * it supports only size, opacity and flow.
     */
KisUniformPaintOpPropertySP createProperty(const QString &id,
                                           KisPaintOpSettingsRestrictedSP settings,
                                           KisPaintopSettingsUpdateProxy *updateProxy);
}

#endif /* __KIS_STANDARD_UNIFORM_PROPERTIES_FACTORY_H */
