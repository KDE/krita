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
#include <kis_view.h>

// #include <kmessagebox.h>

#include "example.h"

typedef KGenericFactory<KritaExample> KritaExampleFactory;
K_EXPORT_COMPONENT_FACTORY( kritaexample, KritaExampleFactory( "krita" ) )

KritaExample::KritaExample(QObject *parent, const char *name, const QStringList &)
        : KParts::Plugin(parent, name)
{
    setInstance(KritaExampleFactory::instance());


    kdDebug(DBG_AREA_PLUGINS) << "Example plugin. Class: "
          << className()
          << ", Parent: "
          << parent -> className()
          << "\n";

    if ( parent->inherits("KisFactory") )
    {
        KisFilterRegistry::instance()->add(new KisFilterInvert());
    }
}

KritaExample::~KritaExample()
{
}

KisFilterInvert::KisFilterInvert() : KisFilter(id(), "colors", "&Invert")
{
}

void KisFilterInvert::process(KisPaintDeviceImplSP src, KisPaintDeviceImplSP dst, KisFilterConfiguration* /*config*/, const QRect& rect)
{
    KisRectIteratorPixel dstIt = dst->createRectIterator(rect.x(), rect.y(), rect.width(), rect.height(), true );
    KisRectIteratorPixel srcIt = src->createRectIterator(rect.x(), rect.y(), rect.width(), rect.height(), false);
    Q_INT32 depth = src -> colorSpace() -> nColorChannels();

    int pixelsProcessed = 0;
    setProgressTotalSteps(rect.width() * rect.height());
    while( ! srcIt.isDone() )
    {
        if(srcIt.isSelected())
        {
            for( int i = 0; i < depth; i++)
            {
                dstIt.rawData()[i] = Q_UINT8_MAX - srcIt.oldRawData()[i];
            }
        }
        setProgress(++pixelsProcessed);
        ++srcIt;
        ++dstIt;
    }
    setProgressDone(); // Must be called even if you don't really support progression
}
