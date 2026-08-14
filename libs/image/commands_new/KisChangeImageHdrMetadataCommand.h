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
#include <optional>

class KRITAIMAGE_EXPORT KisChangeImageHdrMetadataCommand : public KUndo2Command
{
public:
    KisChangeImageHdrMetadataCommand(KisImageWSP image,
                                        std::optional<double> referenceWhite,
                                        std::optional<KisRelativeContentLightLevelInformation> clli,
                                        std::optional<KisColorVolumeInformation> cvi,
                                        KUndo2Command *parent = nullptr);

    void redo() override;
    void undo() override;

    int id() const override;
    bool mergeWith(const KUndo2Command *other) override;
private:
    KisImageWSP m_image;

    std::optional<double> m_referenceWhite;
    std::optional<KisRelativeContentLightLevelInformation> m_clli;
    std::optional<KisColorVolumeInformation> m_cvi;

    std::optional<double> m_oldReferenceWhite;
    std::optional<KisRelativeContentLightLevelInformation> m_oldClli;
    std::optional<KisColorVolumeInformation> m_oldCvi;
};

#endif // KISCHANGEIMAGEHDRMETADATACOMMAND_H
