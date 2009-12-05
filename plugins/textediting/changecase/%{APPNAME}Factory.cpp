#include "%{APPNAME}Factory.h"
#include "%{APPNAME}.h"

#include <KLocale>

%{APPNAME}Factory::%{APPNAME}Factory(QObject *parent)
    : KoTextEditingFactory(parent, "kotext%{APPNAMELC}")
{
    setShowInMenu(true);
    setTitle(i18n ("%{APPNAME}...") );
}

KoTextEditingPlugin *%{APPNAME}Factory::create() const
{
    return new %{APPNAME}();
}
