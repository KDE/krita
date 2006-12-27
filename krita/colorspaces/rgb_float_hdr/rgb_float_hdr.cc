/*
 * rgb_plugin.cc -- Part of Krita
 *
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
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
#include "rgb_float_hdr.h"

#include <stdlib.h>
#include <vector>

#include <QPoint>

#include <klocale.h>
#include <kiconloader.h>
#include <kinstance.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kgenericfactory.h>
#include <KoColorSpaceRegistry.h>
#include <KoBasicHistogramProducers.h>

#include "kis_rgb_f16_hdr_colorspace.h"

typedef KGenericFactory<RGBFloatHDRPlugin> RGBFloatHDRPluginFactory;
K_EXPORT_COMPONENT_FACTORY( kritargbfloathdrplugin, RGBFloatHDRPluginFactory( "krita" ) )


RGBFloatHDRPlugin::RGBFloatHDRPlugin(QObject *parent, const QStringList &)
    : QObject(parent)
{
    KoColorSpaceRegistry * f = KoColorSpaceRegistry::instance();
    // Register F16 HDR colorspace
    KoColorSpace * colorSpaceRGBF16Half  = new KisRgbF16HDRColorSpace(f, 0);
    KoColorSpaceFactory *csf  = new KisRgbF16HDRColorSpaceFactory();
    Q_CHECK_PTR(colorSpaceRGBF16Half);
    f->add(csf);
    KoHistogramProducerFactoryRegistry::instance()->add(
                new KoBasicHistogramProducerFactory<KoBasicF16HalfHistogramProducer>
                (KoID("RGBF16HALFHISTO", i18n("Float16 Half Histogram")), colorSpaceRGBF16Half) );
}

RGBFloatHDRPlugin::~RGBFloatHDRPlugin()
{
}

#include "rgb_float_hdr.moc"
