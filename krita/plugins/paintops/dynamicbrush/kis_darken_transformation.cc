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

#include "kis_darken_transformation.h"

#include "kis_dynamic_coloring.h"
#include "kis_transform_parameter.h"

KisDarkenTransformation::KisDarkenTransformation(KisTransformParameter* transfoParameter)
    : m_transfoParameter(transfoParameter)
{
    
}

KisDarkenTransformation::~KisDarkenTransformation()
{
    delete m_transfoParameter;
}

void KisDarkenTransformation::transformBrush(KisDynamicBrush* dabsrc, const KisPaintInformation& info)
{
    Q_UNUSED(dabsrc);
    Q_UNUSED(info);
    // TODO: implement it, but I wonder if it makes sense to support it for the dab ?
}

void KisDarkenTransformation::transformColoring(KisDynamicColoring* coloringsrc, const KisPaintInformation& info)
{
    coloringsrc->darken(m_transfoParameter->parameter(info));
}
