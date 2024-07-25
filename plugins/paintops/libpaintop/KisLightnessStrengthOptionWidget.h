/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISLIGHTNESSSTRENGTHOPTIONWIDGET_H
#define KISLIGHTNESSSTRENGTHOPTIONWIDGET_H

#include <KisCurveOptionWidget.h>

struct KisLightnessStrengthOptionData;


class PAINTOP_EXPORT KisLightnessStrengthOptionWidget : public KisCurveOptionWidget
{
    Q_OBJECT
public:
    using data_type = KisLightnessStrengthOptionData;

    KisLightnessStrengthOptionWidget(lager::cursor<KisLightnessStrengthOptionData> optionData, lager::reader<bool> lightnessModeEnabled);
    ~KisLightnessStrengthOptionWidget();

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // KISLIGHTNESSSTRENGTHOPTIONWIDGET_H
