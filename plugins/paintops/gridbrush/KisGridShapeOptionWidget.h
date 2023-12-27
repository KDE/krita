/*
 *  SPDX-FileCopyrightText: 2022 Agata Cacko <cacko.azh@gmail.com>
 *  SPDX-FileCopyrightText: 2009, 2010 Lukáš Tvrdý (lukast.dev@gmail.com)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_GRID_SHAPE_OPTION_WIDGET_H
#define KIS_GRID_SHAPE_OPTION_WIDGET_H

#include <kis_paintop_option.h>
#include <KisGridShapeOptionData.h>
#include <lager/cursor.hpp>

class KisCurveWidget;

struct KisGridShapeOptionData;

class KisGridShapeOptionWidget : public KisPaintOpOption
{
public:
    using data_type = KisGridShapeOptionData;

    KisGridShapeOptionWidget(lager::cursor<KisGridShapeOptionData> optionData);
    ~KisGridShapeOptionWidget();

    void writeOptionSetting(KisPropertiesConfigurationSP setting) const override;
    void readOptionSetting(const KisPropertiesConfigurationSP setting) override;
    

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // KIS_GRID_SHAPE_OPTION_WIDGET_H
