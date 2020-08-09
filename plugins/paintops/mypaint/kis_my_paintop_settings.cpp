/*
 * Copyright (c) 2020 Ashwin Dhakaita <ashwingpdhakaita@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include <cmath>

#include <kis_color_option.h>
#include <kis_paint_action_type_option.h>

#include "kis_my_paintop_settings.h"
#include "kis_my_paintop_option.h"

struct KisMyPaintOpSettings::Private
{
};


KisMyPaintOpSettings::KisMyPaintOpSettings(KisResourcesInterfaceSP resourcesInterface)
    : KisOutlineGenerationPolicy<KisPaintOpSettings>(KisCurrentOutlineFetcher::SIZE_OPTION |
                                                     KisCurrentOutlineFetcher::ROTATION_OPTION, resourcesInterface),
    m_d(new Private)
{
}

KisMyPaintOpSettings::~KisMyPaintOpSettings()
{
}

void KisMyPaintOpSettings::setPaintOpSize(qreal value)
{    
    KisMyPaintOptionProperties op;
    op.readOptionSettingImpl(this);
    op.diameter = value;
    op.writeOptionSettingImpl(this);
}

qreal KisMyPaintOpSettings::paintOpSize() const
{
    KisMyPaintOptionProperties op;
    op.readOptionSettingImpl(this);
    return op.diameter;
}

bool KisMyPaintOpSettings::paintIncremental()
{
    return false;
}


QPainterPath KisMyPaintOpSettings::brushOutline(const KisPaintInformation &info, const OutlineMode &mode, qreal alignForZoom)
{    
    QPainterPath path;

    if (mode.isVisible) {
        qreal finalScale = 1.0;

        KisMyPaintOptionProperties op;
        op.readOptionSettingImpl(this);
        qreal radius = 0.5 * op.diameter;
        radius = radius > 3.5 ? radius : 3.5;

        QPainterPath realOutline;
        realOutline.addEllipse(QPointF(), radius, radius);

        path = outlineFetcher()->fetchOutline(info, this, realOutline, mode, alignForZoom, finalScale);

        if (mode.showTiltDecoration) {
            QPainterPath tiltLine = makeTiltIndicator(info,
                realOutline.boundingRect().center(),
                realOutline.boundingRect().width() * 0.5,
                3.0);
            path.addPath(outlineFetcher()->fetchOutline(info, this, tiltLine, mode, alignForZoom, finalScale, 0.0, true, realOutline.boundingRect().center().x(), realOutline.boundingRect().center().y()));
        }
    }

    return path;
}
