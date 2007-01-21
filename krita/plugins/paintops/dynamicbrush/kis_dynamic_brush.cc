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

#include "kis_dynamic_transformation.h"

// FIXME: ugly workaround the fact that transformation are linked and that dynamic brush allways need one transformation, maybe move to a QList based storage for transformations ?
class KisDummyTransformation : public KisDynamicTransformation {
    public:
        KisDummyTransformation() : KisDynamicTransformation(KoID("dummy",i18n("Dummy"))) { }
        virtual ~KisDummyTransformation() {  }
        virtual void transformBrush(KisDynamicShape* , const KisPaintInformation& ) {};
        virtual void transformColoring(KisDynamicColoring* , const KisPaintInformation& ) {};
};

KisDynamicBrush::KisDynamicBrush(const QString& name) : m_name(name)
{
    m_rootTransfo = new KisDummyTransformation();
}
