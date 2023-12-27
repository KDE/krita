/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISSENSORPACKINTERFACE_H
#define KISSENSORPACKINTERFACE_H

#include "kritapaintop_export.h"
#include <QSharedData>

struct KisSensorData;
struct KisCurveOptionDataCommon;
class KisPropertiesConfiguration;

class PAINTOP_EXPORT KisSensorPackInterface : public QSharedData
{
public:
    virtual ~KisSensorPackInterface();

    virtual KisSensorPackInterface * clone() const = 0;

    virtual std::vector<const KisSensorData *> constSensors() const = 0;
    virtual std::vector<KisSensorData *> sensors() = 0;

    virtual bool compare(const KisSensorPackInterface *rhs) const = 0;
    virtual bool read(KisCurveOptionDataCommon &data, const KisPropertiesConfiguration *setting) const = 0;
    virtual void write(const KisCurveOptionDataCommon &data, KisPropertiesConfiguration *setting) const = 0;
    virtual int calcActiveSensorLength(const QString &activeSensorId) const;
};

template<>
inline KisSensorPackInterface* QSharedDataPointer<KisSensorPackInterface>::clone()
{
    return d->clone();
}

#endif // KISSENSORPACKINTERFACE_H