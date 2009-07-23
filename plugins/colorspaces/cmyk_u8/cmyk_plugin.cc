/*
* cmyk_u8_plugin.cc -- Part of Krita
*
* Copyright (c) 2004 Boudewijn Rempt (boud@valdyas.org)
* Copyright (c) 2005 Adrian Page <adrian@pagenet.plus.com>
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
#include <KoColorSpaceRegistry.h>
#include <KoBasicHistogramProducers.h>
#include "cmyk_plugin.h"
#include "kis_cmyk_colorspace.h"

typedef KGenericFactory<CMYKU8Plugin> CMYKU8PluginFactory;
K_EXPORT_COMPONENT_FACTORY( kritacmykplugin, CMYKU8PluginFactory( "kocolorspaces" ) )


CMYKU8Plugin::CMYKU8Plugin(QObject *parent, const QStringList &)
    : QObject(parent)
{
    
    KoColorSpaceRegistry * f = KoColorSpaceRegistry::instance();

    KoColorSpaceFactory * csf = new KisCmykU8ColorSpaceFactory();
    f->add(csf);
    
    KoHistogramProducerFactoryRegistry::instance()->add(
            new KoBasicHistogramProducerFactory<KoBasicU8HistogramProducer>
            (KoID("CMYK8HISTO", i18n("CMYK8 Histogram")), csf->id()) );

}

CMYKU8Plugin::~CMYKU8Plugin()
{
}

#include "cmyk_plugin.moc"
