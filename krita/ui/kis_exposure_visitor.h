/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
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

#ifndef _KIS_EXPOSURE_VISITOR_H_
#define _KIS_EXPOSURE_VISITOR_H_

#include "kis_node_visitor.h"

class KoColorProfile;

/**
 * Set the exposure
 */
class KisExposureVisitor : public KisNodeVisitor
{
public:
    KisExposureVisitor(double exposure);

    using KisNodeVisitor::visit;

    virtual bool visit(KisExternalLayer *);
    virtual bool visit(KisPaintLayer *layer);
    virtual bool visit(KisGroupLayer *layer);
    virtual bool visit(KisAdjustmentLayer* layer);
    virtual bool visit(KisGeneratorLayer* layer);
    bool visit(KisNode*) {
        return true;
    }
    bool visit(KisCloneLayer*) {
        return true;
    }
    bool visit(KisFilterMask*) {
        return true;
    }
    bool visit(KisTransparencyMask*) {
        return true;
    }
    bool visit(KisTransformationMask*) {
        return true;
    }
    bool visit(KisSelectionMask*) {
        return true;
    }

protected:
    void setExposureToProfile(KoColorProfile* profile);
private:
    double m_exposure;
};


#endif
