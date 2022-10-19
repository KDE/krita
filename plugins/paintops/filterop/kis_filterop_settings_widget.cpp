/*
 *  SPDX-FileCopyrightText: 2002 Patrick Julien <freak@codepimps.org>
 *  SPDX-FileCopyrightText: 2004-2008 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2004 Clarence Dang <dang@kde.org>
 *  SPDX-FileCopyrightText: 2004 Adrian Page <adrian@pagenet.plus.com>
 *  SPDX-FileCopyrightText: 2004 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_filterop_settings_widget.h"
#include "kis_filterop_settings.h"

#include <kis_properties_configuration.h>
#include <filter/kis_filter.h>
#include <KisStandardOptionData.h>
#include <KisPaintOpOptionUtils.h>
#include <KisCompositeOpOptionWidget.h>
#include <KisSizeOptionWidget.h>
#include <KisMirrorOptionWidget.h>
#include <KisFilterOptionWidget.h>

KisFilterOpSettingsWidget::KisFilterOpSettingsWidget(QWidget* parent)
    : KisBrushBasedPaintopOptionWidget(KisBrushOptionWidgetFlag::SupportsPrecision, parent)
{
    namespace kpou = KisPaintOpOptionUtils;


    setObjectName("filter option widget");

    addPaintOpOption(kpou::createOptionWidget<KisCompositeOpOptionWidget>());
    addPaintOpOption(kpou::createOpacityOptionWidget());
    addPaintOpOption(kpou::createOptionWidget<KisSizeOptionWidget>());
    addPaintOpOption(kpou::createRotationOptionWidget());
    addPaintOpOption(kpou::createOptionWidget<KisMirrorOptionWidget>());
    addPaintOpOption(kpou::createOptionWidget<KisFilterOptionWidget2>());
}

KisFilterOpSettingsWidget::~KisFilterOpSettingsWidget()
{
}

KisPropertiesConfigurationSP KisFilterOpSettingsWidget::configuration() const
{
    KisFilterOpSettings *config = new KisFilterOpSettings(resourcesInterface());
    config->setProperty("paintop", "filter"); // XXX: make this a const id string
    writeConfiguration(config);
    return config;
}

