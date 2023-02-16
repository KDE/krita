/*
 *  SPDX-FileCopyrightText: 2022 Agata Cacko <cacko.azh@gmail.com>
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_EXPERIMENT_OP_OPTION_WIDGET_H
#define KIS_EXPERIMENT_OP_OPTION_WIDGET_H

#include <kis_paintop_option.h>
#include <KisExperimentOpOptionData.h>
#include <lager/cursor.hpp>

struct KisExperimentOpOptionData;

class KisExperimentOpOptionWidget : public KisPaintOpOption
{
public:
    using data_type = KisExperimentOpOptionData;

    KisExperimentOpOptionWidget(lager::cursor<KisExperimentOpOptionData> optionData);
    ~KisExperimentOpOptionWidget();

    void writeOptionSetting(KisPropertiesConfigurationSP setting) const override;
    void readOptionSetting(const KisPropertiesConfigurationSP setting) override;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // KIS_EXPERIMENT_OP_OPTION_WIDGET_H
