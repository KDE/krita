#include "animation_docker.h"
#include "animation_docker_dock.h"

#include <QVariantList>
#include <kpluginfactory.h>
#include <KoDockFactoryBase.h>
#include "KisViewManager.h"
#include <KoDockRegistry.h>

K_PLUGIN_FACTORY(AnimationDockerPluginFactory, registerPlugin<AnimationDockerPlugin>();)
K_EXPORT_PLUGIN(AnimationDockerPluginFactory( "krita" ) )

class AnimationDockerDockFactory : public KoDockFactoryBase {
public:
    AnimationDockerDockFactory()
    {
    }

    virtual QString id() const
    {
        return QString( "AnimationDocker" );
    }

    virtual Qt::DockWidgetArea defaultDockWidgetArea() const
    {
        return Qt::RightDockWidgetArea;
    }

    virtual QDockWidget * createDockWidget()
    {
        AnimationDockerDock *dockWidget = new AnimationDockerDock();
        dockWidget->setObjectName(id());

        return dockWidget;
    }

    DockPosition defaultDockPosition() const
    {
        return DockMinimized;
    }
private:


};

AnimationDockerPlugin::AnimationDockerPlugin(QObject *parent, const QVariantList &)
    : QObject(parent)
{
    KoDockRegistry::instance()->add(new AnimationDockerDockFactory());
}

AnimationDockerPlugin::~AnimationDockerPlugin()
{
    m_view = 0;
}

#include "animation_docker.moc"
