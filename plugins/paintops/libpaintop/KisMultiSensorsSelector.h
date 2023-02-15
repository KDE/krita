/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KisMultiSensorsSelector_H
#define KisMultiSensorsSelector_H

class KisCubicCurve;
class QModelIndex;
class KisCurveOption;

#include <QWidget>
#include <KisCurveOptionData.h>
#include <lager/cursor.hpp>

class KisMultiSensorsSelector : public QWidget
{
    Q_OBJECT
public:

    KisMultiSensorsSelector(QWidget* parent);
    ~KisMultiSensorsSelector() override;

    void setOptionDataCursor(lager::cursor<KisCurveOptionDataCommon> optionData);

    void setCurrent(const QString &id);
    QString currentHighlighted();

private Q_SLOTS:

    void sensorActivated(const QModelIndex& index);
    void setCurrent(const QModelIndex& index);

Q_SIGNALS:
    void highlightedSensorChanged(const QString &id);

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    struct Private;
    Private* const d;
};

#endif // KisMultiSensorsSelector_H
