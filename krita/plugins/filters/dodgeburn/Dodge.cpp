/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
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

#include "Dodge.h"
#include <filter/kis_filter_configuration.h>

KisFilterDodgeBurn::KisFilterDodgeBurn(const QString& id, const QString& name ) : KisColorTransformationFilter(KoID(id, name), categoryAdjust(), name)
{
    setColorSpaceIndependence(FULLY_INDEPENDENT);
    setSupportsPainting(true);
    setSupportsPreview(true);
    setSupportsIncrementalPainting(false);
}

KoColorTransformation* KisFilterDodgeBurn::createTransformation(const KoColorSpace* cs, const KisFilterConfiguration* config) const
{
    QHash<QString, QVariant> params;
    if (config) {
        params["exposure"] = config->getDouble("exposure", 0.5);
    }
    return cs->createColorTransformation("DodgeShadow.ctl", params);
  
}
