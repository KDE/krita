/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISSCATTEROPTION_H
#define KISSCATTEROPTION_H

#include <KisCurveOption.h>

struct KisScatterOptionData;

class PAINTOP_EXPORT KisScatterOption : public KisCurveOption
{
public:
    KisScatterOption(const KisPropertiesConfiguration *setting);

    QPointF apply(const KisPaintInformation& info, qreal width, qreal height) const;

private:
    KisScatterOption(const KisScatterOptionData &data);

private:
    bool m_axisX;
    bool m_axisY;
};

#endif // KISSCATTEROPTION_H
