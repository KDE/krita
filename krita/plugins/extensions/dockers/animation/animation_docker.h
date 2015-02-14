#ifndef _OVERVIEW_DOCKER_H_
#define _OVERVIEW_DOCKER_H_

#include <QObject>
#include <QVariantList>

class KisViewManager;

class AnimationDockerPlugin : public QObject
{
    Q_OBJECT
    public:
        AnimationDockerPlugin(QObject *parent, const QVariantList &);
        virtual ~AnimationDockerPlugin();
    private:
        KisViewManager* m_view;
};

#endif
