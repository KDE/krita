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

#include "imageenhancement.h"
#include <stdlib.h>
#include <vector>

#include <QPoint>

#include <klocale.h>
#include <kiconloader.h>
#include <kcomponentdata.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <ktemporaryfile.h>
#include <kis_debug.h>
#include <kgenericfactory.h>

#include <kis_doc2.h>
#include <kis_image.h>
#include <kis_iterators_pixel.h>
#include <kis_layer.h>
#include <filter/kis_filter_registry.h>
#include <kis_global.h>
#include <kis_types.h>
#include "kis_simple_noise_reducer.h"
#include "kis_wavelet_noise_reduction.h"

typedef KGenericFactory<KritaImageEnhancement> KritaImageEnhancementFactory;
K_EXPORT_COMPONENT_FACTORY(kritaimageenhancement, KritaImageEnhancementFactory("krita"))

KritaImageEnhancement::KritaImageEnhancement(QObject *parent, const QStringList &)
        : QObject(parent)
{
    //setComponentData(KritaImageEnhancementFactory::componentData());
    KisFilterRegistry::instance()->add(new KisSimpleNoiseReducer());
    KisFilterRegistry::instance()->add(new KisWaveletNoiseReduction());
}

KritaImageEnhancement::~KritaImageEnhancement()
{
}

