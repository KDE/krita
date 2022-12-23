/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef MYPAINTCURVEOPTIONWIDGET2_H
#define MYPAINTCURVEOPTIONWIDGET2_H

#include <KisCurveOptionWidget2.h>
#include <MyPaintCurveOptionData.h>

class MyPaintCurveOptionWidget2 : public KisCurveOptionWidget2
{
public:
    using data_type = MyPaintCurveOptionData;

public:
    MyPaintCurveOptionWidget2(lager::cursor<MyPaintCurveOptionData> optionData,
                              qreal maxYRange, const QString &yValueSuffix);
    ~MyPaintCurveOptionWidget2();

    using KisCurveOptionWidget2::strengthValueDenorm;

    OptionalLodLimitationsReader lodLimitationsReader() const override;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // MYPAINTCURVEOPTIONWIDGET2_H
