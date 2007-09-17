/*
 *  Copyright (c) 2007 Cyrille Berger (cberger@cberger.net)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "tonemapping.h"

#include <stdlib.h>

#include <kactioncollection.h>
#include <kcomponentdata.h>
#include <kdebug.h>
#include <kgenericfactory.h>
#include <klocale.h>
#include <kstandarddirs.h>

#include "kis_config.h"
#include "kis_cursor.h"
#include "kis_global.h"
#include "kis_types.h"
#include "kis_view2.h"

typedef KGenericFactory<tonemappingPlugin> tonemappingPluginFactory;
K_EXPORT_COMPONENT_FACTORY( kritatonemapping, tonemappingPluginFactory( "krita" ) )


tonemappingPlugin::tonemappingPlugin(QObject *parent, const QStringList &)
    : KParts::Plugin(parent)
{
    if ( parent->inherits("KisView2") )
    {
        m_view = (KisView2*) parent;

        setComponentData(tonemappingPluginFactory::componentData());

        setXMLFile(KStandardDirs::locate("data","kritaplugins/tonemapping.rc"), true);

        m_toneMappingAction  = new KAction(i18n("Tonemapping"), this);
        actionCollection()->addAction("tonemapping", m_toneMappingAction );
        connect(m_toneMappingAction, SIGNAL(triggered()), this, SLOT(slotToneMapping()));
    }
}

tonemappingPlugin::~tonemappingPlugin()
{
    m_view = 0;
}

void tonemappingPlugin::slotToneMapping()
{
}

#include "tonemapping.moc"
