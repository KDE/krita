/*
 * Copyright (c) 2005 Boudewijn Rempt (boud@valdyas.org)
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
#include <klocale.h>
#include <kiconloader.h>
#include <kinstance.h>
#include <kstandarddirs.h>
#include <ktempfile.h>
#include <kdebug.h>
#include <kgenericfactory.h>

#include <kis_paintop_registry.h>

#include "kis_smearyop.h"
#include "kis_paintop_registry.h"

#include "smearybrush.h"

typedef KGenericFactory<SmearyBrush> SmearyBrushFactory;
K_EXPORT_COMPONENT_FACTORY(kritasmearybrush, SmearyBrushFactory("kritacore"))

SmearyBrush::SmearyBrush(QObject *parent, const char *name, const QStringList &)
    : KParts::Plugin(parent, name)
{
    setInstance(SmearyBrushFactory::instance());

    // This is not a gui plugin; only load it when the doc is created.
    if ( parent->inherits("KisPaintOpRegistry") )
    {
        kdDebug() << "Loading smeary brush\n";
        KisPaintOpRegistry * r = dynamic_cast<KisPaintOpRegistry*>(parent);
        r->add ( new KisSmearyOpFactory );
    }

}

SmearyBrush::~SmearyBrush()
{
}

#include "smearybrush.moc"
