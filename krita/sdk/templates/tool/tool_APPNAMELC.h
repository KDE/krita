#ifndef _%{APPNAMEUC}_H_
#define _%{APPNAMEUC}_H_

#include <QObject>
#include <QVariant>

class KisView;

class %{APPNAME}Plugin : public QObject
{
    Q_OBJECT
public:
    %{APPNAME}Plugin(QObject *parent, const QVariantList &);
    virtual ~%{APPNAME}Plugin();

private:

    KisView * m_view;

};

#endif
