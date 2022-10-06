/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISCURVEOPTIONMODEL_H
#define KISCURVEOPTIONMODEL_H

#include "kritapaintop_export.h"
#include "KisCurveOptionData.h"

#include <lager/state.hpp>
#include <lager/constant.hpp>
#include <lager/extra/qt.hpp>

using RangeState = std::tuple<qreal, qreal>;
using LabelsState = std::tuple<QString, int>;


class PAINTOP_EXPORT KisCurveOptionModel : public QObject
{
    Q_OBJECT
public:
    KisCurveOptionModel(lager::cursor<KisCurveOptionData> optionData, lager::reader<bool> externallyEnabled);
    ~KisCurveOptionModel();

    // the state must be declared **before** any cursors or readers
    lager::cursor<KisCurveOptionData> optionData;
    lager::state<QString, lager::automatic_tag> activeSensorIdData;
    LAGER_QT_READER(bool, isCheckable);
    LAGER_QT_CURSOR(bool, isChecked);
    LAGER_QT_READER(bool, effectiveIsChecked);
    LAGER_QT_CURSOR(qreal, strengthValue);
    LAGER_QT_READER(RangeState, range);
    LAGER_QT_CURSOR(bool, useCurve);
    LAGER_QT_CURSOR(bool, useSameCurve);
    LAGER_QT_CURSOR(int, curveMode);
    LAGER_QT_CURSOR(QString, activeSensorId);
    LAGER_QT_READER(int, activeSensorLength);
    LAGER_QT_READER(LabelsState, labelsState);
    LAGER_QT_CURSOR(QString, activeCurve);

    KisCurveOptionData bakedOptionData() const;
};

#endif // KISCURVEOPTIONMODEL_H
