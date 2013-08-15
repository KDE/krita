/*
 *  Copyright (c) 2011 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 or later of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
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
class KisDynamicSensor;

class KisMultiSensorsSelector : public QWidget
{
    Q_OBJECT
public:
    KisMultiSensorsSelector(QWidget* parent);
    ~KisMultiSensorsSelector();
    void setCurrent(KisDynamicSensor* _sensor);
    KisDynamicSensor* current();
    KisDynamicSensor* currentHighlighted();
    void setCurrentCurve(const KisCubicCurve& curve, bool useSameCurve);
private slots:
    void sensorActivated(const QModelIndex& index);
signals:
    void sensorChanged(KisDynamicSensor* sensor);
    /**
     * This signal is emitted when the parameters of sensor are changed.
     */
    void parametersChanged();
    void highlightedSensorChanged(KisDynamicSensor* sensor);
private:
    struct Private;
    Private* const d;
};
#endif
