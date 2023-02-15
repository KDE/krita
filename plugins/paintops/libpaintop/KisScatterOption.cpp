/*
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *  SPDX-FileCopyrightText: 2011 Silvio Heinrich <plassy@web.de>
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisScatterOption.h"

#include <QVector2D>

#include <kis_properties_configuration.h>
#include <kis_paint_information.h>
#include <KisScatterOptionData.h>

#include <KisPaintOpOptionUtils.h>
namespace kpou = KisPaintOpOptionUtils;


KisScatterOption::KisScatterOption(const KisPropertiesConfiguration *setting)
    : KisScatterOption(kpou::loadOptionData<KisScatterOptionData>(setting))
{
}

KisScatterOption::KisScatterOption(const KisScatterOptionData &data)
    : KisCurveOption(data)
    , m_axisX(data.axisX)
    , m_axisY(data.axisY)
{
}

QPointF KisScatterOption::apply(const KisPaintInformation& info, qreal width, qreal height) const
{
    if ((!m_axisX && !m_axisY) || (!isChecked())) {
        return info.pos();
    }

    // just use the most significant dimension for calculations
    qreal diameter = qMax(width, height);
    qreal sensorValue = computeSizeLikeValue(info);

    qreal jitter = (2.0 * info.randomSource()->generateNormalized() - 1.0) * diameter * sensorValue;
    QPointF result(0.0, 0.0);

    if (m_axisX && m_axisY) {
        qreal jitterY = (2.0 * info.randomSource()->generateNormalized() - 1.0) * diameter * sensorValue;
        result = QPointF(jitter, jitterY);
        return info.pos() + result;
    }

    qreal drawingAngle = info.drawingAngle();
    QVector2D movement(cos(drawingAngle), sin(drawingAngle));
    if (m_axisX) {
        movement *= jitter;
        result = movement.toPointF();
    }
    else if (m_axisY) {
        QVector2D movementNormal(-movement.y(), movement.x());
        movementNormal *= jitter;
        result = movementNormal.toPointF();
    }

    return info.pos() + result;
}
