/*
 *  SPDX-FileCopyrightText: 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_UPDATE_COMMAND_H
#define __KIS_UPDATE_COMMAND_H


#include "kundo2command.h"
#include "kritaimage_export.h"
#include "kis_types.h"

class KisUpdatesFacade;


class KRITAIMAGE_EXPORT KisUpdateCommand : public KUndo2Command
{
public:
    KisUpdateCommand(KisNodeSP node, QRect dirtyRect,
                     KisUpdatesFacade *updatesFacade,
                     bool needsFullRefresh = false);
    ~KisUpdateCommand() override;

    void undo() override;
    void redo() override;

private:
    void update();

private:
    KisNodeSP m_node;
    QRect m_dirtyRect;
    KisUpdatesFacade *m_updatesFacade;
    bool m_needsFullRefresh;
};

#endif /* __KIS_UPDATE_COMMAND_H */
