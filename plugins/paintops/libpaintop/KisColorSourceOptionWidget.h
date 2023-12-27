/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISCOLORSOURCEOPTIONWIDGET_H
#define KISCOLORSOURCEOPTIONWIDGET_H

#include <kis_paintop_option.h>
#include <KisColorSourceOptionData.h>
#include <lager/cursor.hpp>

class PAINTOP_EXPORT KisColorSourceOptionWidget : public KisPaintOpOption
{
public:
    using data_type = KisColorSourceOptionData;

    KisColorSourceOptionWidget(lager::cursor<KisColorSourceOptionData> optionData);
    ~KisColorSourceOptionWidget();

    void writeOptionSetting(KisPropertiesConfigurationSP setting) const override;
    void readOptionSetting(const KisPropertiesConfigurationSP setting) override;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // KISCOLORSOURCEOPTIONWIDGET_H
