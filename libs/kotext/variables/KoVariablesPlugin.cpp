#include <KoVariableRegistry.h>

#include "KoVariablesPlugin.h"
#include "KoDateVariableFactory.h"
#include <kgenericfactory.h>

K_EXPORT_COMPONENT_FACTORY(textvariables,
                           KGenericFactory<KoVariablesPlugin>( "VariablesPlugin" ) )

KoVariablesPlugin::KoVariablesPlugin( QObject *parent, const QStringList& )
    : QObject(parent)
{
    KoVariableRegistry::instance()->add( new KoDateVariableFactory( parent));
}

#include "KoVariablesPlugin.moc"

