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

#include "kis_light_source.h"

// PUBLIC FUNCTIONS

/*
KisLightSource::KisLightSource(QWidget *parent = 0)
            : QLabel(parent)
{
}
*/
//KisLightSource::KisLightSource(QColor color = QColor(0, 0, 0), qreal azimuth = 0, qreal altitude = 90, QWidget *parent = 0)
KisLightSource::KisLightSource(QColor color, qreal azimuth, qreal altitude, QWidget *parent)
            : QLabel(parent)
{
    m_moving = false;

    // Set basic properties
    m_color = color;
    m_azimuth = azimuth;
    m_altitude = altitude;

    // Deduce other properties from the basic set
    qreal m;  //2D vector magnitude
    lightVector.setZ( sin( altitude * M_PI / 180 ) );
    m = cos( altitude * M_PI / 180);

    qDebug() << "m: " << m;
    qDebug() << "orig Altitude: " << altitude;

    // take the negative azimuth to make the spinning counterclockwise
    // TODO BUG make it negative again
    lightVector.setX( cos( azimuth * M_PI / 180 ) * m );
    lightVector.setY( sin( azimuth * M_PI / 180 ) * m );

    // Create background
    m_icondiameter = 13;
    m_selecthalothickness = 3;
    setPixmap(createBackground());
    resize(m_icondiameter, m_icondiameter);

    m_center = QPointF(qreal(size().width()) / 2, qreal(size().height()) / 2);

    RGBvalue << color.redF();
    RGBvalue << color.greenF();
    RGBvalue << color.blueF();

    show();
    if (parentWidget())
        this->parentWidget()->update();

    qDebug() << lightVector;
}

KisLightSource::~KisLightSource()
{
    this->parentWidget()->update();
}


void KisLightSource::mousePressEvent(QMouseEvent* event)
{
    QLabel::mousePressEvent(event);
    dragStartPosition = event->globalPos();
    m_moving = true;

    // #1
    m_selectedPosition = event->posF();
    if (m_isselected)
        m_selecting = false;
    else
        m_selecting = true;
}

void KisLightSource::mouseReleaseEvent(QMouseEvent* event)
{
    QLabel::mouseReleaseEvent(event);

    m_moving = false;

    if (m_selecting && dragStartPosition == event->globalPos()) {
        select();
        m_selecting = false;
    } else if (dragStartPosition == event->globalPos()) {
        deselect();
    }
}

void KisLightSource::mouseMoveEvent(QMouseEvent* event)
{
    m_moving = true;
    qreal newX = parentWidget()->mapFromGlobal(event->globalPos()).x();
    qreal newY = parentWidget()->mapFromGlobal(event->globalPos()).y();
    qreal newRelativeX = newX - qreal(parentWidget()->width() / 2);
    qreal newRelativeY = newY - qreal(parentWidget()->height() / 2);

    //Normalize
    newRelativeX /= (parentWidget()->width() / 2);
    newRelativeY /= (parentWidget()->height() / 2);

    if (!(newRelativeX == 0 && newRelativeY == 0)) {
    // TODO BUG reverse the symbols here later
        m_azimuth = -atan2(newRelativeX, newRelativeY) + M_PI / 2;  // In radians
        m_azimuth *= (180 / M_PI);  // To degrees
    } else {
        m_azimuth = 0;
    }

    //Pitagoras
    qreal newM = sqrt(pow(newRelativeX, 2) + pow(newRelativeY, 2));

    if (newM > 1) {
        // Adjust coordinates to make newM be worth 1
        newRelativeX = cos(m_azimuth * M_PI / 180 );
        newRelativeY = sin(m_azimuth * M_PI / 180 );
        newM = 1;  // 1 is the current result of sqrt(pow(newRelativeX, 2) + pow(newRelativeY, 2));
    }

    m_altitude = acos(newM);  // In radians
    m_altitude *= (180 / M_PI); // To degrees

    qDebug() << "newM: " << newM;
    qDebug() << "new Inclination: " << m_altitude;

    // Set the base vector values
    lightVector.setX( newRelativeX );
    lightVector.setY( newRelativeY );
    lightVector.setZ( sin( m_altitude * M_PI / 180 ) );

    m_moving = false;

    update();

    qDebug() << lightVector;

    emit moved();
    emit azimuthChanged(m_azimuth);
    emit altitudeChanged(m_altitude);
    emit orbitMoved(m_azimuth, m_altitude);
}

void KisLightSource::paintEvent(QPaintEvent* event)
{
    if (!m_moving) {
        QPointF parentCenter(parentWidget()->width(),
                            parentWidget()->height());
        parentCenter /= 2;
        move((1 + x_3D()) * parentCenter.x() - m_center.x(),
            (1 + y_3D()) * parentCenter.y() - m_center.y());
        QLabel::paintEvent(event);
    } else {
        QLabel::paintEvent(event);
    }

    show();
}

