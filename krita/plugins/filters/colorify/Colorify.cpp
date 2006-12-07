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

#include "Colorify.h"

#include <kcolorbutton.h>
#include <kgenericfactory.h>

#include <kis_iterators_pixel.h>

#include "WdgColorifyBase.h"
#include "KisWdgColorify.h"

typedef KGenericFactory<KritaColorify> KritaColorifyFactory;
K_EXPORT_COMPONENT_FACTORY( kritacolorify, KritaColorifyFactory( "krita" ) )

KritaColorify::KritaColorify(QObject *parent, const char *name, const QStringList &)
        : KParts::Plugin(parent, name)
{
    setInstance(KritaColorifyFactory::instance());


    kdDebug(41006) << "Colorify Filter plugin. Class: "
          << className()
          << ", Parent: "
          << parent -> className()
          << "\n";

    if (parent->inherits("KisFilterRegistry")) {
        KisFilterRegistry * manager = dynamic_cast<KisFilterRegistry *>(parent);
        manager->add(new KisColorify());
    }
}

KritaColorify::~KritaColorify()
{
}



KisColorify::KisColorify() : KisFilter(id(), "colors", i18n("&Colorify..."))
{
}

KisFilterConfigWidget * KisColorify::createConfigurationWidget(QWidget* parent, KisPaintDeviceSP )
{
    return new KisWdgColorify(this, parent, "configuration of colorify");
}

KisFilterConfiguration* KisColorify::configuration(QWidget* w)
{
    KisWdgColorify * wCTA = dynamic_cast<KisWdgColorify*>(w);
    KisFilterConfiguration* config = new KisFilterConfiguration("colorify", 1);
    if(wCTA)
    {
        config->setProperty("color", wCTA->widget()->colorTarget->color() );
    }
    return config;
}

void KisColorify::process(KisPaintDeviceSP src, KisPaintDeviceSP dst, KisFilterConfiguration* config, const QRect& rect)
{
    Q_ASSERT(src != 0);
    Q_ASSERT(dst != 0);
    
    if(config == 0) config = new KisFilterConfiguration("colorify", 1);
    
    QVariant value;
    QColor cTA = (config->getProperty("color", value)) ? value.toColor() : QColor(200,175,125);
    
    KisRectIteratorPixel dstIt = dst->createRectIterator(rect.x(), rect.y(), rect.width(), rect.height(), true );
    KisRectIteratorPixel srcIt = src->createRectIterator(rect.x(), rect.y(), rect.width(), rect.height(), false);

    KisColorSpace * cs = src->colorSpace();
    Q_UINT8* colorpixel =  new Q_UINT8[ cs->pixelSize() ];
    
    cs->fromQColor(cTA, colorpixel);
    
    Q_UINT16 labcTA[4];
    Q_UINT16 lab[4];
    
    cs->toLabA16(colorpixel, (Q_UINT8*)labcTA, 1);
    
    int pixelsProcessed = 0;
    setProgressTotalSteps(rect.width() * rect.height());

    Q_INT32 pixelsize = cs->pixelSize();
    
    while( ! srcIt.isDone() )
    {
        if(srcIt.isSelected())
        {
          cs->toLabA16(srcIt.oldRawData(), (Q_UINT8*)lab, 1);
          labcTA[0] = lab[0];
          cs->fromLabA16((Q_UINT8*)labcTA, dstIt.rawData(), 1);
        }
        setProgress(++pixelsProcessed);
        ++srcIt;
        ++dstIt;
    }
    delete colorpixel;
    setProgressDone(); // Must be called even if you don't really support progression
}


