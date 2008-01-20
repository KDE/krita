#ifndef _%{APPNAMEUC}_H_
#define _%{APPNAMEUC}_H_

#include <kparts/plugin.h>

class KisView;

class %{APPNAME}Plugin : public KParts::Plugin
{
    Q_OBJECT
public:
    %{APPNAME}Plugin(QObject *parent, const QStringList &);
    virtual ~%{APPNAME}Plugin();

private:

    KisView * m_view;

};

#endif
