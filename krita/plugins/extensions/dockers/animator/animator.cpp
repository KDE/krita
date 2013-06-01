#include "animator.h"

#include <stdlib.h>

#include <QTimer>

#include <kactioncollection.h>
#include <kcomponentdata.h>
#include <kis_debug.h>
#include <kpluginfactory.h>
#include <klocale.h>
#include <kstandarddirs.h>

#include <KoDockFactoryBase.h>

#include "kis_config.h"
#include "kis_cursor.h"
#include "kis_global.h"
#include "kis_types.h"
#include "kis_view2.h"

#include "animator_dock.h"
#include "onionskin_dock.h"
#include <KoDockRegistry.h>

K_PLUGIN_FACTORY(AnimatorPluginFactory, registerPlugin<AnimatorPlugin>();)
K_EXPORT_PLUGIN(AnimatorPluginFactory("krita"))

class AnimatorDockFactory : public KoDockFactoryBase {
public:
    AnimatorDockFactory()
    {
    }

    virtual QString id() const
    {
        return QString( "Animator" );
    }

    virtual Qt::DockWidgetArea defaultDockWidgetArea() const
    {
        return Qt::TopDockWidgetArea;
    }

    virtual QDockWidget* createDockWidget()
    {
        AnimatorDock * dockWidget = new AnimatorDock();
        
        dockWidget->setObjectName(id());

        return dockWidget;
    }

    DockPosition defaultDockPosition() const
    {
        return DockMinimized;
    }

};

class OnionSkinDockFactory : public KoDockFactoryBase{
public:
    OnionSkinDockFactory(){
    }

    virtual QString id() const{
        return QString("Onion Skin");
    }

    virtual Qt::DockWidgetArea defaultDockWidgetArea() const{
        return Qt::LeftDockWidgetArea;
    }

    virtual QDockWidget* createDockWidget(){
        OnionSkinDock* dockWidget = new OnionSkinDock();
        dockWidget->setObjectName(id());
        return dockWidget;
    }

    DockPosition defaultDockPosition() const{
        return DockMinimized;
    }
};

AnimatorPlugin::AnimatorPlugin(QObject *parent, const QVariantList &)
    : QObject(parent)
{
    KoDockRegistry::instance()->add(new AnimatorDockFactory());
    KoDockRegistry::instance()->add(new OnionSkinDockFactory());
}

AnimatorPlugin::~AnimatorPlugin()
{
    m_view = 0;
}

#include "animator.moc"
