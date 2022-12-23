/*
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_SKETCHOP_OPTION_WIDGET_H
#define KIS_SKETCHOP_OPTION_WIDGET_H

#include <kis_paintop_option.h>
#include <KisSketchOpOptionData.h>
#include <lager/cursor.hpp>

struct KisSketchOpOptionData;

class KisSketchOpOptionWidget : public KisPaintOpOption
{
public:
    using data_type = KisSketchOpOptionData;

    KisSketchOpOptionWidget(lager::cursor<KisSketchOpOptionData> optionData);
    ~KisSketchOpOptionWidget();

    void writeOptionSetting(KisPropertiesConfigurationSP setting) const override;
    void readOptionSetting(const KisPropertiesConfigurationSP setting) override;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // KIS_SKETCHOP_OPTION_WIDGET_H
