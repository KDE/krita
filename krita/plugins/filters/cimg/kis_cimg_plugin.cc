/*
 * This file is part of the KDE project
 *
 * Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
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

#include <kgenericfactory.h>

#include <kis_filter.h>
#include "kis_cimg_plugin.h"
#include "kis_cimg_filter.h"


typedef KGenericFactory<KisCImgPlugin> KisCImgPluginFactory;
K_EXPORT_COMPONENT_FACTORY( kritacimg, KisCImgPluginFactory( "krita" ) )

KisCImgPlugin::KisCImgPlugin(QObject *parent, const QStringList &) : KParts::Plugin(parent)
{
    setInstance(KisCImgPluginFactory::instance());

    if (parent->inherits("KisFilterRegistry")) {
        KisFilterRegistry * manager = dynamic_cast<KisFilterRegistry *>(parent);
        manager->add(KisFilterSP(new KisCImgFilter()));
    }
}

KisCImgPlugin::~KisCImgPlugin()
{
}

