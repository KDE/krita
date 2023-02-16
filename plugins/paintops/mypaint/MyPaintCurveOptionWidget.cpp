/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "MyPaintCurveOptionWidget.h"
#include "KisLager.h"
#include "kis_paintop_option.h"
#include "MyPaintCurveRangeModel.h"
#include "MyPaintCurveOptionRangeControlsStrategy.h"
#include "KisCurveOptionInputControlsStrategy.h"
#include "MyPaintSensorPack.h"
#include "kis_paintop_lod_limitations.h"

namespace {
KisPaintopLodLimitations calcGeneralMyPaintLodLimitations(const MyPaintCurveOptionData &data)
{
    KisPaintopLodLimitations l;

    if (data.sensorStruct().sensorRandom.isActive) {
        l.limitations << KoID(QString("Random Sensor Active, %1").arg(data.id.id()), i18nc("PaintOp instant preview limitation", "Random Sensor Active in %1 option, consider disabling Instant Preview", data.id.name()));
    }

    return l;
}
} // namespace

struct MyPaintCurveOptionWidget::Private
{
    Private(lager::cursor<MyPaintCurveOptionData> optionData)
        : lodLimitations(optionData.map(&calcGeneralMyPaintLodLimitations))
    {}

    lager::reader<KisPaintopLodLimitations> lodLimitations;
};

MyPaintCurveOptionWidget::MyPaintCurveOptionWidget(lager::cursor<MyPaintCurveOptionData> optionData,
                                                     qreal maxYRange,
                                                     const QString &yValueSuffix)
    : KisCurveOptionWidget(optionData.zoom(kislager::lenses::to_base<KisCurveOptionDataCommon>),
                            KisPaintOpOption::GENERAL,
                            i18n("Base Value: "), yValueSuffix, 1.0,
                            lager::make_constant(true),
                            std::nullopt,
                            MyPaintCurveRangeModel::factory(maxYRange, yValueSuffix),
                            KisCurveOptionInputControlsStrategyDouble::factory(),
                            MyPaintCurveOptionRangeControlsStrategy::factory(),
                            UseFloatingPointStrength)
    , m_d(new Private(optionData))
{
}

MyPaintCurveOptionWidget::~MyPaintCurveOptionWidget()
{
}

KisPaintOpOption::OptionalLodLimitationsReader MyPaintCurveOptionWidget::lodLimitationsReader() const
{
    return m_d->lodLimitations;
}
