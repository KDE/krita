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
#include <kis_doc.h>
#include <kis_image.h>
#include <kis_layer.h>
#include <kis_global.h>
#include <kis_types.h>
#include <kis_view.h>
#include <kis_colorspace_registry.h>
#include <kis_tool_registry.h>
#include <kis_paintop_registry.h>
#include <kis_canvas_subject.h>

#include "wet_plugin.h"
#include "kis_wet_palette_widget.h"
#include "kis_colorspace_wet.h"
#include "kis_wetop.h"
#include "kis_tool_wet_brush.h"
#include "kis_wetness_visualisation_filter.h"
#include "kis_texture_filter.h"

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

	// This is not a gui plugin; only load it when the doc is created.
	if ( parent->inherits("KisFactory") )
	{
		m_colorSpaceWet = new KisColorSpaceWet();
		Q_CHECK_PTR(m_colorSpaceWet);
		// colorspace
		KisColorSpaceRegistry::instance() -> add(m_colorSpaceWet);
		// wet brush op
		KisPaintOpRegistry::instance() -> add(new KisWetOpFactory);
	}
	else if (parent -> inherits("KisView"))
	{
		m_view = dynamic_cast<KisView*>(parent);
		// Create the wet brush paint tool
		m_view -> toolRegistry() -> add(new KisToolWetBrushFactory( actionCollection() ));
		

		// Create the wet palette
		KisWetPaletteWidget * w = new KisWetPaletteWidget(m_view);
		Q_CHECK_PTR(w);

		w -> setCaption(i18n("Paints"));

		m_view -> paletteManager() -> addWidget(actionCollection(), w,
			"watercolor docker", krita::PAINTBOX, INT_MAX, PALETTE_TOOLBOX);
		//i18n("Watercolor Paint Options")
		
		m_view -> getCanvasSubject() -> attach(w);
		
		// Wetness visualisation
		WetnessVisualisationFilter* wf = new WetnessVisualisationFilter(m_view);
		wf -> setAction(new KToggleAction(i18n("Wetness Visualisation"), 0, 0, wf,
						SLOT(slotActivated()), actionCollection(), "wetnessvisualisation"));
		
		// Texture filter
		(void) new KAction(i18n("Initialize Texture"), 0, 0, new TextureFilter(m_view),
						SLOT(slotActivated()), actionCollection(), "texturefilter");
	}

}

WetPlugin::~WetPlugin()
{
}

#include "wet_plugin.moc"
