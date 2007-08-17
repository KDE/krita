/*
 *  Copyright (c) 2005 Casper Boemann <cbr@boemann.dk>
 *  Copyright (c) 2007 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef KIS_NODE_VISITOR_H_
#define KIS_NODE_VISITOR_H_

#include "kis_node.h"

class KisPaintLayer;
class KisGroupLayer;
class KisAdjustmentLayer;
class KisExternalLayer;
class KisCloneLayer;
class KisFilterMask;
class KisTransparencyMask;
class KisTransformationMask;
class KisSelectionMask;

class KisNodeVisitor {
public:
    KisNodeVisitor() {}
    virtual ~KisNodeVisitor() {}

public:
    virtual bool visit(KisNode *node) { Q_UNUSED( node ); return false; }
    virtual bool visit(KisPaintLayer *layer) { Q_UNUSED( layer ); return false; }
    virtual bool visit(KisGroupLayer *layer) { Q_UNUSED( layer ); return false; }
    virtual bool visit(KisAdjustmentLayer *layer) { Q_UNUSED( layer ); return false; }
    virtual bool visit(KisExternalLayer *layer) { Q_UNUSED( layer ); return false; }
    virtual bool visit(KisCloneLayer *layer) { Q_UNUSED( layer ); return false; }
    virtual bool visit(KisFilterMask *mask) { Q_UNUSED( mask ); return false; }
    virtual bool visit(KisTransparencyMask *mask) { Q_UNUSED( mask ); return false; }
    virtual bool visit(KisTransformationMask *mask) { Q_UNUSED( mask ); return false; }
    virtual bool visit(KisSelectionMask *mask) { Q_UNUSED( mask ); return false; }

protected:

    /**
     * Visit all child nodes of the given node until one node returns
     * false. Then visitAll returns false, otherwise true.
     *
     * @param node the parent node whose children will be visited
     * @param breakOnFail break if one of the childs returns false on accept
     * @return true if none of the childnodes returns false on
     * accepting the visitor.
     */
    bool visitAll( KisNode * node, bool breakOnFail = false )
        {
            for ( uint i = 0; i < node->childCount(); ++i ) {
                if ( !node->at( i )->accept( *this ) ) {
                    if ( breakOnFail )
                        return false;
                }
            }
            return true;
        }
};


#endif // KIS_ NODE_VISITOR_H_

