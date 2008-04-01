/*
 * Copyright (C)  Hans Bakker <hansmbakker@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

void KoCanvasController::mouse3DEvent( Ko3DMouseEvent *event ) {
    if(!event->getX()==0 || !event->getZ()==0)
    {
        QPoint distance = QPoint(event->getX()/10, event->getZ()/10);
        this->pan(distance);
    }
    if(!event->getY()==0)
        emit zoomBy (1.01^(event->getY()/100));
}
