/*
 *  SPDX-FileCopyrightText: 2026 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISCHANGEIMAGEHDRMETADATACOMMAND_H
#define KISCHANGEIMAGEHDRMETADATACOMMAND_H

#include <kundo2command.h>
#include <kis_types.h>
#include <kis_hdr_metadata.h>
#include "kritaimage_export.h"

class KRITAIMAGE_EXPORT KisChangeImageHdrContentLightLevelCommand : public KUndo2Command
{
public:
    KisChangeImageHdrContentLightLevelCommand(KisImageWSP image,
                                     std::optional<KisRelativeContentLightLevelInformation> clli,
                                     KUndo2Command *parent = nullptr);

    void redo() override;
    void undo() override;

    int id() const override;
    bool mergeWith(const KUndo2Command *other) override;
private:
    KisImageWSP m_image;
    std::optional<KisRelativeContentLightLevelInformation> m_clli;
    std::optional<KisRelativeContentLightLevelInformation> m_oldClli;
};

#endif // KISCHANGEIMAGEHDRMETADATACOMMAND_H
