
#include "koChart.h"

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
    QObject *ch = QObject::child( 0L, "KoChart::WizardExtension" );
    if ( !ch )
        return 0;
    return static_cast<WizardExtension *>( ch );
}

#include "koChart.moc"
