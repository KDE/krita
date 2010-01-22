/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
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

#include "digitalmixer.h"

#include <stdlib.h>

#include <QTimer>

#include <kactioncollection.h>
#include <kcomponentdata.h>
#include <kis_debug.h>
#include <kpluginfactory.h>
#include <klocale.h>
#include <kstandarddirs.h>

#include <KoDockFactoryBase.h>

#include "kis_config.h"
#include "kis_cursor.h"
#include "kis_global.h"
#include "kis_types.h"
#include "kis_view2.h"

#include "digitalmixer_dock.h"

K_PLUGIN_FACTORY(DigitalMixerPluginFactory, registerPlugin<DigitalMixerPlugin>();)
K_EXPORT_PLUGIN(DigitalMixerPluginFactory( "krita" ) )

class DigitalMixerDockFactory : public KoDockFactoryBase {
public:
    DigitalMixerDockFactory(KisView2 * view)
        : m_view( view )
    {
    }

    virtual QString id() const
    {
        return QString( "DigitalMixer" );
    }

    virtual Qt::DockWidgetArea defaultDockWidgetArea() const
    {
        return Qt::RightDockWidgetArea;
    }

    virtual QDockWidget* createDockWidget()
    {
        DigitalMixerDock * dockWidget = new DigitalMixerDock(m_view);
        
        dockWidget->setObjectName(id());

        return dockWidget;
    }

    DockPosition defaultDockPosition() const
    {
        return DockMinimized;
    }
private:
    KisView2 * m_view;

};


DigitalMixerPlugin::DigitalMixerPlugin(QObject *parent, const QVariantList &)
    : QObject(parent)
{
    dbgPlugins << "DigitalMixerPlugin";
    if ( parent->inherits("KisView2") )
    {
        m_view = (KisView2*) parent;

        //setComponentData(DigitalMixerPluginFactory::componentData());
        DigitalMixerDockFactory dockFactory( m_view);
        m_view->createDockWidget( &dockFactory );
    }
}

DigitalMixerPlugin::~DigitalMixerPlugin()
{
    m_view = 0;
}

#include "digitalmixer.moc"
