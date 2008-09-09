#include <stdlib.h>
#include <vector>

#include <qpoint.h>

#include <klocale.h>
#include <kiconloader.h>
#include <kcomponentdata.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kis_debug.h>
#include <kgenericfactory.h>

#include <kis_global.h>
#include <kis_types.h>
#include <KoToolRegistry.h>

#include "ruler_assistant_tool.h"
#include "kis_ruler_assistant_tool.h"


typedef KGenericFactory<RulerAssistantToolPlugin> RulerAssistantToolFactory;
K_EXPORT_COMPONENT_FACTORY( kritarulerassistanttool, RulerAssistantToolFactory( "krita" ) )


RulerAssistantToolPlugin::RulerAssistantToolPlugin(QObject *parent, const QStringList &)
    : KParts::Plugin(parent)
{
    kDebug() << "RulerAssistantToolPlugin";
    setComponentData(RulerAssistantToolFactory::componentData());

    KoToolRegistry * r = KoToolRegistry::instance();
    r->add(new KisRulerAssistantToolFactory(r, QStringList()));
}

RulerAssistantToolPlugin::~RulerAssistantToolPlugin()
{
}

#include "ruler_assistant_tool.moc"
