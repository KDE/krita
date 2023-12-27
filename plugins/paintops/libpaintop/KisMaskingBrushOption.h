/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISMASKINGBRUSHOPTION_H
#define KISMASKINGBRUSHOPTION_H

#include <kritapaintop_export.h>
#include <QScopedPointer>

#include <kis_types.h>
#include "kis_paintop_option.h"
#include <lager/reader.hpp>

class PAINTOP_EXPORT KisMaskingBrushOption : public KisPaintOpOption
{
    Q_OBJECT
public:
    KisMaskingBrushOption(lager::reader<qreal> effectiveBrushSize);
    ~KisMaskingBrushOption() override;

    void writeOptionSetting(KisPropertiesConfigurationSP setting) const override;
    void readOptionSetting(const KisPropertiesConfigurationSP setting) override;

    void setImage(KisImageWSP image) override;

    void lodLimitations(KisPaintopLodLimitations *l) const override;

    lager::reader<bool> maskingBrushEnabledReader() const;

private Q_SLOTS:
    void slotCompositeModeWidgetChanged(int index);
    void slotCompositeModePropertyChanged(const QString &value);

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};



#endif // KISMASKINGBRUSHOPTION_H
