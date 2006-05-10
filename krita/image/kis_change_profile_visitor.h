/*
 *  Copyright (c) 2006 Boudewijn Rempt <boud@valdyas.org
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
#ifndef KIS_CHANGE_PROFILE_VISITOR_H_
#define KIS_CHANGE_PROFILE_VISITOR_H_

#include "kis_global.h"
#include "kis_types.h"
#include "kis_layer_visitor.h"
#include "kis_paint_layer.h"
#include "kis_paint_device.h"
#include "kis_adjustment_layer.h"
#include "kis_group_layer.h"

/**
 * The Change Profile visitor walks over all layers and if the current
 * layer has the specified colorspace AND the specified old profile, sets
 * the colorspace to the same colorspace with the NEW profile, without doing
 * conversions. This is essential if you have loaded an image that didn't
 * have an embedded profile to which you want to attach the right profile.
 */
class KisChangeProfileVisitor :public KisLayerVisitor {
public:
    KisChangeProfileVisitor(KisColorSpace *oldColorSpace, KisColorSpace *dstColorSpace);
    virtual ~KisChangeProfileVisitor();

public:
    virtual bool visit(KisPaintLayer *layer);
    virtual bool visit(KisGroupLayer *layer);
    virtual bool visit(KisPartLayer *layer);
    virtual bool visit(KisAdjustmentLayer* layer);
    
private:
    KisColorSpace *m_oldColorSpace;
    KisColorSpace *m_dstColorSpace;
};

KisChangeProfileVisitor::KisChangeProfileVisitor(KisColorSpace * oldColorSpace, 
                                                 KisColorSpace *dstColorSpace) :
    KisLayerVisitor(),
    m_oldColorSpace(oldColorSpace),
    m_dstColorSpace(dstColorSpace)
{
}

KisChangeProfileVisitor::~KisChangeProfileVisitor()
{
}

bool KisChangeProfileVisitor::visit(KisGroupLayer * layer)
{
    // Clear the projection, we will have to re-render everything.
    layer->resetProjection();
    
    KisLayerSP child = layer->firstChild();
    while (child) {
        child->accept(*this);
        child = child->nextSibling();
    }
    layer->setDirty();
    return true;
}

bool KisChangeProfileVisitor::visit(KisPaintLayer *layer)
{
    if (!layer) return false;
    if (!layer->paintDevice()) return false;
    if (!layer->paintDevice()->colorSpace()) return false;

    KisColorSpace * cs = layer->paintDevice()->colorSpace();

    if (cs == m_oldColorSpace) {
    
        layer->paintDevice()->setProfile(m_dstColorSpace->getProfile());

        layer->setDirty();
    }
    return true;
}

bool KisChangeProfileVisitor::visit(KisPartLayer *)
{
    return true;
}


bool KisChangeProfileVisitor::visit(KisAdjustmentLayer * layer)
{
    layer->resetCache();
    layer->setDirty();
    return true;
}

#endif // KIS_CHANGE_PROFILE_VISITOR_H_

