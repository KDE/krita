/*
 *  SPDX-FileCopyrightText: 2008 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2008-2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <QPainter>

#include "kis_image.h"

#include "kis_hairy_paintop_settings.h"
#include "kis_hairy_bristle_option.h"
#include "kis_brush_based_paintop_options_widget.h"
#include "kis_boundary.h"

KisHairyPaintOpSettings::KisHairyPaintOpSettings(KisResourcesInterfaceSP resourcesInterface)
    : KisBrushBasedPaintOpSettings(resourcesInterface)
{
}

QPainterPath KisHairyPaintOpSettings::brushOutline(const KisPaintInformation &info, const OutlineMode &mode, qreal alignForZoom)
{
    return brushOutlineImpl(info, mode, alignForZoom, getDouble(HAIRY_BRISTLE_SCALE));
}

bool KisHairyPaintOpSettings::hasPatternSettings() const
{
    return false;
}
