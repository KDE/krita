/*
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

#ifndef __MODIFY_TRANSFORM_MASK_COMMAND_H
#define __MODIFY_TRANSFORM_MASK_COMMAND_H

#include "kundo2command.h"
#include "kis_types.h"
#include "kritatooltransform_export.h"
#include "KoID.h"

class KisAnimatedTransformMaskParameters;

class KRITATOOLTRANSFORM_EXPORT KisModifyTransformMaskCommand : public KUndo2Command {
public:
    KisModifyTransformMaskCommand(KisTransformMaskSP mask, KisTransformMaskParamsInterfaceSP params);

    void redo() override;
    void undo() override;

private:
    KisTransformMaskSP m_mask;
    KisTransformMaskParamsInterfaceSP m_params;
    KisTransformMaskParamsInterfaceSP m_oldParams;
    bool m_wasHidden;
};

#endif
