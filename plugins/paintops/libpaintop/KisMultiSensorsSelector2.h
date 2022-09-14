/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISMULTISENSORSSELECTOR2_H
#define KISMULTISENSORSSELECTOR2_H

class KisCubicCurve;
class QModelIndex;
class KisCurveOption;

#include <QWidget>
#include <KisCurveOptionData.h>
#include <lager/cursor.hpp>

class KisMultiSensorsSelector2 : public QWidget
{
    Q_OBJECT
public:

    KisMultiSensorsSelector2(QWidget* parent);
    ~KisMultiSensorsSelector2() override;

    void setOptionDataCursor(lager::cursor<KisCurveOptionData> optionData);

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

#endif // KISMULTISENSORSSELECTOR2_H
