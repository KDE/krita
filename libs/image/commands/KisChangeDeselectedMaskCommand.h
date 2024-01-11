/*
 *  SPDX-FileCopyrightText: 2024 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISCAHANGEDESELECTEDMASKCOMMAND_H
#define KISCAHANGEDESELECTEDMASKCOMMAND_H

#include "kritaimage_export.h"
#include <kis_types.h>
#include <kundo2command.h>


class KRITAIMAGE_EXPORT KisChangeDeselectedMaskCommand : public KUndo2Command
{
public:
    KisChangeDeselectedMaskCommand(KisImageWSP image);

    KisChangeDeselectedMaskCommand(KisImageWSP image,
                                   KisSelectionMaskSP newDeselectedMask);

    void undo();
    void redo();

private:
    KisImageWSP m_image;
    KisSelectionMaskSP m_newDeselectedMask;
    KisSelectionMaskSP m_oldDeselectedMask;
};

#endif // KISCAHANGEDESELECTEDMASKCOMMAND_H
