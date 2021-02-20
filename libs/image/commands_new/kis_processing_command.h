/*
 *  SPDX-FileCopyrightText: 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_PROCESSING_COMMAND_H
#define __KIS_PROCESSING_COMMAND_H

#include <kundo2command.h>
#include "kis_types.h"
#include "kis_surrogate_undo_adapter.h"

class KisProcessingVisitor;

class KRITAIMAGE_EXPORT KisProcessingCommand : public KUndo2Command
{
public:
    KisProcessingCommand(KisProcessingVisitorSP visitor, KisNodeSP node, KUndo2Command *parent = 0);

    void undo() override;
    void redo() override;

private:
    KisProcessingVisitorSP m_visitor;
    KisNodeSP m_node;
    KisSurrogateUndoAdapter m_undoAdapter;
    bool m_visitorExecuted;
};

#endif /* __KIS_PROCESSING_COMMAND_H */
