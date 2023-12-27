/*
 *  SPDX-FileCopyrightText: 2008 Lukas Tvrdy <lukast.dev@gmail.com>
 *  SPDX-FileCopyrightText: 2010 José Luis Vergara <pentalis@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_HATCHING_OPTIONS_WIDGET_H
#define KIS_HATCHING_OPTIONS_WIDGET_H

#include <kis_paintop_option.h>
#include <KisHatchingOptionsData.h>
#include <lager/cursor.hpp>

struct KisHatchingOptionsData;

class KisHatchingOptionsWidget : public KisPaintOpOption
{
public:
    using data_type = KisHatchingOptionsData;

    KisHatchingOptionsWidget(lager::cursor<KisHatchingOptionsData> optionData);
    ~KisHatchingOptionsWidget();

    void writeOptionSetting(KisPropertiesConfigurationSP setting) const override;
    void readOptionSetting(const KisPropertiesConfigurationSP setting) override;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // KIS_HATCHING_OPTIONS_WIDGET_H
