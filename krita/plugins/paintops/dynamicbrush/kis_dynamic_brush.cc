/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License.
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

#include <kis_paint_device.h>

#include "kis_dynamic_coloring.h"
#include "kis_dynamic_shape.h"
#include "kis_dynamic_transformation.h"

// FIXME: ugly workaround the fact that transformation are linked and that dynamic brush allways need one transformation, maybe move to a QList based storage for transformations ?
class KisDummyTransformation : public KisDynamicTransformation {
    public:
        KisDummyTransformation() : KisDynamicTransformation(KoID("dummy",i18n("Dummy"))) { }
    public:
        virtual ~KisDummyTransformation() {  }
        virtual void transformBrush(KisDynamicShape* , const KisPaintInformation& ) {};
        virtual void transformColoring(KisDynamicColoring* , const KisPaintInformation& ) {};
};

KisDynamicBrush::KisDynamicBrush(const QString& name)
    : m_name(name), m_rootTransfo(0), m_shape(0), m_coloring(0)
{
    m_rootTransfo = new KisDummyTransformation();
    
    // for debug purpose only
    KisPlainColoring* coloringsrc = new KisPlainColoring;
    coloringsrc->type = KisDynamicColoring::ColoringPlainColor;
    coloringsrc->color = KoColor(QColor(255,200,100), 255, coloringsrc->color.colorSpace() );
    m_coloring = coloringsrc;
    
    KisAutoMaskBrush* dabsrc = new KisAutoMaskBrush;
    dabsrc->autoDab.shape = KisAutoDab::ShapeCircle;
    dabsrc->autoDab.width = 10;
    dabsrc->autoDab.height = 10;
    dabsrc->autoDab.hfade = 2;
    dabsrc->autoDab.vfade = 2;
    m_shape = dabsrc;

}

KisDynamicBrush::~KisDynamicBrush()
{
    delete m_rootTransfo;
    if(m_shape) delete m_shape;
    if(m_coloring) delete m_coloring;
}
