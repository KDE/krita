/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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

    void undo();
    void redo();

private:
    KisSelectionMaskSP m_selectionMask;
    KisSelectionMaskSP m_previousActiveMask;
    bool m_value;
    bool m_previousValue;
};

#endif /* __KIS_ACTIVATE_SELECTION_MASK_COMMAND_H */
