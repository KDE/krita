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

#include "kis_dynamic_transformation.h"

#include <QDomElement>

#include <klocale.h>

#include "kis_dynamic_transformation.moc"

KoID KisDynamicTransformation::SizeTransformationID = KoID("size", i18n("Resize"));
KoID KisDynamicTransformation::RotationTransformationID = KoID("rotation",i18n("Rotation"));
KoID KisDynamicTransformation::DarkenTransformationID = KoID("darken",i18n("Darken"));

void KisDynamicTransformation::toXML(QDomDocument&, QDomElement& e) const
{
    e.setAttribute("id", id());
}

void KisDynamicTransformation::fromXML(const QDomElement& e)
{
    Q_ASSERT(e.attribute("id","") == id());
    
}

