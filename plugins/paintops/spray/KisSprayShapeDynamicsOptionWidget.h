/*
 *  SPDX-FileCopyrightText: 2022 Agata Cacko <cacko.azh@gmail.com>
 *  SPDX-FileCopyrightText: 2008, 2009, 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_SPRAY_SHAPE_DYNAMICS_OPTION_WIDGET_H
#define KIS_SPRAY_SHAPE_DYNAMICS_OPTION_WIDGET_H

#include <kis_paintop_option.h>
#include <KisSprayShapeDynamicsOptionData.h>
#include <lager/cursor.hpp>

struct KisSprayShapeDynamicsOptionData;

class KisSprayShapeDynamicsOptionWidget : public KisPaintOpOption
{
public:
    using data_type = KisSprayShapeDynamicsOptionData;

    KisSprayShapeDynamicsOptionWidget(lager::cursor<KisSprayShapeDynamicsOptionData> optionData);
    ~KisSprayShapeDynamicsOptionWidget();

    void writeOptionSetting(KisPropertiesConfigurationSP setting) const override;
    void readOptionSetting(const KisPropertiesConfigurationSP setting) override;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // KIS_SPRAY_SHAPE_DYNAMICS_OPTION_WIDGET_H
