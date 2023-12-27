/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISSENSORDATA_H
#define KISSENSORDATA_H

#include <boost/operators.hpp>
#include "kritapaintop_export.h"
#include "kis_assert.h"
#include "kis_paintop_option.h"

struct PAINTOP_EXPORT KisSensorData : public boost::equality_comparable<KisSensorData>
{
    KisSensorData(const KoID &sensorId);
    virtual ~KisSensorData();

    inline friend bool operator==(const KisSensorData &lhs, const KisSensorData &rhs) {
        return lhs.id == rhs.id &&
                lhs.curve == rhs.curve &&
                lhs.isActive == rhs.isActive;
    }

    virtual QRectF baseCurveRange() const;
    virtual void setBaseCurveRange(const QRectF &rect);
    virtual void write(QDomDocument& doc, QDomElement &e) const;
    virtual void read(const QDomElement &e);
    virtual void reset();

    KoID id;
    QString curve;

    // not a part of XML data, managed by the curve option
    bool isActive = false;
};

struct PAINTOP_EXPORT KisSensorWithLengthData : public KisSensorData, public boost::equality_comparable<KisSensorWithLengthData>
{
    KisSensorWithLengthData(const KoID &sensorId, const QLatin1String &lengthTag = {});

    inline friend bool operator==(const KisSensorWithLengthData &lhs, const KisSensorWithLengthData &rhs) {
        return *static_cast<const KisSensorData*>(&lhs) == *static_cast<const KisSensorData*>(&rhs) &&
                lhs.length == rhs.length &&
                lhs.isPeriodic == rhs.isPeriodic &&
                lhs.m_lengthTag == rhs.m_lengthTag;
    }

    void write(QDomDocument& doc, QDomElement &e) const override;
    void read(const QDomElement &e) override;
    void reset() override;

    int length = 30;
    bool isPeriodic = false;
private:
    QLatin1String m_lengthTag;
};

struct PAINTOP_EXPORT KisDrawingAngleSensorData : public KisSensorData, public boost::equality_comparable<KisDrawingAngleSensorData>
{
    KisDrawingAngleSensorData();

    inline friend bool operator==(const KisDrawingAngleSensorData &lhs, const KisDrawingAngleSensorData &rhs) {
        return *static_cast<const KisSensorData*>(&lhs) == *static_cast<const KisSensorData*>(&rhs) &&
                lhs.fanCornersEnabled == rhs.fanCornersEnabled &&
                lhs.fanCornersStep == rhs.fanCornersStep &&
                lhs.angleOffset == rhs.angleOffset &&
                lhs.lockedAngleMode == rhs.lockedAngleMode;
    }

    void write(QDomDocument& doc, QDomElement &e) const override;
    void read(const QDomElement &e) override;
    void reset() override;

    bool fanCornersEnabled = false;
    int fanCornersStep = 30;
    int angleOffset = 0; // in degrees
    bool lockedAngleMode = false;
};

#endif // KISSENSORDATA_H
