#include "%{APPNAMELC}.h"

#include <stdlib.h>

#include <QTimer>

#include <kactioncollection.h>
#include <kcomponentdata.h>
#include <kis_debug.h>
#include <kpluginfactory.h>
#include <klocale.h>
#include <kstandarddirs.h>

#include <KoDockFactory.h>

#include "kis_config.h"
#include "kis_cursor.h"
#include "kis_global.h"
#include "kis_types.h"
#include "kis_view2.h"

#include "%{APPNAMELC}_dock.h"

K_PLUGIN_FACTORY(%{APPNAME}PluginFactory, registerPlugin<%{APPNAME}Plugin>();)
K_EXPORT_PLUGIN(%{APPNAME}PluginFactory("krita"))

class %{APPNAME}DockFactory : public KoDockFactory {
public:
    %{APPNAME}DockFactory(KisView2 * view)
        : m_view( view )
    {
    }

    virtual QString id() const
    {
        return QString( "%{APPNAME}" );
    }

    virtual Qt::DockWidgetArea defaultDockWidgetArea() const
    {
        return Qt::RightDockWidgetArea;
    }

    virtual QDockWidget* createDockWidget()
    {
        %{APPNAME}Dock * dockWidget = new %{APPNAME}Dock(m_view);
        
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


%{APPNAME}Plugin::%{APPNAME}Plugin(QObject *parent, const QVariantList &)
    : KParts::Plugin(parent)
{
    dbgPlugins << "%{APPNAME}Plugin";
    if ( parent->inherits("KisView2") )
    {
        m_view = (KisView2*) parent;

        setComponentData(%{APPNAME}PluginFactory::componentData());
        %{APPNAME}DockFactory dockFactory( m_view);
        m_view->createDockWidget( &dockFactory );
    }
}

%{APPNAME}Plugin::~%{APPNAME}Plugin()
{
    m_view = 0;
}

#include "%{APPNAMELC}.moc"
