/*
 *  Copyright (c) 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
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

#ifndef __KIS_UPDATE_COMMAND_H
#define __KIS_UPDATE_COMMAND_H


#include "kundo2command.h"
#include "krita_export.h"
#include "kis_types.h"

class KisUpdatesFacade;


class KRITAIMAGE_EXPORT KisUpdateCommand : public KUndo2Command
{
public:
    KisUpdateCommand(KisNodeSP node, QRect dirtyRect,
                     KisUpdatesFacade *updatesFacade,
                     bool needsFullRefresh = false);
    ~KisUpdateCommand();

    void undo();
    void redo();

private:
    void update();

private:
    KisNodeSP m_node;
    QRect m_dirtyRect;
    KisUpdatesFacade *m_updatesFacade;
    bool m_needsFullRefresh;
};

#endif /* __KIS_UPDATE_COMMAND_H */
