/*
 *  Copyright (c) 2011 José Luis Vergara <pentalis@gmail.com>
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


#ifndef KIS_LIGHT_SOURCE_H
#define KIS_LIGHT_SOURCE_H

#include <QWidget>
#include <QLabel>
#include <QtGui>
#include <QVector3D>
#include <cmath>
#include <krita_export.h>

class KRITAUI_EXPORT KisLightSource : public QLabel
{
    Q_OBJECT

public:
    //explicit KisLightSource(QWidget *parent);
    //explicit KisLightSource(QColor color, qreal azimuth, qreal inclination, QWidget *parent);
    explicit KisLightSource(QColor color = QColor(0, 0, 0, 255), qreal azimuth = 0, qreal inclination = 90, QWidget *parent = 0);
    virtual ~KisLightSource();

    virtual void mousePressEvent(QMouseEvent * event);
    virtual void mouseReleaseEvent(QMouseEvent * event);
    virtual void mouseMoveEvent(QMouseEvent * event);
    virtual void paintEvent(QPaintEvent * event);

    bool isSelected();
    QColor color();

    qreal x_3D();
    qreal y_3D();
    qreal z_3D();

    QList<qreal> RGBvalue;
    QVector3D lightVector;

signals:
    void moved();
    void selected();
    void deselected();
    void orbitMoved(qreal azimuth, qreal altitude);
    void azimuthChanged(qreal azimuth);
    void altitudeChanged(qreal altitude);
    void colorChanged(QColor color);

public slots:
    void select();
    void deselect();
    void moveOrbit(qreal azimuth, qreal altitude);
    void setAzimuth(qreal azimuth);
    void setAltitude(qreal altitude);
    void setColor(QColor color);

private:
    bool m_moving;
    bool m_selecting;
    bool m_isselected;
    quint16 m_icondiameter;
    quint16 m_selecthalothickness;
    qreal m_azimuth;
    qreal m_altitude;
    QPoint dragStartPosition;
    QPointF m_selectedPosition;
    QPointF m_center;
    QColor m_color;
    QPixmap createBackground();
    QPixmap createSelectedBackground();
};

#endif // KIS_LIGHT_SOURCE_H
