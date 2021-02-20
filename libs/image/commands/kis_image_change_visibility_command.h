/*
 *  SPDX-FileCopyrightText: 2017 Nikita Smirnov <pakrentos@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */


#ifndef KIS_IMAGE_CHANGE_VISIBILITY_COMMAND_H_
#define KIS_IMAGE_CHANGE_VISIBILITY_COMMAND_H_

#include <kritaimage_export.h>

#include "kis_types.h"
#include <kundo2command.h>

class KisImageChangeVisibilityCommand : public KUndo2Command
{

public:
    KisImageChangeVisibilityCommand(bool visibility, KisNodeSP node);

    void redo() override;
    void undo() override;

private:
    bool m_visible;
    KisNodeSP m_node;

};

#endif
