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

#include "kis_rotation_transformation.h"

#include <math.h>

#include <QDomElement>

#include "kis_dynamic_shape.h"
#include "kis_dynamic_sensor.h"

KisRotationTransformation::KisRotationTransformation(KisDynamicSensor* transfoParameter)
    : KisDynamicTransformation(KisDynamicTransformation::RotationTransformationID), m_transfoParameter(transfoParameter)
{
    
}

KisRotationTransformation::~KisRotationTransformation()
{
    delete m_transfoParameter;
}

void KisRotationTransformation::transformBrush(KisDynamicShape* dabsrc, const KisPaintInformation& info)
{
    dabsrc->rotate(m_transfoParameter->parameter(info) * 2.0 * M_PI);
}

void KisRotationTransformation::transformColoring(KisDynamicColoring* , const KisPaintInformation& )
{
}

void KisRotationTransformation::toXML(QDomDocument& d, QDomElement& e) const
{
    KisDynamicTransformation::toXML(d,e);
    QDomElement eSensor = d.createElement( "sensor" );
    m_transfoParameter->toXML( d, eSensor);
    e.appendChild( eSensor );
}

void KisRotationTransformation::fromXML(const QDomElement& rootElt)
{
    QDomNode n = rootElt.firstChild();
    while (!n.isNull()) {
        QDomElement e = n.toElement();
        if (not e.isNull()) {
            if (e.tagName() == "sensor") {
                m_transfoParameter = KisDynamicSensor::createFromXML(e);
            }
        }
        n = n.nextSibling();
    }

}

#include "kis_rotation_transformation.moc"
