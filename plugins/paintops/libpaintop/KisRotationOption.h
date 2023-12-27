/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISROTATIONOPTION_H
#define KISROTATIONOPTION_H

#include <KisCurveOption.h>

struct KisRotationOptionData;

class PAINTOP_EXPORT KisRotationOption : public KisCurveOption
{
public:
    KisRotationOption(const KisPropertiesConfiguration *setting);

    qreal apply(const KisPaintInformation & info) const;
    void applyFanCornersInfo(KisPaintOp *op);

private:
    KisRotationOption(const KisRotationOptionData &data);

private:
    bool m_fanCornersEnabled {false};
    qreal m_fanCornersStep {30.0};
};

#endif // KISROTATIONOPTION_H
