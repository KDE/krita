/*
 * Copyright (C)  Hans Bakker <hansmbakker@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "SpaceNavigatorPlugin.h"

#include <stdlib.h>

#include <kactioncollection.h>
#include <kcomponentdata.h>
#include <kis_debug.h>
#include <kgenericfactory.h>
#include <klocale.h>
#include <kstandarddirs.h>

#include "kis_config.h"
#include "kis_cursor.h"
#include "kis_global.h"
#include "kis_types.h"
#include "kis_view2.h"

typedef KGenericFactory<SpaceNavigatorPlugin> SpaceNavigatorPluginFactory;
K_EXPORT_COMPONENT_FACTORY( kritaspacenav, SpaceNavigatorPluginFactory( "krita" ) )


SpaceNavigatorPlugin::SpaceNavigatorPlugin(QObject *parent, const QStringList &)
    : KParts::Plugin(parent)
{
    if ( parent->inherits("KisView2") )
    {
        thread = new PollingThread();
	thread->polling=false;

	thread->m_view = (KisView2*) parent;

	thread->m_controller = thread->m_view->canvasController();
	thread->m_x11info = thread->m_view->canvas()->x11Info();




	setComponentData(SpaceNavigatorPluginFactory::componentData());

        setXMLFile(KStandardDirs::locate("data","kritaplugins/spacenav.rc"), true);

        KAction *action  = new KAction(i18n("&Toggle polling of a 3dconnexion device"), this);
        actionCollection()->addAction("spacenav", action );
        connect(action, SIGNAL(triggered()), this, SLOT(slotTogglePolling()));
    }
}

SpaceNavigatorPlugin::~SpaceNavigatorPlugin()
{
    thread->m_view = 0;
    thread->m_controller = 0;
    spnav_close();
}

void SpaceNavigatorPlugin::slotTogglePolling()
{
	if(!thread->polling)
	{
		if(spnav_open() == -1) {
		//if(spnav_x11_open(thread->m_x11info.display(), thread->m_view->effectiveWinId()) == -1) {
		  fprintf(stderr, "failed to connect to the space navigator daemon\n");
		}
		thread->polling=true;
		thread->start();
	}
	else
	{
		spnav_close();
		thread->polling=false;
	}
}

#include "spacenav.moc"
