/*
 *  SPDX-FileCopyrightText: 2010 José Luis Vergara <pentalis@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_HATCHING_PREFERENCES_H
#define KIS_HATCHING_PREFERENCES_H

#include <kis_paintop_option.h>

class KisHatchingPreferencesWidget;

class KisHatchingPreferences : public KisPaintOpOption
{

public:
    KisHatchingPreferences();
    ~KisHatchingPreferences() override;

    void writeOptionSetting(KisPropertiesConfigurationSP setting) const override;
    void readOptionSetting(const KisPropertiesConfigurationSP setting) override;

private:
    KisHatchingPreferencesWidget * m_options;

};

#endif
