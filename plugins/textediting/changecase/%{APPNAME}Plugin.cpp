#include "%{APPNAME}Plugin.h"
#include "%{APPNAME}Factory.h"

#include <KPluginFactory>
#include <KPluginLoader>

#include <KoTextEditingRegistry.h>

K_PLUGIN_FACTORY( %{APPNAME}PluginFactory, registerPlugin<%{APPNAME}Plugin>(); )
K_EXPORT_PLUGIN( %{APPNAME}PluginFactory("%{APPNAME}Plugin") )

%{APPNAME}Plugin::%{APPNAME}Plugin( QObject *parent, const QVariantList& )
    : QObject(parent)
{
    KoTextEditingRegistry::instance()->add( new %{APPNAME}Factory(parent));
}

#include "%{APPNAME}Plugin.moc"
