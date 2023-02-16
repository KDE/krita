/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISSMUDGEOVERLAYMODEOPTIONWIDGET_H
#define KISSMUDGEOVERLAYMODEOPTIONWIDGET_H

#include <kis_paintop_option.h>
#include <KisSmudgeOverlayModeOptionData.h>
#include <lager/cursor.hpp>

class KisSmudgeOverlayModeOptionWidget : public KisPaintOpOption
{
public:
    using data_type = KisSmudgeOverlayModeOptionData;

    KisSmudgeOverlayModeOptionWidget(lager::cursor<KisSmudgeOverlayModeOptionData> optionData,
                                     lager::reader<bool> overlayModeAllowed);
    ~KisSmudgeOverlayModeOptionWidget();

    void writeOptionSetting(KisPropertiesConfigurationSP setting) const override;
    void readOptionSetting(const KisPropertiesConfigurationSP setting) override;

    OptionalLodLimitationsReader lodLimitationsReader() const override;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // KISSMUDGEOVERLAYMODEOPTIONWIDGET_H
