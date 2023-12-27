/*
 *  SPDX-FileCopyrightText: 2022 Agata Cacko <cacko.azh@gmail.com>
 *  SPDX-FileCopyrightText: 2011 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_CURVE_OP_OPTION_WIDGET_H
#define KIS_CURVE_OP_OPTION_WIDGET_H

#include <kis_paintop_option.h>
#include <KisCurveOpOptionData.h>
#include <lager/cursor.hpp>

struct KisCurveOpOptionData;

class KisCurveOpOptionWidget : public KisPaintOpOption
{
public:
    using data_type = KisCurveOpOptionData;

    KisCurveOpOptionWidget(lager::cursor<KisCurveOpOptionData> optionData);
    ~KisCurveOpOptionWidget();

    void writeOptionSetting(KisPropertiesConfigurationSP setting) const override;
    void readOptionSetting(const KisPropertiesConfigurationSP setting) override;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // KIS_EXPERIMENT_OP_OPTION_WIDGET_H
