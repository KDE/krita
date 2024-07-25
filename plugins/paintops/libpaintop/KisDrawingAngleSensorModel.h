/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISDRAWINGANGLESENSORMODEL_H
#define KISDRAWINGANGLESENSORMODEL_H

#include "kritapaintop_export.h"

#include <KisCurveOptionData.h>
#include <lager/cursor.hpp>
#include <lager/extra/qt.hpp>

class PAINTOP_EXPORT KisDrawingAngleSensorModel : public QObject
{
    Q_OBJECT
public:
    KisDrawingAngleSensorModel(lager::cursor<KisDrawingAngleSensorData> data, QObject *parent);
    ~KisDrawingAngleSensorModel();

    // the state must be declared **before** any cursors or readers
    lager::cursor<KisDrawingAngleSensorData> m_data;
    LAGER_QT_CURSOR(bool, fanCornersEnabled);
    LAGER_QT_CURSOR(int, fanCornersStep);
    LAGER_QT_CURSOR(int, angleOffset);
    LAGER_QT_CURSOR(bool, lockedAngleMode);
};

#endif // KISDRAWINGANGLESENSORMODEL_H
