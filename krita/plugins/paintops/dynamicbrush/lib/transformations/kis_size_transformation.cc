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

#include "kis_size_transformation.h"

#include "kis_dynamic_shape.h"
#include "kis_dynamic_sensor.h"
        

KisSizeTransformation::KisSizeTransformation(KisDynamicSensor* hTransfoParameter, KisDynamicSensor* vTransfoParameter)
    : KisDynamicTransformation(KoID("size",i18n("Resize"))), m_horizTransfoParameter(hTransfoParameter), m_vertiTransfoParameter(vTransfoParameter)
{
}
KisSizeTransformation::~KisSizeTransformation()
{
    if(m_horizTransfoParameter != m_vertiTransfoParameter)
        delete m_vertiTransfoParameter;
    delete m_horizTransfoParameter;
}

void KisSizeTransformation::transformBrush(KisDynamicShape* dabsrc, const KisPaintInformation& info)
{
    dabsrc->resize(m_horizTransfoParameter->parameter(info), m_vertiTransfoParameter->parameter(info));
}

void KisSizeTransformation::transformColoring(KisDynamicColoring* coloringsrc, const KisPaintInformation& info)
{
    Q_UNUSED(coloringsrc);
    Q_UNUSED(info);
    // TODO: implement it
}
