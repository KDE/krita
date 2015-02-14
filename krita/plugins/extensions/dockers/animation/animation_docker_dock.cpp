
#include "animation_docker_dock.h"

#include "kis_canvas2.h"
#include "kis_image.h"
#include <klocale.h>
#include <KoIcon.h>
#include "KisViewManager.h"
#include "kis_paint_layer.h"

#include "ui_wdg_animation.h"

AnimationDockerDock::AnimationDockerDock()
    : QDockWidget(i18n("Animation"))
    , m_canvas(0)
    , m_animationWidget(new Ui_WdgAnimation)
{
    QWidget* mainWidget = new QWidget(this);
    setWidget(mainWidget);

    m_animationWidget->setupUi(mainWidget);

    m_animationWidget->btnPreviousFrame->setIcon(themedIcon("prevframe"));
    m_animationWidget->btnPreviousFrame->setIconSize(QSize(22, 22));

    m_animationWidget->btnNextFrame->setIcon(themedIcon("nextframe"));
    m_animationWidget->btnNextFrame->setIconSize(QSize(22, 22));

    m_animationWidget->btnAddKeyframe->setIcon(themedIcon("addblankframe"));
    m_animationWidget->btnAddKeyframe->setIconSize(QSize(22, 22));

    connect(m_animationWidget->btnAddKeyframe, SIGNAL(clicked()), this, SLOT(slotAddBlankFrame()));
    connect(m_animationWidget->btnPreviousFrame, SIGNAL(clicked()), this, SLOT(slotPreviousFrame()));
    connect(m_animationWidget->btnNextFrame, SIGNAL(clicked()), this, SLOT(slotNextFrame()));
}

void AnimationDockerDock::setCanvas(KoCanvasBase * canvas)
{
    if(m_canvas == canvas)
        return;

    setEnabled(canvas != 0);

    if (m_canvas) {
        m_canvas->disconnectCanvasObserver(this);
        m_canvas->image()->disconnect(this);
    }

    m_canvas = dynamic_cast<KisCanvas2*>(canvas);
}

void AnimationDockerDock::unsetCanvas()
{
    setEnabled(false);
    m_canvas = 0;
}

void AnimationDockerDock::slotAddBlankFrame()
{
    KisNodeSP node = m_canvas->viewManager()->activeNode();
    if (!node) return;

    if (node->inherits("KisPaintLayer")) {
        KisPaintLayer *layer = qobject_cast<KisPaintLayer*>(node.data());

        layer->addBlankFrame(m_canvas->image()->currentTime());
    }
}

void AnimationDockerDock::slotPreviousFrame()
{
    int time = m_canvas->image()->currentTime() - 1;
    m_canvas->image()->seekToTime(time);

    m_animationWidget->lblInfo->setText("Frame: " + QString::number(time));
}

void AnimationDockerDock::slotNextFrame()
{
    int time = m_canvas->image()->currentTime() + 1;
    m_canvas->image()->seekToTime(time);

    m_animationWidget->lblInfo->setText("Frame: " + QString::number(time));
}

#include "animation_docker_dock.moc"
