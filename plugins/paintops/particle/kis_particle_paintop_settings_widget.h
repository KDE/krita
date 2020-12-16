/*
 *  SPDX-FileCopyrightText: 2008 Lukas Tvrdy <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_PARTICLEPAINTOP_SETTINGS_WIDGET_H_
#define KIS_PARTICLEPAINTOP_SETTINGS_WIDGET_H_

#include <kis_paintop_settings_widget.h>

#include "ui_wdgparticleoptions.h"
#include "KisPopupButton.h"

class KisPaintActionTypeOption;
class KisParticleOpOption;

class KisParticlePaintOpSettingsWidget : public KisPaintOpSettingsWidget
{
    Q_OBJECT

public:
    KisParticlePaintOpSettingsWidget(QWidget* parent = 0);
    ~KisParticlePaintOpSettingsWidget() override;

    KisPropertiesConfigurationSP configuration() const override;

public:
    KisPaintActionTypeOption* m_paintActionTypeOption;
    KisParticleOpOption* m_particleOption;

};

#endif
