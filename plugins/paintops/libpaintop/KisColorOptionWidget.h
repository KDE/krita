/*
 *  SPDX-FileCopyrightText: 2022 Agata Cacko <cacko.azh@gmail.com>
 *  SPDX-FileCopyrightText: 2009, 2010 Lukáš Tvrdý (lukast.dev@gmail.com)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_COLOR_OPTION_WIDGET_H
#define KIS_COLOR_OPTION_WIDGET_H

#include <kis_paintop_option.h>
#include <KisColorOptionData.h>
#include <lager/cursor.hpp>

class KisCurveWidget;

struct KisColorOptionData;

class PAINTOP_EXPORT KisColorOptionWidget : public KisPaintOpOption
{
public:
    using data_type = KisColorOptionData;

    KisColorOptionWidget(lager::cursor<KisColorOptionData> optionData);
    ~KisColorOptionWidget();

    void writeOptionSetting(KisPropertiesConfigurationSP setting) const override;
    void readOptionSetting(const KisPropertiesConfigurationSP setting) override;
    

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // KIS_COLOR_OPTION_WIDGET_H
