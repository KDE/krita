/*
 *  SPDX-FileCopyrightText: 2023 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISLAZYCREATETRANSFORMMASKKEYFRAMESCOMMAND_H
#define KISLAZYCREATETRANSFORMMASKKEYFRAMESCOMMAND_H

#include "kis_types.h"
#include "kis_command_utils.h"

class KRITAIMAGE_EXPORT KisLazyCreateTransformMaskKeyframesCommand : public KisCommandUtils::AggregateCommand {
public:
    KisLazyCreateTransformMaskKeyframesCommand(KisTransformMaskSP mask, KUndo2Command *parent = nullptr);

    static bool maskHasAnimation(KisTransformMaskSP mask);

private:
    void populateChildCommands() override;

private:
    KisTransformMaskSP m_mask;
};

#endif // KISLAZYCREATETRANSFORMMASKKEYFRAMESCOMMAND_H
