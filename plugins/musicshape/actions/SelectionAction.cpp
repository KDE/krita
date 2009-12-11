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
#include "SelectionAction.h"

#include "../SimpleEntryTool.h"
#include "../MusicShape.h"
#include "../Renderer.h"

#include "../core/Chord.h"
#include "../core/Note.h"
#include "../core/VoiceBar.h"
#include "../core/Clef.h"
#include "../core/Voice.h"
#include "../core/Sheet.h"
#include "../core/Bar.h"
#include "../core/Part.h"
#include "../core/Staff.h"

#include <kicon.h>
#include <kdebug.h>
#include <klocale.h>

#include <math.h>

using namespace MusicCore;

SelectionAction::SelectionAction(SimpleEntryTool* tool)
    : AbstractMusicAction(KIcon("select"), i18n("Select"), tool)
{
    m_firstBar = -1;
}

inline static qreal sqr(qreal a) { return a*a; }

void SelectionAction::mousePress(Staff* staff, int barIdx, const QPointF& pos)
{
    Q_UNUSED(pos);
    /*Part* part = staff->part();
    Sheet* sheet = part->sheet();
    Bar* bar = sheet->bar(barIdx);
    
    // loop over all chords
    qreal closestDist = 1e9;
    Chord* chord = 0;
    
    // outer loop, loop over all voices
    for (int v = 0; v < part->voiceCount(); v++) {
        Voice* voice = part->voice(v);
        VoiceBar* vb = voice->bar(bar);
        
        // next loop over all chords
        for (int e = 0; e < vb->elementCount(); e++) {
            Chord* c = dynamic_cast<Chord*>(vb->element(e));
            if (!c) continue;
            
            qreal centerX = c->x() + (c->width() / 2);
            qreal centerY = c->y() + (c->height() / 2);
            qreal dist = sqrt(sqr(centerX - pos.x()) + sqr(centerY - pos.y()));
            if (dist < closestDist) {
                closestDist = dist;
                chord = c;
            }
        }
    }*/
    
    m_firstBar = barIdx;
    m_startStaff = staff;
    m_tool->setSelection(barIdx, barIdx, staff, staff);
}

void SelectionAction::mouseMove(Staff* staff, int barIdx, const QPointF& pos)
{
    Q_UNUSED(pos);
    m_tool->setSelection(qMin(m_firstBar, barIdx), qMax(m_firstBar, barIdx), m_startStaff, staff);
}
