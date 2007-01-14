/*
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
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

#ifndef _KIS_SIZE_TRANSFORMATION_
#define _KIS_SIZE_TRANSFORMATION_

#include "kis_dynamic_transformation.h"

class KisTransformParameter;

class KisSizeTransformation : public KisDynamicTransformation {
    public:
        KisSizeTransformation(KisTransformParameter* hTransfoParameter, KisTransformParameter* vTransfoParameter);
        virtual ~KisSizeTransformation();
    public:
        virtual void transformBrush(KisDynamicShape* dabsrc, const KisPaintInformation& info);
        virtual void transformColoring(KisDynamicColoring* coloringsrc, const KisPaintInformation& info);
    private:
        KisTransformParameter* m_horizTransfoParameter;
        KisTransformParameter* m_vertiTransfoParameter;
};

#endif
