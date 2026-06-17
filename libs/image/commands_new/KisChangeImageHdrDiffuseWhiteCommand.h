/*
 *  SPDX-FileCopyrightText: 2026 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISCHANGEIMAGEHDRDIFFUSEWHITECOMMAND_H
#define KISCHANGEIMAGEHDRDIFFUSEWHITECOMMAND_H

#include <kundo2command.h>
#include <kis_types.h>
#include "kritaimage_export.h"
#include <optional>

class KRITAIMAGE_EXPORT KisChangeImageHdrDiffuseWhiteCommand : public KUndo2Command
{
public:
    KisChangeImageHdrDiffuseWhiteCommand(KisImageWSP image,
                                              std::optional<double> diffuseWhite,
                                              KUndo2Command *parent = nullptr);

    void redo() override;
    void undo() override;

    int id() const override;
    bool mergeWith(const KUndo2Command *other) override;
private:
    KisImageWSP m_image;
    std::optional<double> m_diffuseWhite;
    std::optional<double> m_oldDiffuseWhite;
};

#endif // KISCHANGEIMAGEHDRDIFFUSEWHITECOMMAND_H
