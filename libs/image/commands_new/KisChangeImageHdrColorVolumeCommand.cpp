/*
 *  SPDX-FileCopyrightText: 2026 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisChangeImageHdrColorVolumeCommand.h"
#include "kis_command_ids.h"
#include <kis_image.h>

KisChangeImageHdrColorVolumeCommand::KisChangeImageHdrColorVolumeCommand(KisImageWSP image,
                                                                         std::optional<KisColorVolumeInformation> cvi,
                                                                         KUndo2Command *parent)
    : KUndo2Command(kundo2_i18n("Set HDR Color Volume Information"), parent)
    , m_image(image)
    , m_cvi(cvi)
{
    KisImageSP img = m_image.toStrongRef();
    if (img) {
        m_oldCvi = img->colorVolumeInformation();
    }
}

void KisChangeImageHdrColorVolumeCommand::redo()
{
    KisImageSP img = m_image.toStrongRef();
    if (!img) return;

        img->setColorVolumeInformation(m_cvi);

}

void KisChangeImageHdrColorVolumeCommand::undo()
{
    KisImageSP img = m_image.toStrongRef();
    if (!img) return;

    img->setColorVolumeInformation(m_oldCvi);
}

int KisChangeImageHdrColorVolumeCommand::id() const
{
    return KisCommandUtils::ChangeImageHdrColorVolumeId;
}

bool KisChangeImageHdrColorVolumeCommand::mergeWith(const KUndo2Command *otherCommand)
{
    const KisChangeImageHdrColorVolumeCommand *other = dynamic_cast<const KisChangeImageHdrColorVolumeCommand *>(otherCommand);
    if (!other || other->m_image != m_image) return false;

    m_cvi = other->m_cvi;
    return true;
}
