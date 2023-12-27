/*
 *  SPDX-FileCopyrightText: 2011 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef KisMultiSensorsModel_H_
#define KisMultiSensorsModel_H_

#include <QScopedPointer>
#include <QAbstractListModel>
#include <lager/cursor.hpp>
#include <KoID.h>


class KisCubicCurve;
class KisCurveOption;


class KisMultiSensorsModel : public QAbstractListModel
{
    Q_OBJECT
public:
    using SensorData = std::pair<KoID, bool>;
    using MultiSensorData = std::vector<SensorData>;
public:

    explicit KisMultiSensorsModel(lager::cursor<MultiSensorData> sensorsData,
                                   QObject* parent = 0);

    ~KisMultiSensorsModel() override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    Qt::ItemFlags flags(const QModelIndex & index) const override;

    QString getSensorId(const QModelIndex& index);

    /**
     * Create an index that correspond to the sensor given in argument.
     */
    QModelIndex sensorIndex(const QString &id);

private:
    void slotSensorModelChanged();
private:
    struct Private;
    QScopedPointer<Private> m_d;
};

#endif
