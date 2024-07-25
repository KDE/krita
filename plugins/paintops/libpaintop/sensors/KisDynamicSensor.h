/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISDYNAMICSENSOR_H
#define KISDYNAMICSENSOR_H

#include <optional>
#include <kis_cubic_curve.h>
#include <KoID.h>

class KisPaintInformation;
struct KisSensorData;

class KisDynamicSensor
{
public:
    KisDynamicSensor(const KoID &id,
                      const KisSensorData &data,
                      std::optional<KisCubicCurve> curveOverride);
    virtual ~KisDynamicSensor();

    KoID id() const;
    qreal parameter(const KisPaintInformation &info) const;

    virtual bool isAdditive() const;
    virtual bool isAbsoluteRotation() const;

protected:

    virtual qreal value(const KisPaintInformation &info) const = 0;

public:

    static inline qreal scalingToAdditive(qreal x) {
        return -1.0 + 2.0 * x;
    }

    static inline qreal additiveToScaling(qreal x) {
        return 0.5 * (1.0 + x);
    }

private:
    KoID m_id;
    std::optional<KisCubicCurve> m_curve;
};

#endif // KISDYNAMICSENSOR_H
