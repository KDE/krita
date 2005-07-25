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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#include <stdlib.h>
#include <vector>

#include <qobject.h>
#include <qapplication.h>
#include <qclipboard.h>
#include <qdockwindow.h>
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

#include <koPaletteManager.h>
#include <koMainWindow.h>

#include <kis_factory.h>
#include <kis_image.h>
#include <kis_global.h>
#include <kis_types.h>
#include <kis_view.h>
#include <kis_colorspace_registry.h>
#include <kis_tool_registry.h>
#include <kis_paintop_registry.h>
#include <kis_canvas_subject.h>
#include <kis_xml_gui_client.h>

#include "wet_plugin.h"
#include "kis_wet_palette_widget.h"
#include "kis_colorspace_wet.h"
#include "kis_wetop.h"
#include "kis_tool_wet_brush.h"
#include "kis_wetness_visualisation_filter.h"
#include "kis_texture_filter.h"
#include "wetphysicsfilter.h"

typedef KGenericFactory<WetPlugin> WetPluginFactory;
K_EXPORT_COMPONENT_FACTORY( kritawetplugin, WetPluginFactory( "kritacore" ) )


WetPlugin::WetPlugin(QObject *parent, const char *name, const QStringList &)
	: KParts::Plugin(parent, name)
{
	setInstance(WetPluginFactory::instance());

 	kdDebug(DBG_AREA_PLUGINS) << "Wet Color model plugin. Class: "
 		  << className()
 		  << ", Parent: "
 		  << parent -> className()
 		  << "\n";

	m_dynamicClient = new KisXMLGUIClient("krita/kpartplugins/pluggablewetplugin.rc");

	// This is not a gui plugin; only load it when the doc is created.
	if ( parent->inherits("KisFactory") )
	{
		m_colorSpaceWet = new KisColorSpaceWet();
		Q_CHECK_PTR(m_colorSpaceWet);

		// colorspace
		KisColorSpaceRegistry::instance() -> add(m_colorSpaceWet);

		// wet brush op
		KisPaintOpRegistry::instance() -> add(new KisWetOpFactory);

		// Dry filter
		KisFilterRegistry::instance()->add( new WetPhysicsFilter() );
		
		// Create the wet brush paint tool
		KisToolRegistry::instance() -> add(new KisToolWetBrushFactory(
				m_dynamicClient -> actionCollection() ) );
	}
	else if (parent -> inherits("KisView"))
	{
		m_view = (KisView*)parent;

		// Wetness visualisation
		WetnessVisualisationFilter * wf = new WetnessVisualisationFilter(m_view);
		KToggleAction* visualisationAction = new KToggleAction(i18n("Wetness Visualisation"),
				0, 0, wf,
				SLOT(slotActivated()), m_dynamicClient -> actionCollection(),
				"wetnessvisualisation");
		wf -> setAction(visualisationAction);

		// Texture filter
		new KAction(i18n("Initialize Texture"), 0, 0,
					new TextureFilter(m_view),
					SLOT(slotActivated()), m_dynamicClient -> actionCollection(),
					"texturefilter");

		// Create the wet palette
		KisWetPaletteWidget * w = new KisWetPaletteWidget(m_view);
		Q_CHECK_PTR(w);

		w -> setCaption(i18n("Watercolors"));

		m_view->paletteManager() -> addWidget(w, "watercolor docker", krita::COLORBOX,
			INT_MAX, PALETTE_DOCKER);
		m_view->paletteManager()->showWidget("hsvwidget");

		m_view->getCanvasSubject() -> attach(w);

		connect(m_view, SIGNAL(currentColorSpaceChanged(KisLayerSP)),
				this, SLOT(colorSpaceChanged(KisLayerSP)));
	}
}

WetPlugin::~WetPlugin()
{
	delete m_dynamicClient;
}

void WetPlugin::colorSpaceChanged(KisLayerSP layer)
{
	if (!factory()) {
		kdDebug(DBG_AREA_PLUGINS) << "factory() returned NULL!" << endl;
		return;
	}
	if (layer -> colorStrategy() -> id() == KisID("WET","")) {
		kdDebug(DBG_AREA_PLUGINS) << "layer changed, wet plugin found WET layer" << endl;
		if (!factory() -> clients() . contains(m_dynamicClient)) {
			factory() -> addClient(m_dynamicClient);
		} else {
			kdDebug(DBG_AREA_PLUGINS) << "Already merged with the client" << endl;
		}
	} else {
		kdDebug(DBG_AREA_PLUGINS) << "layer changed, wet plugin found non-WET layer" << endl;
		if (factory() -> clients() . contains(m_dynamicClient)) {
			factory() -> removeClient(m_dynamicClient);
		} else {
			kdDebug(DBG_AREA_PLUGINS) << "Already disconnected from the client" << endl;
		}
	}
}

#include "wet_plugin.moc"
