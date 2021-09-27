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

class KisMaskingBrushOptionProperties;

class PAINTOP_EXPORT KisMaskingBrushOption : public KisPaintOpOption
{
    Q_OBJECT
public:
    typedef std::function<qreal()> MasterBrushSizeAdapter;

public:
    KisMaskingBrushOption(MasterBrushSizeAdapter masterBrushSizeAdapter);
    ~KisMaskingBrushOption() override;

    void writeOptionSetting(KisPropertiesConfigurationSP setting) const override;
    void readOptionSetting(const KisPropertiesConfigurationSP setting) override;

    void setImage(KisImageWSP image) override;

    void lodLimitations(KisPaintopLodLimitations *l) const override;

private Q_SLOTS:
    void slotMaskingBrushChanged();

private:
    void updateWarningLabelStatus();

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};



#endif // KISMASKINGBRUSHOPTION_H
