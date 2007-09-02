/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
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

#include "kis_dynamic_transformations_factory.h"

KisDynamicTransformation* KisDynamicTransformationsFactory::id2Transformation(QString id)
{
    if(id == KisDynamicTransformation::SizeTransformationID.id())
    {
        return new KisSizeTransformation(0, 0);
    } else if(id == KisDynamicTransformation::DarkenTransformationID.id()) {
        return transfo = new KisDarkenTransformation(0);
    } else if(id == KisDynamicTransformation::RotationTransformationID) {
        return new KisRotationTransformation(0);
    }
    return 0;
}

KisDynamicTransformation* KisDynamicTransformationsFactory::createFromXML(const QDomElement& e)
{
    QString id = e.attribute("id", "");
    KisDynamicTransformation* transfo = id2Transformation(id);
    if(transfo)
    {
        transfo->fromXML( e );
    }
    return transfo;
}

QList<KoID> KisDynamicTransformationsFactory::ids()
{
    QList<KoID> ids;
    ids << KisDynamicTransformation::SizeTransformationID;
    ids << KisDynamicTransformation::RotationTransformationID;
    ids << KisDynamicTransformation::DarkenTransformationID;
    return ids;
}

