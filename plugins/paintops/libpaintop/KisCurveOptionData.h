/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISCURVEOPTIONDATA_H
#define KISCURVEOPTIONDATA_H

#include <KisCurveOptionDataCommon.h>
#include <KisKritaSensorPack.h>


struct PAINTOP_EXPORT KisCurveOptionData : KisCurveOptionDataCommon
{
    using Checkability = KisKritaSensorPack::Checkability;

    /**
     * When `isChecked` is std::nullopt, then the initial checked state
     * is deduced from the checkability property. Non-checkable options
     * will always be checked, checkable --- unchecked
     */
    KisCurveOptionData(const QString &prefix,
                       const KoID &id,
                       Checkability checkability = Checkability::Checkable,
                       std::optional<bool> isChecked = std::nullopt,
                       const std::pair<qreal, qreal> &valueRange = {0.0, 1.0});

    KisCurveOptionData(const KoID &id,
                       Checkability checkability = Checkability::Checkable,
                       std::optional<bool> isChecked = std::nullopt,
                       const std::pair<qreal, qreal> &valueRange = {0.0, 1.0});
        
    KisKritaSensorData& sensorStruct();
    const KisKritaSensorData& sensorStruct() const;
};

#endif // KISCURVEOPTIONDATA_H
