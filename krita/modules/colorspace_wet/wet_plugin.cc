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

#include <klocale.h>
#include <kiconloader.h>
#include <kinstance.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <ktempfile.h>
#include <kdebug.h>
#include <kgenericfactory.h>

#include <kotooldockmanager.h>
#include <kotooldockbase.h>
#include <koMainWindow.h>

#include <kis_factory.h>
#include <kis_doc.h>
#include <kis_image.h>
#include <kis_layer.h>
#include <kis_global.h>
#include <kis_types.h>
#include <kis_view.h>
#include <kis_colorspace_registry.h>
#include <kis_dockframedocker.h>
#include <kis_canvas_subject.h>

#include "wet_plugin.h"
#include "kis_wet_palette_widget.h"
#include "kis_colorspace_wet.h"

typedef KGenericFactory<WetPlugin> WetPluginFactory;
K_EXPORT_COMPONENT_FACTORY( kritawetplugin, WetPluginFactory( "kritacore" ) )


WetPlugin::WetPlugin(QObject *parent, const char *name, const QStringList &)
	: KParts::Plugin(parent, name)
{
       	setInstance(WetPluginFactory::instance());

 	kdDebug() << "Wet Color model plugin. Class: "
 		  << className()
 		  << ", Parent: "
 		  << parent -> className()
 		  << "\n";

	// This is not a gui plugin; only load it when the doc is created.
	if ( parent->inherits("KisFactory") )
	{
		m_colorSpaceWet = new KisColorSpaceWet();
		Q_CHECK_PTR(m_colorSpaceWet);
		KisColorSpaceRegistry::instance() -> add(m_colorSpaceWet);
	}
	else if (parent -> inherits("KisView"))
	{
		m_view = dynamic_cast<KisView*>(parent);
		// Create the wet brush
		// Create the wet palette
		m_docker = new KisDockFrameDocker(m_view, "watercolor docker");
		Q_CHECK_PTR(m_docker);
		m_docker -> setCaption(i18n("Watercolor Paint Options"));

		KisWetPaletteWidget * w = new KisWetPaletteWidget(m_docker);
		Q_CHECK_PTR(w);

		w -> setCaption(i18n("Paints"));
		m_docker -> plug(w);
		m_view -> mainWindow()->addDockWindow( m_docker, DockRight );
		m_view -> getCanvasSubject() -> attach(w);
		m_docker -> show();
	}

}

WetPlugin::~WetPlugin()
{
}

#include "wet_plugin.moc"
