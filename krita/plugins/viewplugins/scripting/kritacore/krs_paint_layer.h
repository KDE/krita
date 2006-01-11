/*
 *  Copyright (c) 2005 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Library General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KROSS_KRITACOREKRSLAYER_H
#define KROSS_KRITACOREKRSLAYER_H

#include <api/class.h>

#include <kis_types.h>

class KisDoc;
class KisTransaction;

namespace Kross {

namespace KritaCore {

/**
@author Cyrille Berger
*/
class PaintLayer : public Kross::Api::Class<PaintLayer>
{
    public:
        explicit PaintLayer(KisPaintLayerSP layer, KisDoc* doc = 0);
        virtual ~PaintLayer();
        virtual const QString getClassName() const;
    private:
        Kross::Api::Object::Ptr createRectIterator(Kross::Api::List::Ptr);
        Kross::Api::Object::Ptr createHLineIterator(Kross::Api::List::Ptr);
        Kross::Api::Object::Ptr createVLineIterator(Kross::Api::List::Ptr);
        Kross::Api::Object::Ptr getWidth(Kross::Api::List::Ptr);
        Kross::Api::Object::Ptr getHeight(Kross::Api::List::Ptr);
        Kross::Api::Object::Ptr createHistogram(Kross::Api::List::Ptr);
        Kross::Api::Object::Ptr createPainter(Kross::Api::List::Ptr);
        Kross::Api::Object::Ptr beginPainting(Kross::Api::List::Ptr args);
        Kross::Api::Object::Ptr endPainting(Kross::Api::List::Ptr args);
        Kross::Api::Object::Ptr convertToColorspace(Kross::Api::List::Ptr args);
    public:
        inline KisPaintLayerSP paintLayer() { return m_layer; }
        inline KisDoc* doc() { return m_doc; }
    private:
        KisPaintLayerSP m_layer;
        KisDoc* m_doc;
        KisTransaction* m_cmd;
};

}

}

#endif
