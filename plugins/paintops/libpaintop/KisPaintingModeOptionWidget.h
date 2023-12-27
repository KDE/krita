/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISPAINTINGMODEOPTIONWIDGET_H
#define KISPAINTINGMODEOPTIONWIDGET_H

#include <kis_paintop_option.h>
#include <KisPaintingModeOptionData.h>
#include <lager/cursor.hpp>

class PAINTOP_EXPORT KisPaintingModeOptionWidget : public KisPaintOpOption
{
public:
    using data_type = KisPaintingModeOptionData;

    KisPaintingModeOptionWidget(lager::cursor<KisPaintingModeOptionData> optionData);
    KisPaintingModeOptionWidget(lager::cursor<KisPaintingModeOptionData> optionData, lager::reader<bool> maskingBrushEnabled);
    ~KisPaintingModeOptionWidget();

    void writeOptionSetting(KisPropertiesConfigurationSP setting) const override;
    void readOptionSetting(const KisPropertiesConfigurationSP setting) override;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // KISPAINTINGMODEOPTIONWIDGET_H
