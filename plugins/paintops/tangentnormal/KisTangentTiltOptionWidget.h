/*
 * SPDX-FileCopyrightText: 2015 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef KIS_TANGENTTILT_OPTION_WIDGET_H
#define KIS_TANGENTTILT_OPTION_WIDGET_H

#include <kis_paintop_option.h>
#include <KisTangentTiltOptionData.h>
#include <lager/cursor.hpp>

struct KisTangentTiltOptionData;

class KisTangentTiltOptionWidget : public KisPaintOpOption
{
public:
    using data_type = KisTangentTiltOptionData;

    KisTangentTiltOptionWidget(lager::cursor<KisTangentTiltOptionData> optionData);
    ~KisTangentTiltOptionWidget();

    void writeOptionSetting(KisPropertiesConfigurationSP setting) const override;
    void readOptionSetting(const KisPropertiesConfigurationSP setting) override;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // KIS_TANGENTTILT_OPTION_WIDGET_H
