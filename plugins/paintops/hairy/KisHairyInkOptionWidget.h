/*
 *  SPDX-FileCopyrightText: 2008-2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_HAIRYINK_OPTION_WIDGET_H
#define KIS_HAIRYINK_OPTION_WIDGET_H

#include <kis_paintop_option.h>
#include <KisHairyInkOptionData.h>
#include <lager/cursor.hpp>

struct KisHairyInkOptionData;

class KisHairyInkOptionWidget : public KisPaintOpOption
{
public:
    using data_type = KisHairyInkOptionData;

    KisHairyInkOptionWidget(lager::cursor<KisHairyInkOptionData> optionData);
    ~KisHairyInkOptionWidget();

    void writeOptionSetting(KisPropertiesConfigurationSP setting) const override;
    void readOptionSetting(const KisPropertiesConfigurationSP setting) override;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // KIS_HAIRYINK_OPTION_WIDGET_H
