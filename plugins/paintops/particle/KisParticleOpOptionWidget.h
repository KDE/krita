/*
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_PARTICLEOP_OPTION_WIDGET_H
#define KIS_PARTICLEOP_OPTION_WIDGET_H

#include <kis_paintop_option.h>
#include <KisParticleOpOptionData.h>
#include <lager/cursor.hpp>

struct KisParticleOpOptionData;

class KisParticleOpOptionWidget : public KisPaintOpOption
{
public:
    using data_type = KisParticleOpOptionData;

    KisParticleOpOptionWidget(lager::cursor<KisParticleOpOptionData> optionData);
    ~KisParticleOpOptionWidget();

    void writeOptionSetting(KisPropertiesConfigurationSP setting) const override;
    void readOptionSetting(const KisPropertiesConfigurationSP setting) override;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // KIS_PARTICLEOP_OPTION_WIDGET_H
