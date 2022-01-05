/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_change_projection_color_command.h"

#include "kis_image.h"
#include "kis_image_animation_interface.h"
#include "kis_command_ids.h"


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
    return KisCommandUtils::ChangeProjectionColorCommand;
}

bool KisChangeProjectionColorCommand::mergeWith(const KUndo2Command* command)
{
    const KisChangeProjectionColorCommand *other =
        dynamic_cast<const KisChangeProjectionColorCommand*>(command);

    if (!other) {
        return false;
    }

    m_newColor = other->m_newColor;
    return true;
}

bool KisChangeProjectionColorCommand::canMergeWith(const KUndo2Command *command) const
{
    const KisChangeProjectionColorCommand *other =
        dynamic_cast<const KisChangeProjectionColorCommand*>(command);

    return other;
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
