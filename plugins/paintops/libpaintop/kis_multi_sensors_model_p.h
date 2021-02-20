/*
 *  SPDX-FileCopyrightText: 2011 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef KISMULTISENSORSMODEL_H_
#define KISMULTISENSORSMODEL_H_

#include <QAbstractListModel>
#include <kis_dynamic_sensor.h>

class KisCubicCurve;
class KisCurveOption;


class KisMultiSensorsModel : public QAbstractListModel
{
    Q_OBJECT
public:

    explicit KisMultiSensorsModel(QObject* parent = 0);

    ~KisMultiSensorsModel() override;

    void setCurveOption(KisCurveOption *curveOption);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    Qt::ItemFlags flags(const QModelIndex & index) const override;

    KisDynamicSensorSP getSensor(const QModelIndex& index);

    void setCurrentCurve(const QModelIndex& currentIndex, const KisCubicCurve& curve, bool useSameCurve);

    /**
     * Create an index that correspond to the sensor given in argument.
     */
    QModelIndex sensorIndex(KisDynamicSensorSP arg1);

    void resetCurveOption();

Q_SIGNALS:

    void sensorChanged(KisDynamicSensorSP sensor);

    /**
     * This signal is emitted when the parameters of sensor are changed.
     */
    void parametersChanged();

private:

    KisCurveOption *m_curveOption;
};

#endif
