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
#include "KisDuplicateOptionData.h"
#include "kis_duplicateop_settings.h"

#include <KisCompositeOpOptionWidget.h>
#include <KisDuplicateOptionWidget.h>
#include <KisMirrorOptionWidget.h>
#include <KisPaintOpOptionWidgetUtils.h>
#include <KisSizeOptionWidget.h>
#include <KisStandardOptionData.h>
#include <KisTextureOptionWidget.h>
#include <brushengine/kis_paintop_lod_limitations.h>
#include <kis_image.h>
#include <kis_paintop_settings_widget.h>
#include <kis_properties_configuration.h>

KisDuplicateOpSettingsWidget::KisDuplicateOpSettingsWidget(QWidget* parent, KisResourcesInterfaceSP resourcesInterface, KoCanvasResourcesInterfaceSP canvasResourcesInterface)
    : KisBrushBasedPaintopOptionWidget(KisBrushOptionWidgetFlag::SupportsPrecision,
                                       parent)
{
    Q_UNUSED(canvasResourcesInterface)
    namespace kpowu = KisPaintOpOptionWidgetUtils;

    setObjectName("brush option widget");

    addPaintOpOption(kpowu::createOptionWidget<KisCompositeOpOptionWidget>());
    addPaintOpOption(kpowu::createOpacityOptionWidget());
    addPaintOpOption(kpowu::createOptionWidget<KisSizeOptionWidget>());
    addPaintOpOption(kpowu::createRotationOptionWidget());
    addPaintOpOption(kpowu::createOptionWidget<KisMirrorOptionWidget>());
    addPaintOpOption(kpowu::createOptionWidget<KisDuplicateOptionWidget>());
    addPaintOpOption(kpowu::createOptionWidget<KisTextureOptionWidget>(KisTextureOptionData(), resourcesInterface));
    addPaintOpOption(kpowu::createCurveOptionWidget(KisStrengthOptionData(),
                                                    KisPaintOpOption::TEXTURE,
                                                    i18n("Weak"),
                                                    i18n("Strong")));
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
