
#include "koChart.h"

#include <qobjectlist.h>

using namespace KoChart;

WizardExtension::WizardExtension( Part *part, const char *name )
    : QObject( part, name )
{
    m_part = part;
}

WizardExtension::~WizardExtension()
{
}

Part::Part( QWidget *parentWidget, const char *widgetName,
            QObject *parent, const char *name,
            bool singleViewMode )
    : KoDocument( parentWidget, widgetName, parent, name, singleViewMode )
{
}

Part::~Part()
{
}

WizardExtension *Part::wizardExtension()
{
    QObjectListIt it( *QObject::children() );
    for (; it.current(); ++it )
        if ( it.current()->inherits( "KoChart::WizardExtension" ) )
            return static_cast<WizardExtension *>( it.current() );

    return 0;
}

#include "koChart.moc"
