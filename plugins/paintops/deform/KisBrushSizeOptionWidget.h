/*
 *  SPDX-FileCopyrightText: 2009, 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_BRUSHSIZE_OPTION_WIDGET_H
#define KIS_BRUSHSIZE_OPTION_WIDGET_H

#include <kis_paintop_option.h>
#include <KisBrushSizeOptionData.h>
#include <lager/cursor.hpp>

struct KisBrushSizeOptionData;

class KisBrushSizeOptionWidget : public KisPaintOpOption
{
public:
    using data_type = KisBrushSizeOptionData;

    KisBrushSizeOptionWidget(lager::cursor<KisBrushSizeOptionData> optionData);
    ~KisBrushSizeOptionWidget();

    void writeOptionSetting(KisPropertiesConfigurationSP setting) const override;
    void readOptionSetting(const KisPropertiesConfigurationSP setting) override;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // KIS_BRUSHSIZE_OPTION_WIDGET_H
