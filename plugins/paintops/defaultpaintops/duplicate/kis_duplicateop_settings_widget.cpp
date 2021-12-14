/*
 *  SPDX-FileCopyrightText: 2002 Patrick Julien <freak@codepimps.org>
 *  SPDX-FileCopyrightText: 2004-2008 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2004 Clarence Dang <dang@kde.org>
 *  SPDX-FileCopyrightText: 2004 Adrian Page <adrian@pagenet.plus.com>
 *  SPDX-FileCopyrightText: 2004 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_duplicateop_settings_widget.h"
#include "kis_duplicateop_settings.h"
#include "kis_duplicateop_option.h"

#include <kis_image.h>
#include <kis_properties_configuration.h>
#include <kis_paintop_settings_widget.h>
#include <kis_pressure_size_option.h>
#include <kis_pressure_opacity_option.h>
#include <kis_pressure_rotation_option.h>
#include <kis_curve_option_widget.h>
#include <kis_compositeop_option.h>
#include "kis_texture_option.h"
#include <kis_pressure_mirror_option_widget.h>
#include "kis_pressure_texture_strength_option.h"
#include <brushengine/kis_paintop_lod_limitations.h>

KisDuplicateOpSettingsWidget::KisDuplicateOpSettingsWidget(QWidget* parent)
    : KisBrushBasedPaintopOptionWidget(parent)
{
    setObjectName("brush option widget");
    setPrecisionEnabled(true);

    addPaintOpOption(new KisCompositeOpOption(true));
    addPaintOpOption(new KisCurveOptionWidget(new KisPressureOpacityOption(), i18n("Transparent"), i18n("Opaque")));
    addPaintOpOption(new KisCurveOptionWidget(new KisPressureSizeOption(), i18n("0%"), i18n("100%")));
    addPaintOpOption(new KisCurveOptionWidget(new KisPressureRotationOption(), i18n("-180°"), i18n("180°")));
    addPaintOpOption(new KisPressureMirrorOptionWidget());
    addPaintOpOption(new KisDuplicateOpOption());
    addPaintOpOption(new KisTextureOption());
    addPaintOpOption(new KisCurveOptionWidget(new KisPressureTextureStrengthOption(), i18n("Weak"), i18n("Strong")));
}

KisDuplicateOpSettingsWidget::~KisDuplicateOpSettingsWidget()
{
}

KisPropertiesConfigurationSP KisDuplicateOpSettingsWidget::configuration() const
{
    KisDuplicateOpSettings *config = new KisDuplicateOpSettings(resourcesInterface());
    config->setProperty("paintop", "duplicate"); // XXX: make this a const id string
    writeConfiguration(config);
    return config;
}

KisPaintopLodLimitations KisDuplicateOpSettingsWidget::lodLimitations() const
{
    KisPaintopLodLimitations l = KisBrushBasedPaintopOptionWidget::lodLimitations();
    l.blockers << KoID("clone-brush", i18nc("PaintOp instant preview limitation", "Clone Brush (temporarily disabled)"));
    return l;
}
