/*
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include "dynamicbrush.h"

#include <klocale.h>
#include <kiconloader.h>
#include <kcomponentdata.h>
#include <kdebug.h>
#include <kgenericfactory.h>

#include <kis_paintop_registry.h>

#include "kis_dynamicop.h"

typedef KGenericFactory<DynamicBrush> DynamicBrushFactory;
K_EXPORT_COMPONENT_FACTORY(kritadynamicbrush, DynamicBrushFactory("kritacore"))

        DynamicBrush::DynamicBrush(QObject *parent, const QStringList &)
    : KParts::Plugin(parent)
{
    setComponentData(DynamicBrushFactory::componentData());

    // This is not a gui plugin; only load it when the doc is created.
    if ( parent->inherits("KisPaintOpRegistry") )
    {
        KisPaintOpRegistry * r = dynamic_cast<KisPaintOpRegistry*>(parent);
        r->add (KisPaintOpFactorySP(new KisDynamicOpFactory));
    }

}

DynamicBrush::~DynamicBrush()
{
}

#include "dynamicbrush.moc"
