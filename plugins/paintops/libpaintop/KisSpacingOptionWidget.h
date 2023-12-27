/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISSPACINGOPTIONWIDGET_H
#define KISSPACINGOPTIONWIDGET_H

#include <KisCurveOptionWidget.h>
#include <KisSpacingOptionData.h>

class PAINTOP_EXPORT KisSpacingOptionWidget : public KisCurveOptionWidget
{
    Q_OBJECT
public:
    using data_type = KisSpacingOptionData;

    KisSpacingOptionWidget(lager::cursor<KisSpacingOptionData> optionData);
    ~KisSpacingOptionWidget();

    void writeOptionSetting(KisPropertiesConfigurationSP setting) const override;
    void readOptionSetting(const KisPropertiesConfigurationSP setting) override;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // KISSPACINGOPTIONWIDGET_H
