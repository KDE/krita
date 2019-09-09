/*
 *  Copyright (c) 2019 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef KIS_PRESSURE_CALLIBRATION_HELPER_H
#define KIS_PRESSURE_CALLIBRATION_HELPER_H

#include <QWidget>
#include <kritaui_export.h>
#include <QTabletEvent>
#include <QPolygon>
#include <QTimer>
/**
 * @brief The KisPressureCallibrationHelper class
 * This class helps users configure the tablet pressure.
 */

class KRITAUI_EXPORT KisPressureCallibrationHelper : public QWidget
{
    Q_OBJECT
public:
    explicit KisPressureCallibrationHelper(QWidget *parent = nullptr);

    ~KisPressureCallibrationHelper() override;

    /**
     * @brief callibrationInfo
     * @return QVector<float> with values for lowest pressure, middle pressure and highest presure.
     */
    QList<QPointF> callibrationInfo();

    enum progressState {
        WAITING = 0,
        MEDIUM_STROKE,
        HEAVY_STROKE,
        LIGHT_STROKE,
        DONE
    };

Q_SIGNALS:
    void callibrationDone();

protected:
    void paintEvent(QPaintEvent *e) override;
    void tabletEvent(QTabletEvent *e) override;

private:

    void UpdateCaption();

    QList<qreal> m_callibrationInfo;
    QPolygon m_currentPath;
    QTimer *m_callibrationTime;
    QString m_caption;
    int m_oldCaption;

};

#endif /* KIS_PRESSURE_CALLIBRATION_HELPER_H */
