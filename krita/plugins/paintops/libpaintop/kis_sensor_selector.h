/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
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

#ifndef _KIS_SENSOR_SELECTOR_H_
#define _KIS_SENSOR_SELECTOR_H_

#include <QWidget>

#include <krita_export.h>

class Ui_SensorSelector;
class KisDynamicSensor;
class QHBoxLayout;
class KoID;

class PAINTOP_EXPORT KisSensorSelector : public QWidget
{
    Q_OBJECT
public:
    KisSensorSelector(QWidget* parent);
public:
    void setCurrent(KisDynamicSensor* sensor);
    KisDynamicSensor* current();
signals:
    void sensorChanged(KisDynamicSensor* sensor);
    /**
     * This signal is emitted when the parameters of sensor are changed.
     */
    void parametersChanged();
private slots:
    void setSensorId(const KoID& id);
private:
    Ui_SensorSelector* sensorSelectorUI;
    QHBoxLayout* m_layout;
    QWidget* m_currentConfigWidget;
    KisDynamicSensor* m_currentSensor;
};

#endif
