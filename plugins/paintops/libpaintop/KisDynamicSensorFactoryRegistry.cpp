/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisDynamicSensorFactoryRegistry.h"

#include <KisDynamicSensorIds.h>

#include <KisSimpleDynamicSensorFactory.h>
#include <KisDynamicSensorFactoryTime.h>
#include <KisDynamicSensorFactoryFade.h>
#include <KisDynamicSensorFactoryDistance.h>
#include <KisDynamicSensorFactoryDrawingAngle.h>

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
    addImpl(RotationId, 0, 360, i18n("0°"), i18n("360°"), i18n("°"));
    addImpl(XTiltId, -60, 0, i18n("-60°"), i18n("0°"), i18n("°"));
    addImpl(YTiltId, -60, 0, i18n("-60°"), i18n("0°"), i18n("°"));
    addImpl(TiltDirectionId, 0, 360, i18n("0°"), i18n("360°"), i18n("°"));
    addImpl(TiltElevationId, 90, 0, i18n("90°"), i18n("0°"), i18n("°"));

    add(new KisDynamicSensorFactoryFade());
    add(new KisDynamicSensorFactoryDistance());
    add(new KisDynamicSensorFactoryDrawingAngle());
    add(new KisDynamicSensorFactoryTime());
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
    add(new KisSimpleDynamicSensorFactory(id.id(),
                                          minimumValue,
                                          maximumValue,
                                          minimumLabel,
                                          maximumLabel,
                                          valueSuffix));
}
