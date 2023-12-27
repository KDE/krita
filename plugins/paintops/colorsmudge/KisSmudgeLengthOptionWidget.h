/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISSMUDGELENGTHOPTIONWIDGET_H
#define KISSMUDGELENGTHOPTIONWIDGET_H

#include <KisCurveOptionWidget.h>
#include <KisSmudgeLengthOptionData.h>


class KisSmudgeLengthOptionWidget : public KisCurveOptionWidget
{
    Q_OBJECT
public:
    using data_type = KisSmudgeLengthOptionData;

    KisSmudgeLengthOptionWidget(lager::cursor<KisSmudgeLengthOptionData> optionData,
                                lager::reader<bool> isBrushPierced,
                                lager::reader<bool> forceNewEngine);
    ~KisSmudgeLengthOptionWidget();

    void writeOptionSetting(KisPropertiesConfigurationSP setting) const override;
    void readOptionSetting(const KisPropertiesConfigurationSP setting) override;

    lager::reader<bool> useNewEngine() const;

private:
    void updateBrushPierced(bool pierced);

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // KISSMUDGELENGTHOPTIONWIDGET_H
