/*
 *  SPDX-FileCopyrightText: 2022 Agata Cacko <cacko.azh@gmail.com>
 *  SPDX-FileCopyrightText: 2008, 2009, 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_SPRAY_SHAPE_OPTION_WIDGET_H
#define KIS_SPRAY_SHAPE_OPTION_WIDGET_H

#include <kis_paintop_option.h>
#include <KisSprayShapeOptionData.h>
#include <lager/cursor.hpp>

struct KisSprayShapeOptionData;

class KisSprayShapeOptionWidget : public KisPaintOpOption
{
	Q_OBJECT
public:
    using data_type = KisSprayShapeOptionData;

    KisSprayShapeOptionWidget(lager::cursor<KisSprayShapeOptionData> optionData, lager::cursor<int> diameter, lager::cursor<qreal> scale);
    ~KisSprayShapeOptionWidget();

    void writeOptionSetting(KisPropertiesConfigurationSP setting) const override;
    void readOptionSetting(const KisPropertiesConfigurationSP setting) override;
    
private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // KIS_SPRAY_SHAPE_OPTION_WIDGET_H
