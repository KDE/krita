#include "%{APPNAMELC}_dock.h"

#include <kis_view2.h>

#include <QLabel>

%{APPNAME}Dock::%{APPNAME}Dock( KisView2 *view ) : QDockWidget(i18n("%{APPNAME}")), m_view(view)
{
    m_label = new QLabel(this);
    setWidget( m_label );
}

#include "%{APPNAMELC}_dock.moc"
