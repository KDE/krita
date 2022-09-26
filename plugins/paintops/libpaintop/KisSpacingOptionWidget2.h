/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISSPACINGOPTIONWIDGET2_H
#define KISSPACINGOPTIONWIDGET2_H

#include <KisCurveOptionWidget2.h>
#include <KisSpacingOptionData.h>

class PAINTOP_EXPORT KisSpacingOptionWidget2 : public KisCurveOptionWidget2
{
    Q_OBJECT
public:
    KisSpacingOptionWidget2(lager::cursor<KisSpacingOptionData> optionData);
    ~KisSpacingOptionWidget2();

    void writeOptionSetting(KisPropertiesConfigurationSP setting) const override;
    void readOptionSetting(const KisPropertiesConfigurationSP setting) override;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // KISSPACINGOPTIONWIDGET2_H
