#include "extensionsmanager.h"

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

typedef KGenericFactory<ExtensionsManagerPlugin> ExtensionsManagerPluginFactory;
K_EXPORT_COMPONENT_FACTORY( kritaextensionsmanager, ExtensionsManagerPluginFactory( "krita" ) )


ExtensionsManagerPlugin::ExtensionsManagerPlugin(QObject *parent, const QStringList &)
    : KParts::Plugin(parent)
{
    if ( parent->inherits("KisView2") )
    {
        m_view = (KisView2*) parent;

        setComponentData(ExtensionsManagerPluginFactory::componentData());

        setXMLFile(KStandardDirs::locate("data","kritaplugins/extensionsmanager.rc"), true);

        KAction *action  = new KAction(i18n("&My action !"), this);
        actionCollection()->addAction("ExtensionsManager", action );
        connect(action, SIGNAL(triggered()), this, SLOT(slotMyAction()));
    }
}

ExtensionsManagerPlugin::~ExtensionsManagerPlugin()
{
    m_view = 0;
}

void ExtensionsManagerPlugin::slotMyAction()
{
  // TODO: implement your action there ! go go go !
}

#include "extensionsmanager.moc"
