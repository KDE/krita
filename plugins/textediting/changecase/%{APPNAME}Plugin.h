#ifndef %{APPNAMEUC}PLUGIN_H
#define %{APPNAMEUC}PLUGIN_H

#include <QObject>
#include <QVariant>

class %{APPNAME}Plugin : public QObject
{
    Q_OBJECT
public:
    %{APPNAME}Plugin(QObject * parent, const QVariantList&);
    ~%{APPNAME}Plugin() {}
};

#endif

