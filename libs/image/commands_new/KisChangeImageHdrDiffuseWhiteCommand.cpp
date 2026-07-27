/*
 *  SPDX-FileCopyrightText: 2026 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisChangeImageHdrDiffuseWhiteCommand.h"
#include "kis_command_ids.h"
#include <kis_image.h>

KisChangeImageHdrDiffuseWhiteCommand::KisChangeImageHdrDiffuseWhiteCommand(KisImageWSP image,
                                                                                     std::optional<double> diffuseWhite,
                                                                                     KUndo2Command *parent)
    : KUndo2Command(kundo2_i18n("Set HDR Diffuse White Light Level"), parent)
    , m_image(image)
    , m_diffuseWhite(diffuseWhite)
{
    KisImageSP img = m_image.toStrongRef();
    if (img) {
        m_oldDiffuseWhite = img->diffuseWhiteLightLevel();
    }
}

void KisChangeImageHdrDiffuseWhiteCommand::redo()
{
    KisImageSP img = m_image.toStrongRef();
    if (!img) return;

    img->setDiffuseWhiteLightLevel(m_diffuseWhite);
}

void KisChangeImageHdrDiffuseWhiteCommand::undo()
{
    KisImageSP img = m_image.toStrongRef();
    if (!img) return;

    img->setDiffuseWhiteLightLevel(m_oldDiffuseWhite);
}

int KisChangeImageHdrDiffuseWhiteCommand::id() const
{
    return KisCommandUtils::ChangeImageHdrDiffuseWhiteId;
}

bool KisChangeImageHdrDiffuseWhiteCommand::mergeWith(const KUndo2Command *otherCommand)
{
    const KisChangeImageHdrDiffuseWhiteCommand *other = dynamic_cast<const KisChangeImageHdrDiffuseWhiteCommand *>(otherCommand);
    if (!other || other->m_image != m_image) return false;

    m_diffuseWhite = other->m_diffuseWhite;
    return true;
}
