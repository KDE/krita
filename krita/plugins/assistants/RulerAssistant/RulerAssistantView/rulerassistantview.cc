/*
 * Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "rulerassistantview.h"

#include <stdlib.h>

#include <kactioncollection.h>
#include <kcomponentdata.h>
#include <kis_debug.h>
#include <kgenericfactory.h>
#include <klocale.h>
#include <kstandarddirs.h>

#include "canvas/kis_canvas2.h"
#include "kis_config.h"
#include "kis_cursor.h"
#include "kis_global.h"
#include "kis_types.h"
#include "kis_view2.h"

#include "RulerAssistant.h"

#include "RulerDecoration.h"

typedef KGenericFactory<RulerAssistantViewPlugin> RulerAssistantViewPluginFactory;
K_EXPORT_COMPONENT_FACTORY( kritarulerassistantview, RulerAssistantViewPluginFactory( "krita" ) )


RulerAssistantViewPlugin::RulerAssistantViewPlugin(QObject *parent, const QStringList &)
    : KParts::Plugin(parent)
{
    if ( parent->inherits("KisView2") )
    {
        m_view = (KisView2*) parent;

        setComponentData(RulerAssistantViewPluginFactory::componentData());

        setXMLFile(KStandardDirs::locate("data","kritaplugins/rulerassistantview.rc"), true);

        RulerAssistant* ra = new RulerAssistant;
        KisPaintingAssistant::setCurrentAssistant( ra );

        RulerDecoration* hbd = new RulerDecoration( m_view, ra->ruler() );
        m_view->canvasBase()->addDecoration( hbd );
        KAction *action  = new KAction(i18n("&Show ruler assistant"), this);
        actionCollection()->addAction("ShowRulerAssitantDecoration", action );
        connect(action, SIGNAL(triggered()), hbd, SLOT(toggleVisibility()));

    }
}

RulerAssistantViewPlugin::~RulerAssistantViewPlugin()
{
    m_view = 0;
}

void RulerAssistantViewPlugin::slotMyAction()
{
  // TODO: implement your action there ! go go go !
}

#include "rulerassistantview.moc"
