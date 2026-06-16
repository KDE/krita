/*
 *  SPDX-FileCopyrightText: 2026 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisChangeImageHdrContentLightLevelCommand.h"
#include "kis_command_ids.h"
#include <kis_image.h>

KisChangeImageHdrContentLightLevelCommand::KisChangeImageHdrContentLightLevelCommand(KisImageWSP image,
                                                                   std::optional<KisRelativeContentLightLevelInformation> clli,
                                                                   KUndo2Command *parent)
    : KUndo2Command(kundo2_i18n("Set HDR Content Light Level"), parent)
    , m_image(image)
    , m_clli(clli)
{
    KisImageSP img = m_image.toStrongRef();
    if (img) {
        m_oldClli = img->relativeContentLightLevelInformation();
    }
}

void KisChangeImageHdrContentLightLevelCommand::redo()
{
    KisImageSP img = m_image.toStrongRef();
    if (!img) return;

    if (img->relativeContentLightLevelInformation() != m_clli) {
        img->setRelativeContentLightLevelInformation(m_clli);
    }
}

void KisChangeImageHdrContentLightLevelCommand::undo()
{
    KisImageSP img = m_image.toStrongRef();
    if (!img) return;

    img->setRelativeContentLightLevelInformation(m_oldClli);
}

int KisChangeImageHdrContentLightLevelCommand::id() const
{
    return KisCommandUtils::ChangeImageHdrContentLightLevelId;
}

bool KisChangeImageHdrContentLightLevelCommand::mergeWith(const KUndo2Command *otherCommand)
{
    const KisChangeImageHdrContentLightLevelCommand *other = dynamic_cast<const KisChangeImageHdrContentLightLevelCommand *>(otherCommand);
    if (!other || other->m_image != m_image) return false;

    m_clli = other->m_clli;
    return true;
}
