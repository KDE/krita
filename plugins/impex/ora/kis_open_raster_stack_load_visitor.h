/*
 *  SPDX-FileCopyrightText: 2006 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_OPEN_RASTER_STACK_LOAD_VISITOR_H_
#define KIS_OPEN_RASTER_STACK_LOAD_VISITOR_H_

#include "kis_global.h"
#include "kis_types.h"

class QDomElement;

class KisUndoStore;
class KisOpenRasterLoadContext;

class KisOpenRasterStackLoadVisitor
{
public:
    KisOpenRasterStackLoadVisitor(KisUndoStore *undoStore, KisOpenRasterLoadContext* orlc);
    virtual ~KisOpenRasterStackLoadVisitor();

public:
    void loadImage();
    void loadPaintLayer(const QDomElement& elem, KisPaintLayerSP pL);
    void loadAdjustmentLayer(const QDomElement& elem, KisAdjustmentLayerSP pL);
    void loadGroupLayer(const QDomElement& elem, KisGroupLayerSP groupLayer);
    KisImageSP image();
    vKisNodeSP activeNodes();
private:
    void loadLayerInfo(const QDomElement& elem, KisLayerSP layer);
    struct Private;
    Private* const d;
};


#endif // KIS_LAYER_VISITOR_H_

