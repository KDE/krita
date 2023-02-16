/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISAIRBRUSHOPTIONWIDGET_H
#define KISAIRBRUSHOPTIONWIDGET_H

#include <kis_paintop_option.h>
#include <KisAirbrushOptionData.h>
#include <lager/cursor.hpp>

class PAINTOP_EXPORT KisAirbrushOptionWidget : public KisPaintOpOption
{
public:
    using data_type = KisAirbrushOptionData;

    KisAirbrushOptionWidget(lager::cursor<KisAirbrushOptionData> optionData, bool canIgnoreSpacing = true);
    ~KisAirbrushOptionWidget();

    void writeOptionSetting(KisPropertiesConfigurationSP setting) const override;
    void readOptionSetting(const KisPropertiesConfigurationSP setting) override;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // KISAIRBRUSHOPTIONWIDGET_H
