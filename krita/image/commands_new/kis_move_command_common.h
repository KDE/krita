/*
 *  Copyright (c) 2007 Boudewijn Rempt <boud@kde.org>
 *  Copyright (c) 2011 Dmitry Kazakov <dimula73@gmail.com>
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
#ifndef KIS_MOVE_COMMAND_COMMON_H
#define KIS_MOVE_COMMAND_COMMON_H

#include <QPoint>
#include <klocalizedstring.h>
#include "kundo2command.h"
#include "kritaimage_export.h"
#include "kis_types.h"

/**
 * KisMoveCommandCommon is a general template for a command that moves
 * entities capable of setX() and setY() actions. Generally in Krita
 * you should now move the device itself, only the node containing
 * that device. But the case of the selections is a bit special, so we
 * move them separately.
 */
template <class ObjectSP>
class KRITAIMAGE_EXPORT KisMoveCommandCommon : public KUndo2Command
{
public:
    KisMoveCommandCommon(ObjectSP object, const QPoint& oldPos, const QPoint& newPos, KUndo2Command *parent = 0)
        : KUndo2Command(kundo2_i18n("Move"), parent),
          m_oldPos(oldPos),
          m_newPos(newPos),
          m_object(object)
    {
    }

    void redo() {
        moveTo(m_newPos);
    }

    void undo() {
        moveTo(m_oldPos);
    }

private:
    void moveTo(const QPoint& pos) {
        /**
         * FIXME: Hack alert:
         * Our iterators don't have guarantees on thread-safety
         * when the offset varies. When it is fixed, remove the locking.
         * see: KisIterator::stressTest(), KisToolMove::mousePressEvent()
         */
        m_object->setX(pos.x());
        m_object->setY(pos.y());
    }

private:
    QPoint m_oldPos;
    QPoint m_newPos;

protected:
    ObjectSP m_object;
};

#endif
