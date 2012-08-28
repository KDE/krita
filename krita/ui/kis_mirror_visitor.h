/*
 *  Copyright (c) 2012 Sven Langkamp <sven.langkamp@gmail.com>
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

#ifndef KIS_MIRROR_VISITOR_H
#define KIS_MIRROR_VISITOR_H

#include <kis_node_visitor.h>

class KisMirrorVisitor : public KisNodeVisitor
{

public:
    KisMirrorVisitor(KisImageWSP image, Qt::Orientation orientation);

    virtual bool visit(KisSelectionMask* mask);
    virtual bool visit(KisTransparencyMask* mask);
    virtual bool visit(KisFilterMask* mask);
    virtual bool visit(KisCloneLayer* layer);
    virtual bool visit(KisGeneratorLayer* layer);
    virtual bool visit(KisExternalLayer* layer);
    virtual bool visit(KisAdjustmentLayer* layer);
    virtual bool visit(KisGroupLayer* layer);
    virtual bool visit(KisPaintLayer* layer);
    virtual bool visit(KisNode* node);
private:
    bool mirrorMask(KisMask* mask);
    bool mirrorDevice(KisPaintDeviceSP device, const QString& transitionName );
    
    KisImageWSP m_image;
    Qt::Orientation m_orientation;
};

#endif // KIS_MIRROR_VISITOR_H
