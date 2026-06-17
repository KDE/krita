/*
 *  SPDX-FileCopyrightText: 2026 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISCHANGEIMAGEHDRCOLORVOLUMECOMMAND_H
#define KISCHANGEIMAGEHDRCOLORVOLUMECOMMAND_H

#include <kundo2command.h>
#include <kis_types.h>
#include <kis_hdr_metadata.h>
#include "kritaimage_export.h"
#include <optional>

class KRITAIMAGE_EXPORT KisChangeImageHdrColorVolumeCommand : public KUndo2Command
{
public:
    KisChangeImageHdrColorVolumeCommand(KisImageWSP image,
                                              std::optional<KisColorVolumeInformation> cvi,
                                              KUndo2Command *parent = nullptr);

    void redo() override;
    void undo() override;

    int id() const override;
    bool mergeWith(const KUndo2Command *other) override;
private:
    KisImageWSP m_image;
    std::optional<KisColorVolumeInformation> m_cvi;
    std::optional<KisColorVolumeInformation> m_oldCvi;
};

#endif // KISCHANGEIMAGEHDRMETADATACOMMAND_H
