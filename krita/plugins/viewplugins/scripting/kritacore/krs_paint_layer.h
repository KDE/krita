/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
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

#ifndef KROSS_KRITACOREKRSPAINTLAYER_H
#define KROSS_KRITACOREKRSPAINTLAYER_H

#include <QObject>

#include <kis_types.h>
#include <kis_paint_layer.h>

class KisDoc2;
class KisTransaction;

namespace Scripting {

class Image;

/**
 * A paintlayer is a layer within a \a Image where you are able
 * to perform paint-operations on.
 */
class PaintLayer : public QObject
{
        Q_OBJECT
    public:
        explicit PaintLayer(KisPaintLayerSP layer, KisDoc2* doc = 0);
        virtual ~PaintLayer();

    public slots:
        QObject* paintDevice();

    public:
        inline KisPaintLayerSP paintLayer() { return m_layer; }
        inline const KisPaintLayerSP paintLayer() const { return m_layer; }
        inline KisDoc2* doc() { return m_doc; }
    private:
        KisPaintLayerSP m_layer;
        KisDoc2* m_doc;
        KisTransaction* m_cmd;
};

}

#endif
