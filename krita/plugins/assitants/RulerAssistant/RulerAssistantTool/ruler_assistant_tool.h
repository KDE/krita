#ifndef _RULERASSISTANTTOOL_H_
#define _RULERASSISTANTTOOL_H_

#include <kparts/plugin.h>

class KisView;

class RulerAssistantToolPlugin : public KParts::Plugin
{
    Q_OBJECT
public:
    RulerAssistantToolPlugin(QObject *parent, const QStringList &);
    virtual ~RulerAssistantToolPlugin();

private:

    KisView * m_view;

};

#endif
