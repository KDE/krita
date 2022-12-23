/*
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_HAIRYBRISTLE_OPTION_WIDGET_H
#define KIS_HAIRYBRISTLE_OPTION_WIDGET_H

#include <kis_paintop_option.h>
#include <KisHairyBristleOptionData.h>
#include <lager/cursor.hpp>

struct KisHairyBristleOptionData;

class KisHairyBristleOptionWidget : public KisPaintOpOption
{
public:
    using data_type = KisHairyBristleOptionData;

    KisHairyBristleOptionWidget(lager::cursor<KisHairyBristleOptionData> optionData);
    ~KisHairyBristleOptionWidget();

    void writeOptionSetting(KisPropertiesConfigurationSP setting) const override;
    void readOptionSetting(const KisPropertiesConfigurationSP setting) override;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // KIS_HAIRYBRISTLE_OPTION_WIDGET_H
