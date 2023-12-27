/*
 * SPDX-FileCopyrightText: 2022 Sharaf Zaman <shzam@sdf.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef __KISDUPLICATEOPTIONWIDGET_H_
#define __KISDUPLICATEOPTIONWIDGET_H_

#include <KisDuplicateOptionData.h>
#include <kis_paintop_option.h>

class KisDuplicateOptionWidget : public KisPaintOpOption
{
    Q_OBJECT
public:
    using data_type = KisDuplicateOptionData;

    KisDuplicateOptionWidget(lager::cursor<data_type> optionData);
    ~KisDuplicateOptionWidget();

    void writeOptionSetting(KisPropertiesConfigurationSP setting) const override;
    void readOptionSetting(const KisPropertiesConfigurationSP setting) override;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // __KISDUPLICATEOPTIONWIDGET_H_
