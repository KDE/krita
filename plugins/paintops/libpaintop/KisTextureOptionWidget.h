/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISTEXTUREOPTIONWIDGET_H
#define KISTEXTUREOPTIONWIDGET_H

#include <kis_paintop_option.h>
#include <KisTextureOptionData.h>
#include <lager/cursor.hpp>


class PAINTOP_EXPORT KisTextureOptionWidget : public KisPaintOpOption
{
public:
    using data_type = KisTextureOptionData;

    KisTextureOptionWidget(lager::cursor<KisTextureOptionData> optionData, KisResourcesInterfaceSP resourcesInterface, KisBrushTextureFlags flags = None);
    ~KisTextureOptionWidget();

    void writeOptionSetting(KisPropertiesConfigurationSP setting) const override;
    void readOptionSetting(const KisPropertiesConfigurationSP setting) override;

protected:
    OptionalLodLimitationsReader lodLimitationsReader() const override;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // KISTEXTUREOPTIONWIDGET_H
