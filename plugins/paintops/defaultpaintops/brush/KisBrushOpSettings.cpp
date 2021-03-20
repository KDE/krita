/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisBrushOpSettings.h"


KisBrushOpSettings::KisBrushOpSettings(KisResourcesInterfaceSP resourcesInterface)
    : KisBrushBasedPaintOpSettings(resourcesInterface)
{
}

bool KisBrushOpSettings::needsAsynchronousUpdates() const
{
    return true;
}
