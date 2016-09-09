/*
 * Copyright (c) 2009,2010 Lukáš Tvrdý (lukast.dev@gmail.com)
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

#include <kis_paint_action_type_option.h>

#include "kis_grid_paintop_settings.h"
#include "kis_grid_paintop_settings_widget.h"

#include "kis_gridop_option.h"
#include "kis_grid_shape_option.h"
#include <kis_color_option.h>

struct KisGridPaintOpSettings::Private
{
    QList<KisUniformPaintOpPropertyWSP> uniformProperties;
};

KisGridPaintOpSettings::KisGridPaintOpSettings()
    : KisOutlineGenerationPolicy<KisPaintOpSettings>(KisCurrentOutlineFetcher::NO_OPTION),
    m_d(new Private)
{
}

KisGridPaintOpSettings::~KisGridPaintOpSettings()
{
}

bool KisGridPaintOpSettings::paintIncremental()
{
    return (enumPaintActionType)getInt("PaintOpAction", WASH) == BUILDUP;
}

QPainterPath KisGridPaintOpSettings::brushOutline(const KisPaintInformation &info, OutlineMode mode)
{
    QPainterPath path;
    if (mode == CursorIsOutline || mode == CursorIsCircleOutline || mode == CursorTiltOutline) {
        qreal sizex = getInt(GRID_WIDTH) * getDouble(GRID_SCALE);
        qreal sizey = getInt(GRID_HEIGHT) * getDouble(GRID_SCALE);
        QRectF rc(0, 0, sizex, sizey);
        rc.translate(-rc.center());
        path.addRect(rc);

        QPainterPath tiltLine;
        QLineF tiltAngle(QPointF(0.0,0.0), QPointF(0.0,sizex));
        tiltAngle.setLength(qMax(sizex*0.5, 50.0) * (1 - info.tiltElevation(info, 60.0, 60.0, true)));
        tiltAngle.setAngle((360.0 - fmod(KisPaintInformation::tiltDirection(info, true) * 360.0 + 270.0, 360.0))-3.0);
        tiltLine.moveTo(tiltAngle.p1());
        tiltLine.lineTo(tiltAngle.p2());
        tiltAngle.setAngle((360.0 - fmod(KisPaintInformation::tiltDirection(info, true) * 360.0 + 270.0, 360.0))+3.0);
        tiltLine.lineTo(tiltAngle.p2());
        tiltLine.lineTo(tiltAngle.p1());

        path = outlineFetcher()->fetchOutline(info, this, path);

        if (mode == CursorTiltOutline) {
            QPainterPath tiltLine = makeTiltIndicator(info, QPointF(0.0, 0.0), sizex * 0.5, 3.0);
            path.addPath(outlineFetcher()->fetchOutline(info, this, tiltLine, 1.0, 0.0, true, 0, 0));
        }
    }
    return path;
}


#include <brushengine/kis_slider_based_paintop_property.h>
#include "kis_paintop_preset.h"
#include "kis_paintop_settings_update_proxy.h"
#include "kis_gridop_option.h"


QList<KisUniformPaintOpPropertySP> KisGridPaintOpSettings::uniformProperties()
{
    QList<KisUniformPaintOpPropertySP> props =
        listWeakToStrong(m_d->uniformProperties);

    if (props.isEmpty()) {
        {
            KisIntSliderBasedPaintOpPropertyCallback *prop =
                new KisIntSliderBasedPaintOpPropertyCallback(
                    KisIntSliderBasedPaintOpPropertyCallback::Int,
                    "grid_divisionlevel",
                    i18n("Division Level"),
                    this, 0);

            prop->setRange(1, 25);
            prop->setSingleStep(1);

            prop->setReadCallback(
                [](KisUniformPaintOpProperty *prop) {
                    GridOption option;
                    option.readOptionSetting(prop->settings().data());

                    prop->setValue(int(option.grid_division_level));
                });
            prop->setWriteCallback(
                [](KisUniformPaintOpProperty *prop) {
                    GridOption option;
                    option.readOptionSetting(prop->settings().data());
                    option.grid_division_level = prop->value().toInt();
                    option.writeOptionSetting(prop->settings().data());
                });

            QObject::connect(preset()->updateProxy(), SIGNAL(sigSettingsChanged()), prop, SLOT(requestReadValue()));
            prop->requestReadValue();
            props << toQShared(prop);
        }
    }

    return KisPaintOpSettings::uniformProperties() + props;
}
