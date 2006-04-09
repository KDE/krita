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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#include <stdlib.h>
#include <vector>

#include <qpoint.h>

#include <klocale.h>
#include <kiconloader.h>
#include <kinstance.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <ktempfile.h>
#include <kdebug.h>
#include <kgenericfactory.h>

#include <kis_doc.h>
#include <kis_image.h>
#include <kis_iterators_pixel.h>
#include <kis_layer.h>
#include <kis_filter_registry.h>
#include <kis_global.h>
#include <kis_types.h>

#include "imageenhancement.h"

#include "kis_simple_noise_reducer.h"
#include "kis_wavelet_noise_reduction.h"

typedef KGenericFactory<KritaImageEnhancement> KritaImageEnhancementFactory;
K_EXPORT_COMPONENT_FACTORY( kritaimageenhancement, KritaImageEnhancementFactory( "krita" ) )

        KritaImageEnhancement::KritaImageEnhancement(QObject *parent, const char *name, const QStringList &)
  : KParts::Plugin(parent)
{
    setObjectName(name);
    setInstance(KritaImageEnhancementFactory::instance());

    kDebug(41006) << "Image enhancement filter plugin. Class: "
           << className()
           << ", Parent: "
           << parent->className()
           << "\n";


    if ( parent->inherits("KisFilterRegistry") )
    {
        KisFilterRegistry * r = dynamic_cast<KisFilterRegistry*>(parent);
        r->add(KisFilterSP(new KisSimpleNoiseReducer()));
        r->add(KisFilterSP(new KisWaveletNoiseReduction()));
    }
}

KritaImageEnhancement::~KritaImageEnhancement()
{
}

