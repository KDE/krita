/*
 *  SPDX-FileCopyrightText: 2021 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISSIMPLEMODIFYTRANSFORMMASKCOMMAND_H
#define KISSIMPLEMODIFYTRANSFORMMASKCOMMAND_H

#include "kritaimage_export.h"
#include "kis_types.h"

#include <kundo2command.h>


class KRITAIMAGE_EXPORT KisSimpleModifyTransformMaskCommand : public KUndo2Command
{
public:
    KisSimpleModifyTransformMaskCommand(KisTransformMaskSP mask,
                                        KisTransformMaskParamsInterfaceSP oldParams,
                                        KisTransformMaskParamsInterfaceSP newParams);

    int id() const override;

    bool mergeWith(const KUndo2Command *other) override;

    void undo() override;

    void redo() override;

private:
    KisTransformMaskSP m_mask;
    KisTransformMaskParamsInterfaceSP m_oldParams;
    KisTransformMaskParamsInterfaceSP m_newParams;
};

#endif // KISSIMPLEMODIFYTRANSFORMMASKCOMMAND_H
