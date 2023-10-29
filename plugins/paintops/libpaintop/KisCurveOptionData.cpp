/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisCurveOptionData.h"

KisCurveOptionData::KisCurveOptionData(const QString &prefix,
                                       const KoID &id,
                                       Checkability checkability,
                                       std::optional<bool> isChecked,
                                       const std::pair<qreal, qreal> &valueRange)
    : KisCurveOptionDataCommon(prefix,
                               id,
                               checkability == Checkability::Checkable ||
                                   (checkability == Checkability::CheckableIfHasPrefix && !prefix.isEmpty()),
                               isChecked ? *isChecked : checkability == Checkability::NotCheckable,
                               valueRange.first,
                               valueRange.second,
                               new KisKritaSensorPack(checkability))
{
}

KisCurveOptionData::KisCurveOptionData(const KoID &id,
                                       Checkability checkability,
                                       std::optional<bool> isChecked,
                                       const std::pair<qreal, qreal> &valueRange)
    : KisCurveOptionData("", id, checkability, isChecked, valueRange)
{
}

KisKritaSensorData &KisCurveOptionData::sensorStruct()
{
    return dynamic_cast<KisKritaSensorPack *>(sensorData.data())->sensorsStruct();
}

const KisKritaSensorData &KisCurveOptionData::sensorStruct() const
{
    return dynamic_cast<const KisKritaSensorPack*>(sensorData.constData())->constSensorsStruct();
}
