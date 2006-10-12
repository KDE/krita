/*
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
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

#ifndef _KIS_DARKEN_TRANSFORMATION_
#define _KIS_DARKEN_TRANSFORMATION_

#include "kis_dynamic_transformation.h"

class KisTransformParameter;

class KisDarkenTransformation : public KisDynamicTransformation {
    public:
        KisDarkenTransformation(KisTransformParameter* transfoParameter);
        virtual ~KisDarkenTransformation();
    public:
        virtual void transformDab(KisDabSource& dabsrc, const KisPaintInformation& info);
        virtual void transformColoring(KisColoringSource& coloringsrc, const KisPaintInformation& info);
    private:
        KisTransformParameter* m_transfoParameter;
};

#endif
