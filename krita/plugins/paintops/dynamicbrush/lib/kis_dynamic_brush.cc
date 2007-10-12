/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_dynamic_brush.h"

#include <klocale.h>

#include <KoColorSpaceRegistry.h>

#include <kis_paint_device.h>

#include "kis_dynamic_coloring.h"
#include "kis_dynamic_shape.h"

// TODO: for debug purpose only
#include "shapes/kis_bristle_shape.h"
// #include "shapes/kis_dab_shape.h"

KisDynamicBrush::KisDynamicBrush(const QString& name)
    : m_name(name), m_shape(0), m_coloring(0), m_shapeProgram(0), m_coloringProgram(0)
{
    // for debug purpose only
    KisPlainColoring* coloringsrc = new KisPlainColoring( KoColor(QColor(255,200,100), 255, KoColorSpaceRegistry::instance()->rgb8() ) );
    m_coloring = coloringsrc;
    
#if 0
    KisAutoMaskShape* dabsrc = new KisAutoMaskShape;
    dabsrc->autoDab.shape = KisAutoMaskShape::KisAutoDab::ShapeCircle;
    dabsrc->autoDab.width = 10;
    dabsrc->autoDab.height = 10;
    dabsrc->autoDab.hfade = 2;
    dabsrc->autoDab.vfade = 2;
#endif
#if 1
    KisBristleShape* dabsrc = new KisBristleShape();
#endif
    m_shape = dabsrc;

}

KisDynamicBrush::~KisDynamicBrush()
{

    delete m_shape;
    delete m_coloring;
    delete m_shapeProgram;
    delete m_coloringProgram;
}

void KisDynamicBrush::startPainting(KisPainter* _painter)
{
    m_shape->startPainting(_painter);
}

void KisDynamicBrush::endPainting()
{
    m_shape->endPainting();
}

void KisDynamicBrush::setShapeProgram(KisDynamicShapeProgram* p)
{
    delete m_shapeProgram;
    m_shapeProgram = p;
}
void KisDynamicBrush::setColoringProgram(KisDynamicColoringProgram* p)
{
    delete m_coloringProgram;
    m_coloringProgram = p;
}
