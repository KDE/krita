/*
 *  Copyright (c) 2011 Cyrille Berger <cberger@cberger.net>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2.1 of the License, or
 *  (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
    void slotSensorEnableChange(bool enabled);

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
