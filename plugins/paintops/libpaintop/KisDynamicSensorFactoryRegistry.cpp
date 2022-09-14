/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisDynamicSensorFactoryRegistry.h"

// TODO: mode ids into a sepatate header
#include <kis_dynamic_sensor.h>

#include <KisSimpleDynamicSensorFactory.h>
#include <KisLengthBasedDynamicSensorFactory.h>

Q_GLOBAL_STATIC(KisDynamicSensorFactoryRegistry, s_instance)

KisDynamicSensorFactoryRegistry::KisDynamicSensorFactoryRegistry()
{
    addImpl(PressureId, 0, 100, i18n("0.0"), i18n("1.0"), i18n("%"));

    addImpl(FuzzyPerDabId, 0, 100, "", "", i18n("%"));
    addImpl(FuzzyPerStrokeId, 0, 100, "", "", i18n("%"));
    addImpl(PressureInId, 0, 100, i18n("Low"), i18n("High"), i18n("%"));
    addImpl(SpeedId, 0, 100, i18n("Slow"), i18n("Fast"), i18n("%"));
    addImpl(PerspectiveId, 0, 100, i18n("Far"), i18n("Near"), i18n("%"));
    addImpl(TangentialPressureId, 0, 100, i18n("Low"), i18n("High"), i18n("%"));
    // TODO: widget
    addImpl(DrawingAngleId, 0, 360, i18n("0°"), i18n("360°"), i18n("°"));
    addImpl(RotationId, 0, 360, i18n("0°"), i18n("360°"), i18n("°"));
    addImpl(XTiltId, -60, 0, i18n("-60°"), i18n("0°"), i18n("°"));
    addImpl(YTiltId, -60, 0, i18n("-60°"), i18n("0°"), i18n("°"));
    addImpl(TiltDirectionId, 0, 360, i18n("0°"), i18n("360°"), i18n("°"));
    addImpl(TiltElevationId, 90, 0, i18n("90°"), i18n("0°"), i18n("°"));

    // TODO: widget
    addImpl(FadeId, 0, 1000, i18n("0"), [] (int length) {return i18n("%1", length); }, "");
    addImpl(DistanceId, 0, 30, i18n("0"), [] (int length) {return i18n("%1 px", length); }, i18n(" px"));
    addImpl(TimeId, 0, 3000, i18n("0"), [] (int length) {return i18n("%1 ms", length); }, i18n(" ms"));
}

KisDynamicSensorFactoryRegistry::~KisDynamicSensorFactoryRegistry()
{
    Q_FOREACH (const QString & id, keys()) {
        delete get(id);
    }
}

KisDynamicSensorFactoryRegistry *KisDynamicSensorFactoryRegistry::instance()
{
    return s_instance;
}

void KisDynamicSensorFactoryRegistry::addImpl(const KoID &id, int minimumValue, int maximumValue, const QString &minimumLabel, const QString &maximumLabel, const QString &valueSuffix)
{
    add(id.id(), new KisSimpleDynamicSensorFactory(minimumValue, maximumValue, minimumLabel, maximumLabel, valueSuffix));
}

void KisDynamicSensorFactoryRegistry::addImpl(const KoID &id,
                                              int minimumValue,
                                              int maximumValue,
                                              const QString &minimumLabel,
                                              std::function<QString(int)> calcMaximumLabel,
                                              const QString &valueSuffix)
{
    add(id.id(), new KisLengthBasedDynamicSensorFactory(minimumValue, maximumValue,
                                                        minimumLabel, calcMaximumLabel,
                                                        valueSuffix));

}
