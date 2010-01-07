/*
 * This file is part of the KDE project
 *
 * Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
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

#include "example.h"
#include <stdlib.h>
#include <vector>

#include <QPoint>
#include <QTime>

#include <klocale.h>
#include <kiconloader.h>
#include <kcomponentdata.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kis_debug.h>
#include <kpluginfactory.h>

#include <kis_processing_information.h>
#include <kis_types.h>
#include <kis_selection.h>
#include <kis_iterators_pixel.h>
#include <kis_layer.h>
#include <filter/kis_filter_registry.h>
#include <kis_global.h>

K_PLUGIN_FACTORY(KritaExampleFactory, registerPlugin<KritaExample>();)
K_EXPORT_PLUGIN(KritaExampleFactory("krita"))

KritaExample::KritaExample(QObject *parent, const QVariantList &)
        : QObject(parent)
{
    //setComponentData(KritaExampleFactory::componentData());

    KisFilterRegistry::instance()->add(KisFilterSP(new KisFilterInvert()));
}

KritaExample::~KritaExample()
{
}

KisFilterInvert::KisFilterInvert() : KisColorTransformationFilter(id(), categoryAdjust(), i18n("&Invert"))
{
    setColorSpaceIndependence(FULLY_INDEPENDENT);
    setSupportsPainting(true);
    setSupportsPreview(true);
    setSupportsIncrementalPainting(false);
}

KoColorTransformation* KisFilterInvert::createTransformation(const KoColorSpace* cs, const KisFilterConfiguration* config) const
{
    Q_UNUSED(config);
    return cs->createInvertTransformation();
}

