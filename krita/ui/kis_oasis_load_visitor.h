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
#ifndef KIS_OASIS_LOAD_VISITOR_H_
#define KIS_OASIS_LOAD_VISITOR_H_

#include "kis_global.h"
#include "kis_types.h"

class QDomElement;
class KisPaintLayer;
class KisGroupLayer;
class KisAdjustmentLayer;
class KisDoc2;

class KisOasisLoadVisitor {
public:
    KisOasisLoadVisitor(KisDoc2* doc) : m_doc(doc) {};
    virtual ~KisOasisLoadVisitor() {};

public:
    void loadImage(const QDomElement& elem);
    void loadPaintLayer(const QDomElement& elem, KisPaintLayerSP pL);
    void loadGroupLayer(const QDomElement& elem, KisGroupLayerSP gL);
    inline KisImageSP image() { return m_image; }
    QMap<KisLayer *, QString>& layerFilenames() { return m_layerFilenames; }
private:
    void loadLayerInfo(const QDomElement& elem, KisLayer* layer);
    KisImageSP m_image;
    KisDoc2* m_doc;
    QMap<KisLayer *, QString> m_layerFilenames;
};


#endif // KIS_LAYER_VISITOR_H_

