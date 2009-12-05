#ifndef %{APPNAMEUC}FACTORY_H
#define %{APPNAMEUC}FACTORY_H

#include <KoTextEditingFactory.h>

class KoTextEditingPlugin;

class %{APPNAME}Factory : public KoTextEditingFactory
{
public:
    explicit %{APPNAME}Factory(QObject *parent);
    ~%{APPNAME}Factory() {}

    KoTextEditingPlugin *create() const;
};

#endif
