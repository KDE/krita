/*
 *  SPDX-FileCopyrightText: 2026 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisChangeImageHdrReferenceWhiteCommand.h"
#include "kis_command_ids.h"
#include <kis_image.h>

KisChangeImageHdrReferenceWhiteCommand::KisChangeImageHdrReferenceWhiteCommand(KisImageWSP image,
                                                                                     std::optional<double> diffuseWhite,
                                                                                     KUndo2Command *parent)
    : KUndo2Command(kundo2_i18n("Set HDR Diffuse White Light Level"), parent)
    , m_image(image)
    , m_diffuseWhite(diffuseWhite)
{
    KisImageSP img = m_image.toStrongRef();
    if (img) {
        m_oldDiffuseWhite = img->hdrReferenceWhiteLightLevel();
    }
}

void KisChangeImageHdrReferenceWhiteCommand::redo()
{
    KisImageSP img = m_image.toStrongRef();
    if (!img) return;

    img->setHdrReferenceWhiteLightLevel(m_diffuseWhite);
}

void KisChangeImageHdrReferenceWhiteCommand::undo()
{
    KisImageSP img = m_image.toStrongRef();
    if (!img) return;

    img->setHdrReferenceWhiteLightLevel(m_oldDiffuseWhite);
}

int KisChangeImageHdrReferenceWhiteCommand::id() const
{
    return KisCommandUtils::ChangeImageHdrDiffuseWhiteId;
}

bool KisChangeImageHdrReferenceWhiteCommand::mergeWith(const KUndo2Command *otherCommand)
{
    const KisChangeImageHdrReferenceWhiteCommand *other = dynamic_cast<const KisChangeImageHdrReferenceWhiteCommand *>(otherCommand);
    if (!other || other->m_image != m_image) return false;

    m_diffuseWhite = other->m_diffuseWhite;
    return true;
}
