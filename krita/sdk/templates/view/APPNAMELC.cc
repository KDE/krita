#include "%{APPNAMELC}.h"

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

typedef KGenericFactory < % {APPNAME} Plugin > % {APPNAME} PluginFactory;
K_EXPORT_COMPONENT_FACTORY(krita % {APPNAMELC}, % {APPNAME} PluginFactory("krita"))


% {APPNAME} Plugin:: % {APPNAME} Plugin(QObject *parent, const QStringList &)
        : KParts::Plugin(parent)
{
    if (parent->inherits("KisView2")) {
        m_view = (KisView2*) parent;

        setComponentData( % {APPNAME} PluginFactory::componentData());

        setXMLFile(KStandardDirs::locate("data", "kritaplugins/%{APPNAMELC}.rc"), true);

        KAction *action  = new KAction(i18n("&My action !"), this);
        actionCollection()->addAction("%{APPNAME}", action);
        connect(action, SIGNAL(triggered()), this, SLOT(slotMyAction()));
    }
}

% {APPNAME} Plugin::~ % {APPNAME} Plugin()
{
    m_view = 0;
}

void % {APPNAME} Plugin::slotMyAction()
{
    // TODO: implement your action there ! go go go !
}

#include "%{APPNAMELC}.moc"
