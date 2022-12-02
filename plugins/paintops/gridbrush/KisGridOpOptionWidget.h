/*
 *  SPDX-FileCopyrightText: 2022 Agata Cacko <cacko.azh@gmail.com>
 *  SPDX-FileCopyrightText: 2009, 2010 Lukáš Tvrdý (lukast.dev@gmail.com)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_GRID_OP_OPTION_WIDGET_H
#define KIS_GRID_OP_OPTION_WIDGET_H

#include <kis_paintop_option.h>
#include <KisGridOpOptionData.h>
#include <lager/cursor.hpp>

class KisCurveWidget;

struct KisGridOpOptionData;

class KisGridOpOptionWidget : public KisPaintOpOption
{
public:
    using data_type = KisGridOpOptionData;

    KisGridOpOptionWidget(lager::cursor<KisGridOpOptionData> optionData);
    ~KisGridOpOptionWidget();

    void writeOptionSetting(KisPropertiesConfigurationSP setting) const override;
    void readOptionSetting(const KisPropertiesConfigurationSP setting) override;
    

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // KIS_GRID_OP_OPTION_WIDGET_H
