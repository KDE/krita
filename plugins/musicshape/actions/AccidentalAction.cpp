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
#include "AccidentalAction.h"

#include "../SimpleEntryTool.h"
#include "../MusicShape.h"
#include "../Renderer.h"

#include "../core/Staff.h"
#include "../core/Part.h"
#include "../core/Sheet.h"
#include "../core/Bar.h"
#include "../core/VoiceBar.h"
#include "../core/Chord.h"
#include "../core/Note.h"
#include "../core/Voice.h"
#include "../core/Clef.h"

#include "../commands/SetAccidentalsCommand.h"

#include <KoIcon.h>

#include <kdebug.h>
#include <klocale.h>

#include <math.h>

using namespace MusicCore;

static KIcon getIcon(int accidentals)
{
    static const char *const iconNames[5] =
    {
        koIconNameCStr("music-doubleflat"),
        koIconNameCStr("music-flat"),
        koIconNameCStr("music-natural"),
        koIconNameCStr("music-cross"),
        koIconNameCStr("music-doublecross")
    };

    if ((-2 <= accidentals) && (accidentals <= 2)) {
        return KIcon(QLatin1String(iconNames[accidentals+2]));
    }

    return KIcon();
}

static QString getText(int accidentals)
{
    switch (accidentals) {
        case -2: return i18n("Double flat");
        case -1: return i18nc("lowered half a step", "Flat");
        case 0:  return i18n("Natural");
        case 1:  return i18nc("raised half a step", "Sharp");
        case 2:  return i18n("Double sharp");
    }
    if (accidentals < 0) {
        return i18n("%1 flats", -accidentals);
    } else {
        return i18n("%1 sharps", accidentals);
    }
}

AccidentalAction::AccidentalAction(int accidentals, SimpleEntryTool* tool)
    : AbstractNoteMusicAction(getIcon(accidentals), getText(accidentals), tool)
    , m_accidentals(accidentals)
{
}

void AccidentalAction::renderPreview(QPainter& painter, const QPointF& point)
{
    m_tool->shape()->renderer()->renderAccidental(painter, m_accidentals, point, Qt::gray);
}

void AccidentalAction::mousePress(Chord* chord, Note* note, qreal distance, const QPointF& pos)
{
    Q_UNUSED( chord );
    Q_UNUSED( pos );

    if (!note) return;
    if (distance > 15) return; // bah, magic numbers are ugly....

    m_tool->addCommand(new SetAccidentalsCommand(m_tool->shape(), note, m_accidentals));
}
