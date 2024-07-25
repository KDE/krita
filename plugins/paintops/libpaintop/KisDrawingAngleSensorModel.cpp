/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisDrawingAngleSensorModel.h"

KisDrawingAngleSensorModel::KisDrawingAngleSensorModel(lager::cursor<KisDrawingAngleSensorData> data, QObject *parent)
    : QObject(parent)
    , m_data{data}
    , LAGER_QT(fanCornersEnabled) {m_data[&KisDrawingAngleSensorData::fanCornersEnabled]}
    , LAGER_QT(fanCornersStep) {m_data[&KisDrawingAngleSensorData::fanCornersStep]}
    , LAGER_QT(angleOffset) {m_data[&KisDrawingAngleSensorData::angleOffset]}
    , LAGER_QT(lockedAngleMode) {m_data[&KisDrawingAngleSensorData::lockedAngleMode]}
{
}

KisDrawingAngleSensorModel::~KisDrawingAngleSensorModel()
{
}
