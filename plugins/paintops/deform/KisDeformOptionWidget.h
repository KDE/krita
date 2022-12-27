/*
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_DEFORM_OPTION_WIDGET_H
#define KIS_DEFORM_OPTION_WIDGET_H

#include <kis_paintop_option.h>
#include <KisDeformOptionData.h>
#include <lager/cursor.hpp>

struct KisDeformOptionData;

class KisDeformOptionWidget : public KisPaintOpOption
{
public:
    using data_type = KisDeformOptionData;

    KisDeformOptionWidget(lager::cursor<KisDeformOptionData> optionData);
    ~KisDeformOptionWidget();

    void writeOptionSetting(KisPropertiesConfigurationSP setting) const override;
    void readOptionSetting(const KisPropertiesConfigurationSP setting) override;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // KIS_DEFORM_OPTION_WIDGET_H
