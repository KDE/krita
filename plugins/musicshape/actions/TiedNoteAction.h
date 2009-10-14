/* This file is part of the KDE project
 * Copyright 2007 Marijn Kruisselbrink <m.Kruisselbrink@student.tue.nl>
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
#ifndef TIEDNOTEACTION_H
#define TIEDNOTEACTION_H

#include "AbstractNoteMusicAction.h"

class TiedNoteAction : public AbstractNoteMusicAction
{
public:
    TiedNoteAction(SimpleEntryTool* tool);
    
    virtual void mousePress(MusicCore::Chord* chord, MusicCore::Note* note, qreal distance, const QPointF& pos);

    //! To avoid 'mousePress() was hidden' warning
    virtual void mousePress(MusicCore::Staff* staff, int bar, const QPointF& pos) {
        AbstractNoteMusicAction::mousePress(staff, bar, pos);
    }
    //! To avoid 'mousePress() was hidden' warning
    virtual void mousePress(MusicCore::StaffElement* se, qreal distance, const QPointF& pos) {
        AbstractNoteMusicAction::mousePress(se, distance, pos);
    }
};

#endif // ERASERACTION_H

