/*
 * SPDX-FileCopyrightText: 2009, 2010 Lukáš Tvrdý (lukast.dev@gmail.com)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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

KisGridPaintOpSettings::KisGridPaintOpSettings(KisResourcesInterfaceSP resourcesInterface)
    : KisOutlineGenerationPolicy<KisPaintOpSettings>(KisCurrentOutlineFetcher::NO_OPTION,
                                                     resourcesInterface),
    m_d(new Private)
{
}

void KisGridPaintOpSettings::setPaintOpSize(qreal value)
{
    KisGridOpProperties option;
    option.readOptionSetting(this);
    option.grid_width = value;
    option.grid_height = value;
    option.writeOptionSetting(this);
}

qreal KisGridPaintOpSettings::paintOpSize() const
{
    KisGridOpProperties option;
    option.readOptionSetting(this);

    return option.grid_width;
}

KisGridPaintOpSettings::~KisGridPaintOpSettings()
{
}

bool KisGridPaintOpSettings::paintIncremental()
{
    return (enumPaintActionType)getInt("PaintOpAction", WASH) == BUILDUP;
}

QPainterPath KisGridPaintOpSettings::brushOutline(const KisPaintInformation &info, const OutlineMode &mode, qreal alignForZoom)
{
    QPainterPath path;
    if (mode.isVisible) {
        qreal sizex = getInt(GRID_WIDTH) * getDouble(GRID_SCALE);
        qreal sizey = getInt(GRID_HEIGHT) * getDouble(GRID_SCALE);
        QRectF rc(0, 0, sizex, sizey);
        rc.translate(-rc.center());
        path.addRect(rc);

        path = outlineFetcher()->fetchOutline(info, this, path, mode, alignForZoom);

        if (mode.showTiltDecoration) {
            QPainterPath tiltLine = makeTiltIndicator(info, QPointF(0.0, 0.0), sizex * 0.5, 3.0);
            path.addPath(outlineFetcher()->fetchOutline(info, this, tiltLine, mode, alignForZoom, 1.0, 0.0, true, 0, 0));
        }
    }
    return path;
}


#include <brushengine/kis_slider_based_paintop_property.h>
#include "kis_paintop_preset.h"
#include "kis_paintop_settings_update_proxy.h"

QList<KisUniformPaintOpPropertySP> KisGridPaintOpSettings::uniformProperties(KisPaintOpSettingsSP settings)
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
                    settings, 0);

            prop->setRange(1, 25);
            prop->setSingleStep(1);

            prop->setReadCallback(
                [](KisUniformPaintOpProperty *prop) {
                    KisGridOpProperties option;
                    option.readOptionSetting(prop->settings().data());

                    prop->setValue(int(option.grid_division_level));
                });
            prop->setWriteCallback(
                [](KisUniformPaintOpProperty *prop) {
                    KisGridOpProperties option;
                    option.readOptionSetting(prop->settings().data());
                    option.grid_division_level = prop->value().toInt();
                    option.writeOptionSetting(prop->settings().data());
                });

            QObject::connect(updateProxy(), SIGNAL(sigSettingsChanged()), prop, SLOT(requestReadValue()));
            prop->requestReadValue();
            props << toQShared(prop);
        }
    }

    return KisPaintOpSettings::uniformProperties(settings) + props;
}
