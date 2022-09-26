/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISSCATTEROPTIONWIDGET_H
#define KISSCATTEROPTIONWIDGET_H

#include <KisCurveOptionWidget2.h>
#include <KisScatterOptionData.h>


class PAINTOP_EXPORT KisScatterOptionWidget : public KisCurveOptionWidget2
{
    Q_OBJECT
public:
    KisScatterOptionWidget(lager::cursor<KisScatterOptionData> optionData);
    ~KisScatterOptionWidget();

    void writeOptionSetting(KisPropertiesConfigurationSP setting) const override;
    void readOptionSetting(const KisPropertiesConfigurationSP setting) override;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // KISSCATTEROPTIONWIDGET_H
