/*
 *  SPDX-FileCopyrightText: 2012 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_SMOOTHING_OPTIONS_H
#define KIS_SMOOTHING_OPTIONS_H

#include <qglobal.h>
#include <QObject>
#include <QSharedPointer>
#include <QScopedPointer>
#include <kritaui_export.h>


class KRITAUI_EXPORT KisSmoothingOptions : public QObject
{
    Q_OBJECT
public:
    enum SmoothingType {
        NO_SMOOTHING = 0,
        SIMPLE_SMOOTHING,
        WEIGHTED_SMOOTHING,
        STABILIZER
    };

public:

    KisSmoothingOptions(bool useSavedSmoothing = true);
    ~KisSmoothingOptions() override;

    SmoothingType smoothingType() const;
    void setSmoothingType(SmoothingType value);

    qreal smoothnessDistance() const;
    void setSmoothnessDistance(qreal value);

    qreal tailAggressiveness() const;
    void setTailAggressiveness(qreal value);

    bool smoothPressure() const;
    void setSmoothPressure(bool value);

    bool useScalableDistance() const;
    void setUseScalableDistance(bool value);

    qreal delayDistance() const;
    void setDelayDistance(qreal value);

    void setUseDelayDistance(bool value);
    bool useDelayDistance() const;

    void setFinishStabilizedCurve(bool value);
    bool finishStabilizedCurve() const;

    void setStabilizeSensors(bool value);
    bool stabilizeSensors() const;

Q_SIGNALS:
    void sigSmoothingTypeChanged();

private Q_SLOTS:
    void slotWriteConfig();

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

typedef QSharedPointer<KisSmoothingOptions> KisSmoothingOptionsSP;

#endif // KIS_SMOOTHING_OPTIONS_H
