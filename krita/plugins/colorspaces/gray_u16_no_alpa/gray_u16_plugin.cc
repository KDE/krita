/*
* gray_u16_plugin.cc -- Part of Krita
*
* Copyright (c) 2004 Boudewijn Rempt (boud@valdyas.org)
* Copyright (c) 2005 Adrian Page <adrian@pagenet.plus.com>
*
*  This program is free software; you can distribute it and/or modify
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

#include "gray_u16_plugin.h"

#include <kgenericfactory.h>
#include <KoColorSpaceRegistry.h>
#include <KoBasicHistogramProducers.h>


#include "kis_gray_u16_colorspace.h"

typedef KGenericFactory<GRAYU16Plugin> GRAYU16PluginFactory;
K_EXPORT_COMPONENT_FACTORY(krita_gray_u16_plugin, GRAYU16PluginFactory("krita"))


GRAYU16Plugin::GRAYU16Plugin(QObject *parent, const QStringList &)
        : QObject(parent)
{
    KoColorSpaceRegistry * f = KoColorSpaceRegistry::instance();

    KoColorSpaceFactory * csf = new KisGrayU16ColorSpaceFactory();
    f->add(csf);

    KoHistogramProducerFactoryRegistry::instance()->add(
        new KoBasicHistogramProducerFactory<KoBasicU16HistogramProducer>
        (KoID("GRAYA16HISTO", i18n("GRAY/Alpha16 Histogram")), csf->id()));

}

GRAYU16Plugin::~GRAYU16Plugin()
{
}

#include "gray_u16_plugin.moc"
