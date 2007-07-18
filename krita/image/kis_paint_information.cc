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


void KisPaintInformation::toXML(QDomDocument&, QDomElement& e)
{
    e.setAttribute("pointX", pos.x());
    e.setAttribute("pointY", pos.y());
    e.setAttribute("pressure", pressure);
    e.setAttribute("xTilt", xTilt);
    e.setAttribute("yTilt", yTilt);
    e.setAttribute("movementX", movement.x());
    e.setAttribute("movementY", movement.y());
}
