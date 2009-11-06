/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
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

#ifndef KIS_IMAGE_NODE_PROPERTY_LIST_COMMAND_H_
#define KIS_IMAGE_NODE_PROPERTY_LIST_COMMAND_H_

#include "kis_node_command.h"
#include "KoDocumentSectionModel.h"


/// The command for changing the property list of a layer
class KRITAIMAGE_EXPORT KisNodePropertyListCommand : public KisNodeCommand
{

public:
    /**
     * Constructor
     * @param node the layer to add
     */
    KisNodePropertyListCommand(KisNodeSP node, KoDocumentSectionModel::PropertyList newPropertyList);

    virtual void redo();
    virtual void undo();

private:
    KoDocumentSectionModel::PropertyList m_newPropertyList;
    KoDocumentSectionModel::PropertyList m_oldPropertyList;
};
#endif
