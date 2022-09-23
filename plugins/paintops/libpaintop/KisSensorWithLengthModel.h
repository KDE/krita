/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISSENSORWITHLENGTHMODEL_H
#define KISSENSORWITHLENGTHMODEL_H

#include "kritapaintop_export.h"

#include <KisCurveOptionData.h>
#include <lager/cursor.hpp>
#include <lager/extra/qt.hpp>


class PAINTOP_EXPORT KisSensorWithLengthModel : public QObject
{
    Q_OBJECT
public:
    KisSensorWithLengthModel(lager::cursor<KisSensorWithLengthData> data, QObject *parent);


    // the state must be declared **before** any cursors or readers
    lager::cursor<KisSensorWithLengthData> m_data;
    LAGER_QT_CURSOR(int, length);
    LAGER_QT_CURSOR(bool, isPeriodic);
};


#endif // KISSENSORWITHLENGTHMODEL_H
