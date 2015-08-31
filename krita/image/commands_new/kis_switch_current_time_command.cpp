/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_switch_current_time_command.h"

#include "kis_image.h"
#include "kis_image_animation_interface.h"



KisSwitchCurrentTimeCommand::KisSwitchCurrentTimeCommand(KisImageSP image, int newTime, KUndo2Command *parent)
    : KUndo2Command(kundo2_i18n("Switch current time"), parent),
      m_image(image),
      m_oldTime(image->animationInterface()->currentUITime()),
      m_newTime(newTime)
{
}

KisSwitchCurrentTimeCommand::~KisSwitchCurrentTimeCommand()
{
}

int KisSwitchCurrentTimeCommand::id() const
{
    // we don't have a common commands id source in Krita yet, so
    // just use a random one ;)
    return 4 + 8 + 15 + 16 + 23 + 42;
}

bool KisSwitchCurrentTimeCommand::mergeWith(const KUndo2Command* command)
{
    const KisSwitchCurrentTimeCommand *other =
        dynamic_cast<const KisSwitchCurrentTimeCommand*>(command);

    if (!other || other->id() != id()) {
        return false;
    }

    m_oldTime = other->m_oldTime;
    return true;
}

void KisSwitchCurrentTimeCommand::redo()
{
    m_image->animationInterface()->requestTimeSwitchNonGUI(m_newTime);
}

void KisSwitchCurrentTimeCommand::undo()
{
    m_image->animationInterface()->requestTimeSwitchNonGUI(m_oldTime);
}
