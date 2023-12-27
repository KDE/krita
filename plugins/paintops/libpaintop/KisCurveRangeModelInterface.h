/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISCURVERANGEMODELINTERFACE_H
#define KISCURVERANGEMODELINTERFACE_H

#include "kritapaintop_export.h"

#include <lager/cursor.hpp>
#include <lager/reader.hpp>

#include <QString>
#include <QRectF>

class PAINTOP_EXPORT KisCurveRangeModelInterface
{
public:
    virtual ~KisCurveRangeModelInterface();
    virtual lager::cursor<QString> curve() = 0;
    virtual lager::reader<QString> xMinLabel() = 0;
    virtual lager::reader<QString> xMaxLabel() = 0;
    virtual lager::reader<QString> yMinLabel() = 0;
    virtual lager::reader<QString> yMaxLabel() = 0;
    virtual lager::reader<qreal> yMinValue() = 0;
    virtual lager::reader<qreal> yMaxValue() = 0;
    virtual lager::reader<QString> yValueSuffix() = 0;
    virtual lager::reader<qreal> xMinValue() = 0;
    virtual lager::reader<qreal> xMaxValue() = 0;
    virtual lager::reader<QString> xValueSuffix() = 0;
};

// usage: factory(curve, curveRange, activeSensorId, activeSensorLength)
using KisCurveRangeModelFactory = 
    std::function<KisCurveRangeModelInterface *(lager::cursor<QString>, 
                                                lager::cursor<QRectF>,
                                                lager::reader<QString>, 
                                                lager::reader<int>)>;

#endif // KISCURVERANGEMODELINTERFACE_H
