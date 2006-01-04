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

#include "krs_painter.h"

#include <kis_paint_layer.h>
#include <kis_paintop_registry.h>
#include <kis_painter.h>

#include "krs_brush.h"
#include "krs_color.h"

namespace Kross {

namespace KritaCore {

Painter::Painter(KisPaintLayerSP layer)
    : Kross::Api::Class<Painter>("KritaPainter"), m_layer(layer),m_painter(new KisPainter(layer->paintDevice()))
{
    addFunction("paintAt", &Painter::paintAt, Kross::Api::ArgumentList() << Kross::Api::Argument("Kross::Api::Variant") << Kross::Api::Argument("Kross::Api::Variant") << Kross::Api::Argument("Kross::Api::Variant") );
    addFunction("setPaintColor", &Painter::setPaintColor, Kross::Api::ArgumentList() << Kross::Api::Argument("Kross::Krita::Color") );
    addFunction("setBrush", &Painter::setBrush, Kross::Api::ArgumentList() << Kross::Api::Argument("Kross::Krita::Brush") );
    addFunction("setPaintOp", &Painter::setPaintOp, Kross::Api::ArgumentList() << Kross::Api::Argument("Kross::Api::Variant::String") );
}


Painter::~Painter()
{
    delete m_painter;
}

Kross::Api::Object::Ptr Painter::paintAt(Kross::Api::List::Ptr args)
{
    m_painter->paintAt( KisPoint( Kross::Api::Variant::toVariant(args->item(0)).toDouble(), Kross::Api::Variant::toVariant(args->item(1)).toDouble() ), Kross::Api::Variant::toVariant(args->item(2)).toDouble(), 0.0, 0.0);
    return 0;
}

Kross::Api::Object::Ptr Painter::setPaintColor(Kross::Api::List::Ptr args)
{
//     kdDebug() << "setPaintColor : " << args->item(0)->getClassName() << endl;
    Color* c = (Color*)args->item(0).data();
    m_painter->setPaintColor( KisColor(c->toQColor(), paintLayer()->paintDevice()->colorSpace() ));
    return 0;
}

Kross::Api::Object::Ptr Painter::setBrush(Kross::Api::List::Ptr args)
{
    Brush* b = (Brush*)args->item(0).data();
    m_painter->setBrush( b->getBrush());
    return 0;
}
Kross::Api::Object::Ptr Painter::setPaintOp(Kross::Api::List::Ptr args)
{
    QString id = Kross::Api::Variant::toString(args->item(0));
    KisPaintOp* op = KisPaintOpRegistry::instance()->paintOp( id, m_painter );
    m_painter->setPaintOp( op );
    return 0;
}

}

}