bool KisLightSource::isSelected()
{
    return m_isselected;
}

QColor KisLightSource::color()
{
    return m_color;
}

qreal KisLightSource::x_3D()
{
    //qDebug("x_3D = %f", lightVector.x());
    return lightVector.x();
}

qreal KisLightSource::y_3D()
{
    //qDebug("y_3D = %f", lightVector.y());
    return lightVector.y();
}

qreal KisLightSource::z_3D()
{
    return lightVector.z();
}


// PRIVATE FUNCTIONS

QPixmap KisLightSource::createBackground()
{
    QPen pen;
    QBrush brush;
    QPixmap harhar(m_icondiameter, m_icondiameter);
    harhar.fill(QColor(0,0,0,0));

    QPainter painter(&harhar);

    painter.setBackgroundMode(Qt::TransparentMode);
    painter.setRenderHint(QPainter::Antialiasing, 0x08);

    pen.setWidth(1);
    pen.setColor(QColor(0, 0, 0, 150));

    brush.setStyle(Qt::SolidPattern);
    brush.setColor(m_color);

    painter.setBrush(brush);
    painter.setPen(pen);
    painter.drawEllipse(1, 1, m_icondiameter-2, m_icondiameter-2);

    return harhar;
}

QPixmap KisLightSource::createSelectedBackground()
{
    QPen pen;
    QBrush brush;
    QPixmap harhar(m_icondiameter + m_selecthalothickness * 2 + 2,
                   m_icondiameter + m_selecthalothickness * 2 + 2);
    harhar.fill(QColor(0,0,0,0));

    QPainter painter(&harhar);

    painter.setBackgroundMode(Qt::TransparentMode);
    painter.setRenderHint(QPainter::Antialiasing, 0x08);

    pen.setWidth(m_selecthalothickness);
    pen.setColor(QColor(255, 255, 0));

    brush.setStyle(Qt::SolidPattern);
    brush.setColor(m_color);

    painter.setBrush(brush);
    painter.setPen(pen);
    painter.drawEllipse(m_selecthalothickness,
                        m_selecthalothickness,
                        m_icondiameter - m_selecthalothickness,
                        m_icondiameter - m_selecthalothickness);

    return harhar;
}


// SIGNALS

void moved();

void selected();

void deselected();

void orbitMoved(qreal azimuth, qreal altitude);

void azimuthChanged(qreal azimuth);

void altitudeChanged(qreal altitude);

void colorChanged(QColor color);


// SLOTS

void KisLightSource::select()
{
    m_moving = true;
    m_isselected = true;
    setPixmap(createSelectedBackground());
    QSize newsize(m_icondiameter + m_selecthalothickness * 2 + 2,
                  m_icondiameter + m_selecthalothickness * 2 + 2);
    QPoint newtopleft(pos().x() - (m_selecthalothickness - 1),
                      pos().y() - (m_selecthalothickness - 1));

    setGeometry(QRect(newtopleft, newsize));
    m_center = QPointF(qreal(size().width()) / 2, qreal(size().height()) / 2);
    selected();

    setColor(QColor(0, 255, 0));
}

void KisLightSource::deselect()
{
    m_moving = true;
    m_isselected = false;
    setPixmap(createBackground());
    QSize newsize(m_icondiameter, m_icondiameter);
    QPoint newtopleft(pos().x() + (m_selecthalothickness),
                     pos().y() + (m_selecthalothickness));

    setGeometry(QRect(newtopleft, newsize));
        m_center = QPointF(qreal(size().width()) / 2, qreal(size().height()) / 2);
    deselected();
}

void KisLightSource::moveOrbit(qreal azimuth, qreal altitude)
{
    if (m_azimuth != azimuth)
        m_azimuth = azimuth;
    if (m_altitude != altitude)
        m_altitude = altitude;
    emit azimuthChanged(azimuth);
    emit altitudeChanged(altitude);
    emit orbitMoved(azimuth, altitude);
}

void KisLightSource::setAzimuth(qreal azimuth)
{
    if (m_azimuth != azimuth)
        m_azimuth = azimuth;
    emit azimuthChanged(azimuth);
    emit orbitMoved(azimuth, m_altitude);
}

void KisLightSource::setAltitude(qreal altitude)
{
    if (m_altitude != altitude)
        m_altitude = altitude;
    emit altitudeChanged(altitude);
    emit orbitMoved(m_azimuth, altitude);
}

void KisLightSource::setColor(QColor color)
{
    if (m_color != color)
        m_color = color;
    emit colorChanged(color);
}


