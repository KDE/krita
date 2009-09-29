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
#ifndef KIS_OPEN_RASTER_STACK_LOAD_VISITOR_H_
#define KIS_OPEN_RASTER_STACK_LOAD_VISITOR_H_

#include "kis_global.h"
#include "kis_types.h"

#include <krita_export.h>

class QDomElement;

class KoStore;

class KisAdjustmentLayer;
class KisDoc2;
class KisGroupLayer;
class KisOpenRasterLoadContext;
class KisPaintLayer;

class KRITAUI_EXPORT KisOpenRasterStackLoadVisitor
{
public:
    KisOpenRasterStackLoadVisitor(KisDoc2* doc, KisOpenRasterLoadContext* orlc);
    virtual ~KisOpenRasterStackLoadVisitor();

public:
    void loadImage();
    void loadPaintLayer(const QDomElement& elem, KisPaintLayerSP pL);
    void loadAdjustmentLayer(const QDomElement& elem, KisAdjustmentLayerSP pL);
    void loadGroupLayer(const QDomElement& elem, KisGroupLayerSP gL);
    KisImageWSP image();
private:
    void loadLayerInfo(const QDomElement& elem, KisLayer* layer);
    struct Private;
    Private* const d;
};


#endif // KIS_LAYER_VISITOR_H_

