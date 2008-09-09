#ifndef _RULERASSISTANTVIEW_H_
#define _RULERASSISTANTVIEW_H_

#include <kparts/plugin.h>

class KisView2;

/**
 * Template of view plugin
 */
class RulerAssistantViewPlugin : public KParts::Plugin
{
    Q_OBJECT
public:
    RulerAssistantViewPlugin(QObject *parent, const QStringList &);
    virtual ~RulerAssistantViewPlugin();

private slots:

    void slotMyAction();

private:

    KisView2 * m_view;

};

#endif // RulerAssistantViewPlugin_H
