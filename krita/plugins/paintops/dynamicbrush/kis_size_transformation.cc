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

#include "kis_size_transformation.h"

#include "kis_transform_parameter.h"

KisSizeTransformation::KisSizeTransformation(KisTransformParameter* hTransfoParameter, KisTransformParameter* vTransfoParameter)
    : m_horizTransfoParameter(hTransfoParameter), m_vertiTransfoParameter(vTransfoParameter)
{
}
KisSizeTransformation::~KisSizeTransformation()
{
    if(m_horizTransfoParameter != m_vertiTransfoParameter)
        delete m_vertiTransfoParameter;
    delete m_horizTransfoParameter;
}
void KisSizeTransformation::transformDab(KisDabSource& dabsrc, const KisPaintInformation& info)
{
    // TODO: implement it, but I wonder if it makes sense to support it for the dab ?
    switch(dabsrc.type)
    {
        case KisDabSource::DabAuto:
            dabsrc.autoDab.width *= 2 * m_horizTransfoParameter->parameter(info);
            dabsrc.autoDab.hfade *= 2 * m_horizTransfoParameter->parameter(info);
            dabsrc.autoDab.height *= 2 * m_vertiTransfoParameter->parameter(info);
            dabsrc.autoDab.vfade *= 2 * m_vertiTransfoParameter->parameter(info);
            return;
        case KisDabSource::DabAlphaMask:
            // TODO: implement it
        return;
    }

}

void KisSizeTransformation::transformColoring(KisColoringSource& coloringsrc, const KisPaintInformation& info)
{
    Q_UNUSED(coloringsrc);
    Q_UNUSED(info);
    // TODO: implement it
}
