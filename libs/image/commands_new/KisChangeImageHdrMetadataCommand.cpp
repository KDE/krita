/*
 *  SPDX-FileCopyrightText: 2026 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisChangeImageHdrMetadataCommand.h"
#include "kis_command_ids.h"
#include <kis_image.h>

KisChangeImageHdrMetadataCommand::KisChangeImageHdrMetadataCommand(
    KisImageWSP image,
    std::optional<double> referenceWhite,
    std::optional<KisRelativeContentLightLevelInformation> clli,
    std::optional<KisColorVolumeInformation> cvi,
    KUndo2Command *parent)
    : KUndo2Command(kundo2_i18n("Set HDR Metadata"), parent)
    , m_image(image)
    , m_referenceWhite(referenceWhite)
    , m_clli(clli)
    , m_cvi(cvi)
{
    KisImageSP img = m_image.toStrongRef();
    if (img) {
        m_oldReferenceWhite = img->hdrReferenceWhiteLightLevel();
        m_oldClli = img->relativeContentLightLevelInformation();
        m_oldCvi = img->colorVolumeInformation();
    }
}

void KisChangeImageHdrMetadataCommand::redo()
{
    KisImageSP img = m_image.toStrongRef();
    if (!img) return;

    img->setHdrReferenceWhiteLightLevel(m_referenceWhite);
    img->setRelativeContentLightLevelInformation(m_clli);
    img->setColorVolumeInformation(m_cvi);
}

void KisChangeImageHdrMetadataCommand::undo()
{
    KisImageSP img = m_image.toStrongRef();
    if (!img) return;

    img->setHdrReferenceWhiteLightLevel(m_oldReferenceWhite);
    img->setRelativeContentLightLevelInformation(m_oldClli);
    img->setColorVolumeInformation(m_oldCvi);
}

int KisChangeImageHdrMetadataCommand::id() const
{
    return KisCommandUtils::ChangeImageHdrMetadataId;
}

bool KisChangeImageHdrMetadataCommand::mergeWith(const KUndo2Command *otherCommand)
{
    const KisChangeImageHdrMetadataCommand *other = dynamic_cast<const KisChangeImageHdrMetadataCommand *>(otherCommand);
    if (!other || other->m_image != m_image) return false;

    m_referenceWhite = other->m_referenceWhite;
    m_clli = other->m_clli;
    m_cvi = other->m_cvi;

    return true;
}
