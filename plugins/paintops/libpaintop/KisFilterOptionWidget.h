/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISFILTEROPTIONWIDGET_H
#define KISFILTEROPTIONWIDGET_H

#include <kis_paintop_option.h>
#include <KisFilterOptionData.h>
#include <lager/cursor.hpp>

class PAINTOP_EXPORT KisFilterOptionWidget : public KisPaintOpOption
{
    Q_OBJECT
public:
    using data_type = KisFilterOptionData;

    KisFilterOptionWidget(lager::cursor<KisFilterOptionData> optionData);
    ~KisFilterOptionWidget();

    void writeOptionSetting(KisPropertiesConfigurationSP setting) const override;
    void readOptionSetting(const KisPropertiesConfigurationSP setting) override;

    void setImage(KisImageWSP image) override;
    void setNode(KisNodeWSP node) override;

private Q_SLOTS:
    void updateFilterState(const QString &fitlerId, const QString &filterConfig, bool forceResetWidget = false);
    void slotFilterIdChangedInGui(const KoID &fitlerId);
    void slotFilterConfigChangedInGui();

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // KISFILTEROPTIONWIDGET_H
