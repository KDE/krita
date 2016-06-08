/*
 *  Copyright (c) 2013 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_tablet_event.h"

KisTabletEvent::KisTabletEvent(ExtraEventType type, const QPoint &pos, const QPoint &globalPos,
                               const QPointF &hiResGlobalPos, int device, int pointerType,
                               qreal pressure, int xTilt, int yTilt, qreal tangentialPressure,
                               qreal rotation, int z, Qt::KeyboardModifiers keyState, qint64 uniqueID,
                               Qt::MouseButton button, Qt::MouseButtons buttons)
    : QInputEvent((Type)(type), keyState),
      mPos(pos),
      mGPos(globalPos),
      mHiResGlobalPos(hiResGlobalPos),
      mDev(device),
      mPointerType(pointerType),
      mXT(xTilt),
      mYT(yTilt),
      mZ(z),
      mPress(pressure),
      mTangential(tangentialPressure),
      mRot(rotation),
      mUnique(uniqueID),
      mouseButton(button),
      mouseButtons(buttons)
{
}


KisTabletEvent::~KisTabletEvent()
{
}
