/*
 *  SPDX-FileCopyrightText: 2008 Lukas Tvrdy <lukast.dev@gmail.com>
 *  SPDX-FileCopyrightText: 2010 José Luis Vergara <pentalis@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_HATCHING_PREFERENCES_WIDGET_H
#define KIS_HATCHING_PREFERENCES_WIDGET_H

#include <kis_paintop_option.h>
#include <KisHatchingPreferencesData.h>
#include <lager/cursor.hpp>

struct KisHatchingPreferencesData;

class KisHatchingPreferencesWidget : public KisPaintOpOption
{
public:
    using data_type = KisHatchingPreferencesData;

    KisHatchingPreferencesWidget(lager::cursor<KisHatchingPreferencesData> optionData);
    ~KisHatchingPreferencesWidget();

    void writeOptionSetting(KisPropertiesConfigurationSP setting) const override;
    void readOptionSetting(const KisPropertiesConfigurationSP setting) override;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // KIS_HATCHING_PREFERENCES_WIDGET_H
