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
#ifndef KIS_NODE_COMPOSITEOP_COMMAND_H
#define KIS_NODE_COMPOSITEOP_COMMAND_H

#include "kis_node_command.h"


/// The command for setting the composite op
class KRITAIMAGE_EXPORT KisNodeCompositeOpCommand : public KisNodeCommand
{

public:
    /**
     * Constructor
     * @param node The node the command will be working on.
     * @param oldCompositeOp the old node composite op
     * @param newCompositeOp the new node composite op
     */
    KisNodeCompositeOpCommand(KisNodeSP node, const QString& oldCompositeOp, const QString& newCompositeOp);

    virtual void redo();
    virtual void undo();

private:
    QString m_oldCompositeOp;
    QString m_newCompositeOp;
};

#endif /* KIS_NODE_COMPOSITEOP_COMMAND_H */
