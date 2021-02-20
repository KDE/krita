/*
 *  SPDX-FileCopyrightText: 2011 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */
#ifndef KIS_MULTI_SENSORS_SELECTOR_H
#define KIS_MULTI_SENSORS_SELECTOR_H
#include <QWidget>

class KisCubicCurve;
class QModelIndex;
class KisCurveOption;

#include <kis_dynamic_sensor.h>

class KisMultiSensorsSelector : public QWidget
{
    Q_OBJECT
public:

    KisMultiSensorsSelector(QWidget* parent);
    ~KisMultiSensorsSelector() override;

    void setCurveOption(KisCurveOption *curveOption);
    void setCurrent(KisDynamicSensorSP _sensor);
    KisDynamicSensorSP currentHighlighted();
    void setCurrentCurve(const KisCubicCurve& curve, bool useSameCurve);
    void reload();

private Q_SLOTS:

    void sensorActivated(const QModelIndex& index);
    void setCurrent(const QModelIndex& index);

Q_SIGNALS:

    void sensorChanged(KisDynamicSensorSP sensor);

    /**
     * This signal is emitted when the parameters of sensor are changed.
     */
    void parametersChanged();

    void highlightedSensorChanged(KisDynamicSensorSP sensor);
private:
    struct Private;
    Private* const d;
};
#endif
