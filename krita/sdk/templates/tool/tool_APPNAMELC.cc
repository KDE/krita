
#include <kpluginfactory.h>

#include <KoToolRegistry.h>

#include "tool_%{APPNAMELC}.h"
#include "kis_tool_%{APPNAMELC}.h"

K_PLUGIN_FACTORY(%{APPNAME}PluginFactory, registerPlugin<%{APPNAME}Plugin>();)
K_EXPORT_PLUGIN(%{APPNAME}PluginFactory("krita"))

%{APPNAME}Plugin::%{APPNAME}Plugin(QObject *parent, const QVariantList &)
    : QObject(parent)
{
    KoToolRegistry * r = KoToolRegistry::instance();
    r->add(new KisTool%{APPNAME}Factory(r));
}

%{APPNAME}Plugin::~%{APPNAME}Plugin()
{
}

#include "tool_%{APPNAMELC}.moc"
