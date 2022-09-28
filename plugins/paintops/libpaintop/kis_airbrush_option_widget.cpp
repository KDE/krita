/*
 *  SPDX-FileCopyrightText: 2008 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "kis_airbrush_option_widget.h"
#include "kis_paintop_settings.h"
#include <klocalizedstring.h>

#include <QWidget>
#include <QRadioButton>

#include "ui_wdgairbrush.h"

const qreal DEFAULT_RATE = 20.0;

void KisAirbrushOptionProperties::readOptionSettingImpl(const KisPropertiesConfiguration *setting){
    enabled = setting->getBool(AIRBRUSH_ENABLED);
    airbrushInterval = 1000.0 / setting->getDouble(AIRBRUSH_RATE, DEFAULT_RATE);
    ignoreSpacing = setting->getBool(AIRBRUSH_IGNORE_SPACING, false);
}

void KisAirbrushOptionProperties::writeOptionSettingImpl(KisPropertiesConfiguration *setting) const {
    setting->setProperty(AIRBRUSH_ENABLED, enabled);
    setting->setProperty(AIRBRUSH_RATE, airbrushInterval > 0 ? 1000.0 / airbrushInterval : 1.0);
    setting->setProperty(AIRBRUSH_IGNORE_SPACING, ignoreSpacing);
}
