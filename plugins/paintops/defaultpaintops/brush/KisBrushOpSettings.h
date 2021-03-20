/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISBRUSHOPSETTINGS_H
#define KISBRUSHOPSETTINGS_H

#include "kis_brush_based_paintop_settings.h"


class KisBrushOpSettings : public KisBrushBasedPaintOpSettings
{
public:
    KisBrushOpSettings(KisResourcesInterfaceSP resourcesInterface);
    bool needsAsynchronousUpdates() const;
};

#endif // KISBRUSHOPSETTINGS_H
