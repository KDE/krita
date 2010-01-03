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

#include "cpaintbrush.h"
#include <klocale.h>
#include <kiconloader.h>
#include <kcomponentdata.h>
#include <kstandarddirs.h>
#include <kis_debug.h>
#include <kgenericfactory.h>

#include <kis_paintop_registry.h>

#include "kis_cpaintop.h"

typedef KGenericFactory<CPaintBrush> CPaintBrushFactory;
K_EXPORT_COMPONENT_FACTORY(kritacpaintbrush, CPaintBrushFactory("krita"))

CPaintBrush::CPaintBrush(QObject *parent, const QStringList &)
        : QObject(parent)
{
    //setComponentData(CPaintBrushFactory::componentData());

    KisPaintOpRegistry *r = KisPaintOpRegistry::instance();
    if (r)
        r->add(new KisCPaintOpFactory);
}

CPaintBrush::~CPaintBrush()
{
}

#include "cpaintbrush.moc"
