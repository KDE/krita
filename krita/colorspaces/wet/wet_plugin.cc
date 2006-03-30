/*
 * wet_plugin.cc -- Part of Krita
 *
 * Copyright (c) 2004 Boudewijn Rempt (boud@valdyas.org)
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
#include <stdlib.h>
#include <vector>

#include <qobject.h>
#include <qapplication.h>
#include <qclipboard.h>
#include <q3dockwindow.h>
#include <qpoint.h>
#include <qlabel.h>
#include <qwidget.h>

#include <kactionclasses.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kinstance.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <ktempfile.h>
#include <kdebug.h>
#include <kgenericfactory.h>

#include <kopalettemanager.h>
#include <KoMainWindow.h>

#include <kis_debug_areas.h>
#include "kis_meta_registry.h"
#include <kis_factory.h>
#include <kis_image.h>
#include <kis_debug_areas.h>
#include <kis_types.h>
#include <kis_view.h>
#include <kis_colorspace_factory_registry.h>
#include <kis_tool_registry.h>
#include <kis_paintop_registry.h>
#include <kis_canvas_subject.h>
#include <kis_basic_histogram_producers.h>

#include "wet_plugin.h"
#include "kis_wet_palette_widget.h"
#include "kis_wet_colorspace.h"
#include "kis_wetop.h"
#include "kis_wetness_visualisation_filter.h"
#include "kis_texture_filter.h"
#include "wetphysicsfilter.h"

typedef KGenericFactory<WetPlugin> WetPluginFactory;
K_EXPORT_COMPONENT_FACTORY( kritawetplugin, WetPluginFactory( "kritacore" ) )


WetPlugin::WetPlugin(QObject *parent, const char *name, const QStringList &)
    : KParts::Plugin(parent, name)
{
    setInstance(WetPluginFactory::instance());

    // This is not a gui plugin; only load it when the doc is created.
    if ( parent->inherits("KisColorSpaceFactoryRegistry") ) {
        KisColorSpaceFactoryRegistry * f = dynamic_cast<KisColorSpaceFactoryRegistry*>(parent);

        KisColorSpace* colorSpaceWet = new KisWetColorSpace(f, 0);

        KisColorSpaceFactory * csf = new KisWetColorSpaceFactory();
        Q_CHECK_PTR(colorSpaceWet);

        // colorspace
        f->add(csf);

        // histogram producer
        KisHistogramProducerFactoryRegistry::instance()->add(
                new KisBasicHistogramProducerFactory<KisBasicU16HistogramProducer>
                (KisID("WETHISTO", i18n("Wet Histogram")), colorSpaceWet) );

        // wet brush op
        KisPaintOpRegistry::instance()->add(new KisWetOpFactory);

        // Dry filter
        // KisFilterRegistry::instance()->add( new WetPhysicsFilter() );

        // Texture Action:
        f->addPaintDeviceAction(colorSpaceWet, new WetPaintDevAction);
    }
    else if (parent->inherits("KisView"))
    {
        setInstance(WetPluginFactory::instance());
        setXMLFile(locate("data","kritaplugins/wetplugin.rc"), true);

        m_view = dynamic_cast<KisView*>(parent);
        // Wetness visualisation
        WetnessVisualisationFilter * wf = new WetnessVisualisationFilter(m_view);
        wf->setAction(new KToggleAction(i18n("Wetness Visualisation"), 0, 0, wf,
                        SLOT(slotActivated()), actionCollection(), "wetnessvisualisation"));

        // Create the wet palette
        KisWetPaletteWidget * w = new KisWetPaletteWidget(m_view);
        Q_CHECK_PTR(w);

        w->setCaption(i18n("Watercolors"));

        m_view->canvasSubject()->paletteManager()->addWidget(w, "watercolor docker", krita::COLORBOX, INT_MAX, PALETTE_DOCKER,  false);
        m_view->canvasSubject()->attach(w);
    }


}

WetPlugin::~WetPlugin()
{
}

#include "wet_plugin.moc"
