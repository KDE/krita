/*
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
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
#ifndef KIS_OASIS_SAVE_DATA_VISITOR_H_
#define KIS_OASIS_SAVE_DATA_VISITOR_H_

#include "kis_global.h"
#include "kis_types.h"

#include "kis_layer_visitor.h"

class KoOasisStore;
class KoXmlWriter;

class KisAdjustmentLayer;
class KisGroupLayer;
class KisPaintLayer;

class KisOasisSaveDataVisitor : public KisLayerVisitor {
public:
    KisOasisSaveDataVisitor(KoOasisStore* os, KoXmlWriter* manifestWriter);
    virtual ~KisOasisSaveDataVisitor() {};

public:
    bool visit(KisPaintLayer *layer);
    bool visit(KisGroupLayer *layer);
    bool visit(KisAdjustmentLayer *layer);
    bool visit( KisExternalLayer * )
        {
            return true;
        }
private:
    KoOasisStore* m_oasisStore;
    KoXmlWriter* m_manifestWriter;
};


#endif // KIS_LAYER_VISITOR_H_

