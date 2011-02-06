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

#ifndef KISMULTISENSORSMODEL_H_
#define KISMULTISENSORSMODEL_H_

#include <QAbstractListModel>

class KisDynamicSensorList;
class KisDynamicSensor;

class KisMultiSensorsModel : public QAbstractListModel {
    Q_OBJECT
public:
    explicit KisMultiSensorsModel(QObject* parent = 0);
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    virtual Qt::ItemFlags flags( const QModelIndex & index ) const;
    void setCurrentSensor(KisDynamicSensor* sensor);
    QWidget* createConfigurationWidget(const QModelIndex& index, QWidget* parent, QWidget* selector);
signals:
    void sensorChanged(KisDynamicSensor* sensor);
    /**
     * This signal is emitted when the parameters of sensor are changed.
     */
    void parametersChanged();    
private:
    KisDynamicSensor* m_currentSensor;
    KisDynamicSensorList* m_listSensor;
    
};

#endif
