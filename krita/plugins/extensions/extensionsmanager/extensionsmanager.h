#ifndef _EXTENSIONSMANAGER_H_
#define _EXTENSIONSMANAGER_H_

#include <kparts/plugin.h>

class KisView2;

/**
 * Template of view plugin
 */
class ExtensionsManagerPlugin : public KParts::Plugin
{
    Q_OBJECT
public:
    ExtensionsManagerPlugin(QObject *parent, const QStringList &);
    virtual ~ExtensionsManagerPlugin();

private slots:

    void slotMyAction();

private:

    KisView2 * m_view;

};

#endif // ExtensionsManagerPlugin_H
