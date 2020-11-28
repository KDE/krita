/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_ACTIVATE_SELECTION_MASK_COMMAND_H
#define __KIS_ACTIVATE_SELECTION_MASK_COMMAND_H

#include <klocalizedstring.h>
#include "kundo2command.h"
#include "kritaimage_export.h"
#include "kis_types.h"

class KRITAIMAGE_EXPORT KisActivateSelectionMaskCommand : public KUndo2Command
{
public:
    KisActivateSelectionMaskCommand(KisSelectionMaskSP selectionMask, bool value);

    void undo() override;
    void redo() override;

private:
    KisSelectionMaskSP m_selectionMask;
    KisSelectionMaskSP m_previousActiveMask;
    bool m_value;
    bool m_previousValue;
};

#endif /* __KIS_ACTIVATE_SELECTION_MASK_COMMAND_H */
