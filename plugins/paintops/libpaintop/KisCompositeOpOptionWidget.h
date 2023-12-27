/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISCOMPOSITEOPOPTIONWIDGET_H
#define KISCOMPOSITEOPOPTIONWIDGET_H

#include <kis_paintop_option.h>
#include <KisCompositeOpOptionData.h>
#include <lager/cursor.hpp>


class PAINTOP_EXPORT KisCompositeOpOptionWidget : public KisPaintOpOption
{
public:
    using data_type = KisCompositeOpOptionData;

    KisCompositeOpOptionWidget(lager::cursor<KisCompositeOpOptionData> optionData);
    ~KisCompositeOpOptionWidget();

    void writeOptionSetting(KisPropertiesConfigurationSP setting) const override;
    void readOptionSetting(const KisPropertiesConfigurationSP setting) override;

private:
    void updateCompositeOpLabel(const QString &id);

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // KISCOMPOSITEOPOPTIONWIDGET_H
