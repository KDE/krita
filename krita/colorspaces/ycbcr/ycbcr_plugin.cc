/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
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
#include "ycbcr_plugin.h"

#include <config-krita.h>

#include <stdlib.h>
#include <vector>

#include <QPoint>

#include <klocale.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kgenericfactory.h>
#include <KoColorSpaceRegistry.h>
#include <KoBasicHistogramProducers.h>

#include "kis_ycbcr_u16_colorspace.h"
#include "kis_ycbcr_u8_colorspace.h"

typedef KGenericFactory<YCbCrPlugin> YCbCrPluginFactory;
K_EXPORT_COMPONENT_FACTORY( kritaycbcrplugin, YCbCrPluginFactory( "krita" ) )


YCbCrPlugin::YCbCrPlugin(QObject *parent, const QStringList &)
    : QObject(parent)
{
    KoColorSpaceRegistry * f = KoColorSpaceRegistry::instance();
    // Register U16 YCbCr colorspace
    {
        KoColorSpace * colorSpaceYCbCr16  = new KisYCbCrU16ColorSpace(f, 0);
        KoColorSpaceFactory *csf  = new KisYCbCrU16ColorSpaceFactory();
        Q_CHECK_PTR(colorSpaceYCbCr16);
        f->add(csf);
        KoHistogramProducerFactoryRegistry::instance()->add(
                    new KoBasicHistogramProducerFactory<KoBasicU16HistogramProducer>
                    (KoID("YCBCRU16HISTO", i18n("Unsigned Int 16 Half Histogram")), colorSpaceYCbCr16) );
    }
    // Register U8 YCbCr colorspace
    {
        KoColorSpace * colorSpaceYCbCr8  = new KisYCbCrU8ColorSpace(f, 0);
        KoColorSpaceFactory *csf  = new KisYCbCrU8ColorSpaceFactory();
        Q_CHECK_PTR(colorSpaceYCbCr8);
        f->add(csf);
        KoHistogramProducerFactoryRegistry::instance()->add(
                    new KoBasicHistogramProducerFactory<KoBasicU8HistogramProducer>
                    (KoID("YCBCRU8HISTO", i18n("Unsigned Int 8 Half Histogram")), colorSpaceYCbCr8) );
    }
}

YCbCrPlugin::~YCbCrPlugin()
{
}

#include "ycbcr_plugin.moc"
