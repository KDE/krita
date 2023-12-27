/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisSensorWithLengthModel.h"

KisSensorWithLengthModel::KisSensorWithLengthModel(lager::cursor<KisSensorWithLengthData> data, QObject *parent)
    : QObject(parent)
    , m_data(data)
    , LAGER_QT(length) {m_data[&KisSensorWithLengthData::length]}
    , LAGER_QT(isPeriodic) {m_data[&KisSensorWithLengthData::isPeriodic]}
{
}
