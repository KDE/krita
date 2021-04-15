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
    m_d(new Private),
    m_modifyOffsetWithShortcut(false)
{
}

void KisGridPaintOpSettings::setPaintOpSize(qreal value)
{
    KisGridOpProperties option;
    option.readOptionSetting(this);
    option.diameter = value;
    option.writeOptionSetting(this);
}

qreal KisGridPaintOpSettings::paintOpSize() const
{
    KisGridOpProperties option;
    option.readOptionSetting(this);
    return option.diameter;
}

KisGridPaintOpSettings::~KisGridPaintOpSettings()
{
}

bool KisGridPaintOpSettings::paintIncremental()
{
    return (enumPaintActionType)getInt("PaintOpAction", WASH) == BUILDUP;
}

bool KisGridPaintOpSettings::mousePressEvent(const KisPaintInformation& info, Qt::KeyboardModifiers modifiers, KisNodeWSP currentNode)
{
    KisGridOpProperties option;
    option.readOptionSetting(this);
    bool ignoreEvent = false;
    qreal newHorizontalOffset = std::fmod(info.pos().x() + option.grid_width/2.0, (float)option.grid_width);
    qreal newVerticalOffset = std::fmod(info.pos().y() + option.grid_height/2.0, (float)option.grid_height);

    // If pressing ctrl+alt change the offset according to mouse position
    if (modifiers & (Qt::ControlModifier + Qt::AltModifier) || m_modifyOffsetWithShortcut) {
        m_modifyOffsetWithShortcut = true;
        newHorizontalOffset = (newHorizontalOffset / (float)option.grid_width);
        newVerticalOffset = (newVerticalOffset / (float)option.grid_height);

        if (newHorizontalOffset > 0.5) {
            newHorizontalOffset = newHorizontalOffset - 1;
        }
        if (newVerticalOffset > 0.5) {
            newVerticalOffset = newVerticalOffset -1;
        }
        option.horizontal_offset = newHorizontalOffset * option.grid_width;
        option.vertical_offset = newVerticalOffset * option.grid_height;
        option.writeOptionSetting(this);
        ignoreEvent = true;
    }
    return ignoreEvent;
}

bool KisGridPaintOpSettings::mouseReleaseEvent()
{
    m_modifyOffsetWithShortcut = false;
    bool ignoreEvent = true;
    return ignoreEvent;
}
QPainterPath KisGridPaintOpSettings::brushOutline(const KisPaintInformation &info, const OutlineMode &mode, qreal alignForZoom)
{
    QPainterPath path;
    KisGridOpProperties option;
    option.readOptionSetting(this);
    if (mode.isVisible) {
        qreal sizex = option.diameter * option.grid_scale;
        qreal sizey = option.diameter * option.grid_scale;
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
