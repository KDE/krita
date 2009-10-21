#include <stdlib.h>
#include <vector>

#include <qpoint.h>

#include <klocale.h>
#include <kiconloader.h>
#include <kcomponentdata.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kis_debug.h>
#include <kgenericfactory.h>

#include <kis_global.h>
#include <kis_types.h>
#include <KoToolRegistry.h>

#include "tool_%{APPNAMELC}.h"
#include "kis_tool_%{APPNAMELC}.h"


typedef KGenericFactory < % {APPNAME} Plugin > % {APPNAME} Factory;
K_EXPORT_COMPONENT_FACTORY(kritatool % {APPNAMELC}, % {APPNAME} Factory("krita"))


% {APPNAME} Plugin:: % {APPNAME} Plugin(QObject *parent, const QStringList &)
        : KParts::Plugin(parent)
{
    setComponentData( % {APPNAME} Factory::componentData());

    KoToolRegistry * r = KoToolRegistry::instance();
    r->add(new KisTool % {APPNAME} Factory(r, QStringList()));
}

% {APPNAME} Plugin::~ % {APPNAME} Plugin()
{
}

#include "tool_%{APPNAMELC}.moc"
