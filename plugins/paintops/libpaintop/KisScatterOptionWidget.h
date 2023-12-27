/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISSCATTEROPTIONWIDGET_H
#define KISSCATTEROPTIONWIDGET_H

#include <KisCurveOptionWidget.h>
#include <KisScatterOptionData.h>


class PAINTOP_EXPORT KisScatterOptionWidget : public KisCurveOptionWidget
{
    Q_OBJECT
public:
    using data_type = KisScatterOptionData;

    KisScatterOptionWidget(lager::cursor<KisScatterOptionData> optionData);
    KisScatterOptionWidget(lager::cursor<KisScatterOptionData> optionData, KisPaintOpOption::PaintopCategory categoryOverride);
    ~KisScatterOptionWidget();

    void writeOptionSetting(KisPropertiesConfigurationSP setting) const override;
    void readOptionSetting(const KisPropertiesConfigurationSP setting) override;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // KISSCATTEROPTIONWIDGET_H
