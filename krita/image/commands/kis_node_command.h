/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2005 Casper Boemann <cbr@boemann.dk>
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
#ifndef KIS_NODE_COMMAND_H_
#define KIS_NODE_COMMAND_H_

#include <krita_export.h>
#include <QUndoCommand>
#include "kis_types.h"

class KisNode;

/// the base command for commands altering a node
class KRITAIMAGE_EXPORT KisNodeCommand : public QUndoCommand
{

public:
    /**
     * Constructor
     * @param name The name that will be shown in the ui
     * @param node The node the command will be working on.
     */
    KisNodeCommand(const QString& name, KisNodeSP node);
    virtual ~KisNodeCommand();

protected:
    KisNodeSP m_node;
};

#endif /* KIS_NODE_COMMAND_H_*/
