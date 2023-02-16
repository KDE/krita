/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISSHARPNESSOPTIONWIDGET_H
#define KISSHARPNESSOPTIONWIDGET_H

#include <KisCurveOptionWidget.h>
#include <KisSharpnessOptionData.h>

class PAINTOP_EXPORT KisSharpnessOptionWidget : public KisCurveOptionWidget
{
    Q_OBJECT
public:
    using data_type = KisSharpnessOptionData;

    KisSharpnessOptionWidget(lager::cursor<KisSharpnessOptionData> optionData);
    ~KisSharpnessOptionWidget();

    void writeOptionSetting(KisPropertiesConfigurationSP setting) const override;
    void readOptionSetting(const KisPropertiesConfigurationSP setting) override;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // KISSHARPNESSOPTIONWIDGET_H
