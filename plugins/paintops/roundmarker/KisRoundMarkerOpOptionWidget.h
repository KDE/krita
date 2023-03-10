/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_ROUNDMARKER_OP_OPTION_WIDGET_H
#define KIS_ROUNDMARKER_OP_OPTION_WIDGET_H

#include <kis_paintop_option.h>
#include <KisRoundMarkerOpOptionData.h>
#include <lager/cursor.hpp>

struct KisRoundMarkerOpOptionData;

class KisRoundMarkerOpOptionWidget : public KisPaintOpOption
{
public:
    using data_type = KisRoundMarkerOpOptionData;

    KisRoundMarkerOpOptionWidget(lager::cursor<KisRoundMarkerOpOptionData> optionData);
    ~KisRoundMarkerOpOptionWidget();

    void writeOptionSetting(KisPropertiesConfigurationSP setting) const override;
    void readOptionSetting(const KisPropertiesConfigurationSP setting) override;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // KIS_ROUNDMARKER_OP_OPTION_WIDGET_H
