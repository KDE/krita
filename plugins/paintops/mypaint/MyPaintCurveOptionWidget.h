/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef MYPAINTCURVEOPTIONWIDGET_H
#define MYPAINTCURVEOPTIONWIDGET_H

#include <KisCurveOptionWidget.h>
#include <MyPaintCurveOptionData.h>

class MyPaintCurveOptionWidget : public KisCurveOptionWidget
{
public:
    using data_type = MyPaintCurveOptionData;

public:
    MyPaintCurveOptionWidget(lager::cursor<MyPaintCurveOptionData> optionData,
                              qreal maxYRange, const QString &yValueSuffix);
    ~MyPaintCurveOptionWidget();

    using KisCurveOptionWidget::strengthValueDenorm;

    OptionalLodLimitationsReader lodLimitationsReader() const override;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // MYPAINTCURVEOPTIONWIDGET_H
