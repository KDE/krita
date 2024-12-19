/*
 *  SPDX-FileCopyrightText: 2024 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */


#ifndef KISLAYERCOLLAPSECOMMAND_H
#define KISLAYERCOLLAPSECOMMAND_H

#include <kritaimage_export.h>
#include <kundo2command.h>
#include <kis_types.h>


class KRITAIMAGE_EXPORT KisLayerCollapseCommand : public KUndo2Command
{
public:
    KisLayerCollapseCommand(KisNodeSP node, bool oldValue, bool newValue, KUndo2Command *parent = nullptr);
    KisLayerCollapseCommand(KisNodeSP node, bool newValue, KUndo2Command *parent = nullptr);

    void redo() override;
    void undo() override;
    bool mergeWith(const KUndo2Command *other) override;

private:
    KisNodeSP m_node;
    bool m_oldValue;
    bool m_newValue;
};
#endif // KISLAYERCOLLAPSECOMMAND_H
