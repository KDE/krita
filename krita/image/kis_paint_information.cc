/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
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

#include "kis_paint_information.h"

#include <QDomElement>

struct KisPaintInformation::Private {
    QPointF pos;
    double pressure;
    double xTilt;
    double yTilt;
    KisVector2D movement;
};

KisPaintInformation::KisPaintInformation(const QPointF & pos, double pressure,
                        double xTilt, double yTilt,
                        KisVector2D movement)
    : d(new Private)
{
    d->pos = pos;
    d->pressure = pressure;
    d->xTilt = xTilt;
    d->yTilt = yTilt;
    d->movement = movement;
}

KisPaintInformation::KisPaintInformation(const KisPaintInformation& rhs) : d(new Private(*rhs.d))
{
}

void KisPaintInformation::operator=(const KisPaintInformation& rhs)
{
    *d = *rhs.d;
}

KisPaintInformation::~KisPaintInformation()
{
    delete d;
}


void KisPaintInformation::toXML(QDomDocument&, QDomElement& e) const
{
    e.setAttribute("pointX", pos().x());
    e.setAttribute("pointY", pos().y());
    e.setAttribute("pressure", pressure());
    e.setAttribute("xTilt", xTilt());
    e.setAttribute("yTilt", yTilt());
    e.setAttribute("movementX", movement().x());
    e.setAttribute("movementY", movement().y());
}

KisPaintInformation KisPaintInformation::fromXML(QDomElement& e)
{
    double pointX = e.attribute("pointX","0.0").toDouble();
    double pointY = e.attribute("pointY","0.0").toDouble();
    double pressure = e.attribute("pressure","0.0").toDouble();
    double xTilt = e.attribute("xTilt","0.0").toDouble();
    double yTilt = e.attribute("yTilt","0.0").toDouble();
    double movementX = e.attribute("movementX","0.0").toDouble();
    double movementY = e.attribute("movementY","0.0").toDouble();
    return KisPaintInformation(QPointF(pointX, pointY), pressure, xTilt, yTilt, KisVector2D(movementX, movementY));
}

QPointF KisPaintInformation::pos() const
{
    return d->pos;
}

void KisPaintInformation::setPos(const QPointF& p)
{
    d->pos = p;
}

double KisPaintInformation::pressure() const
{
    return d->pressure;
}

void KisPaintInformation::setPressure(double p)
{
    d->pressure = p;
}

double KisPaintInformation::xTilt() const
{
    return d->xTilt;
}

double KisPaintInformation::yTilt() const
{
    return d->yTilt;
}

KisVector2D KisPaintInformation::movement() const
{
    return d->movement;
}
