#ifndef _%{APPNAMEUC}_H_
#define _%{APPNAMEUC}_H_

#include <QVariant>

#include <kparts/plugin.h>

class KisView2;

/**
 * Template of view plugin
 */
class %{APPNAME}Plugin : public KParts::Plugin
{
    Q_OBJECT
public:
    %{APPNAME}Plugin(QObject *parent, const QVariantList &);
    virtual ~%{APPNAME}Plugin();

private slots:

    void slotMyAction();

private:

    KisView2 * m_view;

};

#endif // %{APPNAME}Plugin_H
