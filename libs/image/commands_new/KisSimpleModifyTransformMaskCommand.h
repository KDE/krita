/*
 *  SPDX-FileCopyrightText: 2021 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISSIMPLEMODIFYTRANSFORMMASKCOMMAND_H
#define KISSIMPLEMODIFYTRANSFORMMASKCOMMAND_H

#include "kritaimage_export.h"
#include "kis_types.h"
#include <boost/none.hpp>

#include <kundo2command.h>


class KRITAIMAGE_EXPORT KisSimpleModifyTransformMaskCommand : public KUndo2Command
{
public:
    KisSimpleModifyTransformMaskCommand(KisTransformMaskSP mask,
                                        KisTransformMaskParamsInterfaceSP newParams,
                                        QWeakPointer<boost::none_t> updatesBlockerCookie = QWeakPointer<boost::none_t>(),
                                        KUndo2Command *parent = nullptr);

    int id() const override;

    bool mergeWith(const KUndo2Command *other) override;

    void undo() override;

    void redo() override;

private:
    bool m_isInitialized {false};

    KisTransformMaskSP m_mask;
    KisTransformMaskParamsInterfaceSP m_oldParams;
    KisTransformMaskParamsInterfaceSP m_newParams;

    QWeakPointer<boost::none_t> m_updatesBlockerCookie;

    std::vector<std::unique_ptr<KUndo2Command>> m_undoCommands;
};

#endif // KISSIMPLEMODIFYTRANSFORMMASKCOMMAND_H
