/*
 * This file is part of Krita
 *
 * Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
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

#include "colors.h"

#include <kgenericfactory.h>

#include "kis_minmax_filters.h"
#include "kis_color_to_alpha.h"

typedef KGenericFactory<KritaExtensionsColors> KritaExtensionsColorsFactory;
K_EXPORT_COMPONENT_FACTORY( kritaextensioncolorsfilters, KritaExtensionsColorsFactory( "krita" ) )

KritaExtensionsColors::KritaExtensionsColors(QObject *parent, const char *name, const QStringList &)
        : KParts::Plugin(parent, name)
{
    setInstance(KritaExtensionsColorsFactory::instance());


    kdDebug(41006) << "Extensions Colors Filter plugin. Class: "
          << className()
          << ", Parent: "
          << parent -> className()
          << "\n";

    if (parent->inherits("KisFilterRegistry")) {
        KisFilterRegistry * manager = dynamic_cast<KisFilterRegistry *>(parent);
        manager->add(new KisFilterMax());
        manager->add(new KisFilterMin());
        manager->add(new KisFilterColorToAlpha());
    }
}

KritaExtensionsColors::~KritaExtensionsColors()
{
}
