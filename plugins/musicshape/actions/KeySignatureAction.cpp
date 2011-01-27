/* This file is part of the KDE project
 * Copyright 2007 Marijn Kruisselbrink <mkruisselbrink@kde.org>
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
#include "KeySignatureAction.h"

#include "../core/Bar.h"
#include "../core/Staff.h"
#include "../core/Part.h"
#include "../core/Sheet.h"
#include "../core/KeySignature.h"

#include "../commands/SetKeySignatureCommand.h"

#include "../dialogs/KeySignatureDialog.h"

#include "../SimpleEntryTool.h"
#include "../MusicShape.h"

#include <klocale.h>

using namespace MusicCore;

static QString getText(int accidentals)
{
    switch (accidentals) {
        case -4: return i18n("Ab Major");
        case -3: return i18n("Eb Major");
        case -2: return i18n("Bb Major");
        case -1: return i18n("F Major");
        case 0: return i18n("C Major");
        case 1: return i18n("G Major");
        case 2: return i18n("D Major");
        case 3: return i18n("A Major");
        case 4: return i18n("E Major");
    }
    if (accidentals < 0) return i18n("%1 flats", -accidentals);
    else return i18n("%1 sharps", accidentals);
}

KeySignatureAction::KeySignatureAction(SimpleEntryTool* tool, int accidentals)
    : AbstractMusicAction(getText(accidentals), tool), m_accidentals(accidentals), m_showDialog(false)
{
    setCheckable(false);
}

KeySignatureAction::KeySignatureAction(SimpleEntryTool* tool)
: AbstractMusicAction(i18nc("Other key signature", "Other"), tool), m_showDialog(true)
{
    setCheckable(false);
}


void KeySignatureAction::mousePress(Staff* staff, int barIdx, const QPointF& pos)
{
    Q_UNUSED( pos );
    
    if (m_showDialog) {
        KeySignatureDialog dlg;
        dlg.setMusicStyle(m_tool->shape()->style());
        dlg.setBar(barIdx);
        KeySignature* ks = staff->lastKeySignatureChange(barIdx);
        dlg.setAccidentals(ks ? ks->accidentals() : 0);
        if (dlg.exec() == QDialog::Accepted) {
            if (dlg.updateAllStaves()) {
                staff = NULL;
            }
            if (dlg.updateToNextChange() || dlg.updateTillEndOfPiece()) {
                SetKeySignatureCommand::RegionType t = dlg.updateToNextChange() ? SetKeySignatureCommand::NextChange : SetKeySignatureCommand::EndOfPiece;
                m_tool->addCommand(new SetKeySignatureCommand(m_tool->shape(), dlg.startBar(), t, staff, dlg.accidentals()));
            } else {
                m_tool->addCommand(new SetKeySignatureCommand(m_tool->shape(), dlg.startBar(), dlg.endBar(), staff, dlg.accidentals()));
            }
        }
    } else {
        m_tool->addCommand(new SetKeySignatureCommand(m_tool->shape(), barIdx, SetKeySignatureCommand::NextChange, NULL, m_accidentals));
    }
}
