/*
 *  SPDX-FileCopyrightText: 2022 Agata Cacko <cacko.azh@gmail.com>
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_SPRAY_OP_OPTION_WIDGET_H
#define KIS_SPRAY_OP_OPTION_WIDGET_H

#include <kis_paintop_option.h>
#include <KisSprayOpOptionData.h>
#include <lager/cursor.hpp>

class KisCurveWidget;

struct KisSprayOpOptionData;

class KisSprayOpOptionWidget : public KisPaintOpOption
{
public:
    using data_type = KisSprayOpOptionData;

    KisSprayOpOptionWidget(lager::cursor<KisSprayOpOptionData> optionData);
    ~KisSprayOpOptionWidget();

    void writeOptionSetting(KisPropertiesConfigurationSP setting) const override;
    void readOptionSetting(const KisPropertiesConfigurationSP setting) override;
    
    lager::cursor<qreal> scale() const;
    lager::cursor<int> diameter() const;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // KIS_SPRAY_OP_OPTION_WIDGET_H
