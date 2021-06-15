/*
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __MODIFY_TRANSFORM_MASK_COMMAND_H
#define __MODIFY_TRANSFORM_MASK_COMMAND_H

#include "kundo2command.h"
#include "kis_types.h"
#include "kritatooltransform_export.h"
#include "KoID.h"
#include <boost/none.hpp>

class KisAnimatedTransformMaskParameters;

class KRITATOOLTRANSFORM_EXPORT KisModifyTransformMaskCommand : public KUndo2Command {
public:
    KisModifyTransformMaskCommand(KisTransformMaskSP mask, KisTransformMaskParamsInterfaceSP params, QWeakPointer<boost::none_t> updatesBlockerCookie = QWeakPointer<boost::none_t>());

    void redo() override;
    void undo() override;

private:
    KisTransformMaskSP m_mask;
    KisTransformMaskParamsInterfaceSP m_params;
    KisTransformMaskParamsInterfaceSP m_oldParams;
    bool m_wasHidden;
    QWeakPointer<boost::none_t> m_updatesBlockerCookie;
};

class KRITATOOLTRANSFORM_EXPORT KisInitializeTransformMaskKeyframesCommand : public KUndo2Command {
public:
    KisInitializeTransformMaskKeyframesCommand(KisTransformMaskSP mask);
};

#endif
