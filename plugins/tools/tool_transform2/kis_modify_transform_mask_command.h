/*
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
    KisModifyTransformMaskCommand(KisTransformMaskSP mask, KisTransformMaskParamsInterfaceSP params, bool skipUpdate = false);

    void redo() override;
    void undo() override;

private:
    KisTransformMaskSP m_mask;
    KisTransformMaskParamsInterfaceSP m_params;
    KisTransformMaskParamsInterfaceSP m_oldParams;
    bool m_wasHidden;
    bool m_skipUpdate;
};

#endif
