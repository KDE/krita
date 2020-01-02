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

#include "kis_change_projection_color_command.h"

#include "kis_image.h"
#include "kis_image_animation_interface.h"



KisChangeProjectionColorCommand::KisChangeProjectionColorCommand(KisImageSP image, const KoColor &newColor, KUndo2Command *parent)
    : KUndo2Command(kundo2_noi18n("CHANGE_PROJECTION_COLOR_COMMAND"), parent),
      m_image(image),
      m_oldColor(image->defaultProjectionColor()),
      m_newColor(newColor)
{
}

KisChangeProjectionColorCommand::~KisChangeProjectionColorCommand()
{
}

int KisChangeProjectionColorCommand::id() const
{
    // we don't have a common commands id source in Krita yet, so
    // just use a random one ;)

    // https://www.scientificamerican.com/article/most-popular-numbers-grapes-of-math/
    return 142857;
}

bool KisChangeProjectionColorCommand::mergeWith(const KUndo2Command* command)
{
    const KisChangeProjectionColorCommand *other =
        dynamic_cast<const KisChangeProjectionColorCommand*>(command);

    if (!other || other->id() != id()) {
        return false;
    }

    m_newColor = other->m_newColor;
    return true;
}

void KisChangeProjectionColorCommand::redo()
{
    KisImageSP image = m_image.toStrongRef();
    if (!image) {
        return;
    }
    image->setDefaultProjectionColor(m_newColor);
    image->animationInterface()->setDefaultProjectionColor(m_newColor);
}

void KisChangeProjectionColorCommand::undo()
{
    KisImageSP image = m_image.toStrongRef();
    if (!image) {
        return;
    }
    image->setDefaultProjectionColor(m_oldColor);
    image->animationInterface()->setDefaultProjectionColor(m_oldColor);
}
