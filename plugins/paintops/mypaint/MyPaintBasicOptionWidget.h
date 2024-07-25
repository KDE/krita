/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef MYPAINTBASICOPTIONWIDGET_H
#define MYPAINTBASICOPTIONWIDGET_H

#include <kis_paintop_option.h>
#include <MyPaintBasicOptionData.h>
#include <lager/cursor.hpp>


class MyPaintBasicOptionWidget : public KisPaintOpOption
{
public:
    using data_type = MyPaintBasicOptionData;

    MyPaintBasicOptionWidget(lager::cursor<MyPaintBasicOptionData> optionData,
                               lager::cursor<qreal> radiusCursor,
                               lager::cursor<qreal> hardnessCursor,
                               lager::cursor<qreal> opacityCursor);
    ~MyPaintBasicOptionWidget();

    void writeOptionSetting(KisPropertiesConfigurationSP setting) const override;
    void readOptionSetting(const KisPropertiesConfigurationSP setting) override;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // MYPAINTBASICOPTIONWIDGET_H
