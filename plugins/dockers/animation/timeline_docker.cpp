/*
 *  Copyright (c) 2015 Jouni Pentikäinen <joupent@gmail.com>
 *  Copyright (c) 2020 Emmet O'Neill <emmetoneill.pdx@gmail.com>
 *  Copyright (c) 2020 Eoin O'Neill <eoinoneill1991@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "timeline_docker.h"

#include "QHBoxLayout"
#include "QLabel"
#include "QPushButton"
#include "QSpinBox"
#include "QToolButton"

#include "kis_canvas2.h"
#include "kis_image.h"
#include <KoIcon.h>
#include "KisViewManager.h"
#include "kis_paint_layer.h"
#include "KisDocument.h"
#include "kis_dummies_facade.h"
#include "kis_shape_controller.h"
#include "kis_action.h"
#include "kis_action_manager.h"
#include "kis_animation_player.h"
#include "kis_keyframe_channel.h"
#include "kis_image.h"

#include "timeline_frames_model.h"
#include "timeline_frames_view.h"
#include "kis_animation_frame_cache.h"
#include "kis_image_animation_interface.h"
#include "kis_signal_auto_connection.h"
#include "kis_node_manager.h"
#include "kis_transport_controls.h"

#include <QPointer>

TimelineDockerTitlebar::TimelineDockerTitlebar(QWidget* parent) :
    QWidget(parent)
{
    QDockWidget *dockWidget = qobject_cast<QDockWidget*>(parentWidget());

    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    QMargins margins = mainLayout->contentsMargins();
    margins.setTop(0);
    margins.setBottom(0);
    mainLayout->setContentsMargins(margins);

    mainLayout->addWidget(new QLabel("Animation Timeline", this));

    mainLayout->addSpacing(SPACING_UNIT);

    // Transport Controls...
    transport = new KisTransportControls(this);
    mainLayout->addWidget(transport);

    mainLayout->addSpacing(SPACING_UNIT);

    // Frame Counter...
    frameCounter = new QSpinBox(this);
    frameCounter->setPrefix("#  ");
    frameCounter->setRange(0, MAX_FRAMES);
    mainLayout->addWidget(frameCounter);

    mainLayout->addSpacing(SPACING_UNIT);

    {   // Frame ops...
        QHBoxLayout *layout = new QHBoxLayout(this);
        layout->setSpacing(0);
        layout->setContentsMargins(0,0,0,0);

        //add keyframe..
        btnAddKeyframe = new QToolButton(this);
        layout->addWidget(btnAddKeyframe);

        btnDuplicateKeyframe = new QToolButton(this);
        layout->addWidget(btnDuplicateKeyframe);

        //remove keyframe..
        btnRemoveKeyframe = new QToolButton(this);
        layout->addWidget(btnRemoveKeyframe);

        QWidget *widget = new QWidget();
        widget->setLayout(layout);

        mainLayout->addWidget(widget);
    }

    mainLayout->addStretch();

    {   // Menus...
        QHBoxLayout *layout = new QHBoxLayout(this);
        layout->setSpacing(0);
        layout->setContentsMargins(0,0,0,0);

        layout->addWidget(new QPushButton(KisIconUtils::loadIcon("onion_skin_options"), "", this)); //onion skins menu.
        layout->addWidget(new QPushButton(KisIconUtils::loadIcon("audio-none"), "", this)); //audio menu.
        layout->addWidget(new QPushButton(KisIconUtils::loadIcon("configure"),"", this)); //settings menu.

        QWidget *widget = new QWidget();
        widget->setLayout(layout);

        mainLayout->addWidget(widget);
    }

    mainLayout->addSpacing(SPACING_UNIT);

    {   //Docker Controls...
        QHBoxLayout *layout = new QHBoxLayout(this);
        layout->setSpacing(0);
        layout->setContentsMargins(0,0,0,0);

        {   // Float button...
            QPushButton *button = new QPushButton(style()->standardIcon(QStyle::SP_TitleBarNormalButton), "", this); //TODO: Style-correct float icon.
            button->setFlat(true);
            connect(button, &QPushButton::clicked, [this, dockWidget](){
                dockWidget->setFloating(!dockWidget->isFloating());
            } );

            layout->addWidget(button);
        }

        {   // Close button...
            QPushButton *button = new QPushButton(style()->standardIcon(QStyle::SP_DockWidgetCloseButton), "", this); //TODO: Style-correct close icon.
            button->setFlat(true);
            connect(button, SIGNAL(clicked(bool)), dockWidget, SLOT(close()));
            layout->addWidget(button);
        }

        QWidget *widget = new QWidget();
        widget->setLayout(layout);

        mainLayout->addWidget(widget);
    }

    setLayout(mainLayout);
}

TimelineDockerTitlebar::~TimelineDockerTitlebar()
{

}

struct TimelineDocker::Private
{
    Private(QWidget *parent)
        : model(new TimelineFramesModel(parent)),
          view(new TimelineFramesView(parent)),
          titlebar(new TimelineDockerTitlebar(parent))
    {
        view->setModel(model);
    }

    TimelineFramesModel *model;
    TimelineFramesView *view;
    TimelineDockerTitlebar *titlebar;

    QPointer<KisCanvas2> canvas;

    KisSignalAutoConnectionsStore canvasConnections;
};

TimelineDocker::TimelineDocker()
    : QDockWidget(i18n("Timeline"))
    , m_d(new Private(this))
{
    setWidget(m_d->view);
    setTitleBarWidget(m_d->titlebar);
}

TimelineDocker::~TimelineDocker()
{
}

struct NodeManagerInterface : TimelineFramesModel::NodeManipulationInterface
{
    NodeManagerInterface(KisNodeManager *manager) : m_manager(manager) {}

    KisLayerSP addPaintLayer() const override {
        return m_manager->createPaintLayer();
    }

    void removeNode(KisNodeSP node) const override {
        m_manager->removeSingleNode(node);
    }

    bool setNodeProperties(KisNodeSP node, KisImageSP image, KisBaseNode::PropertyList properties) const override
    {
        return m_manager->trySetNodeProperties(node, image, properties);
    }

private:
    KisNodeManager *m_manager;
};

void TimelineDocker::setCanvas(KoCanvasBase * canvas)
{
    if (m_d->canvas == canvas) return;

    if (m_d->model->hasConnectionToCanvas()) {
        m_d->canvasConnections.clear();
        m_d->model->setDummiesFacade(0, 0, 0);
        m_d->model->setFrameCache(0);
        m_d->model->setAnimationPlayer(0);
        m_d->model->setNodeManipulationInterface(0);
    }

    if (m_d->canvas) {
        m_d->canvas->disconnectCanvasObserver(this);
        m_d->canvas->animationPlayer()->disconnect(this);
    }


    m_d->canvas = dynamic_cast<KisCanvas2*>(canvas);
    setEnabled(m_d->canvas != 0);

    if(m_d->canvas) {
        KisDocument *doc = static_cast<KisDocument*>(m_d->canvas->imageView()->document());
        KisShapeController *kritaShapeController = dynamic_cast<KisShapeController*>(doc->shapeController());
        m_d->model->setDummiesFacade(kritaShapeController,
                                     m_d->canvas->image(),
                                     m_d->canvas->viewManager()->nodeManager()->nodeDisplayModeAdapter());

        updateFrameCache();
        m_d->model->setAnimationPlayer(m_d->canvas->animationPlayer());

        m_d->model->setNodeManipulationInterface(
            new NodeManagerInterface(m_d->canvas->viewManager()->nodeManager()));

        m_d->canvasConnections.addConnection(
            m_d->canvas->viewManager()->nodeManager(), SIGNAL(sigNodeActivated(KisNodeSP)),
            m_d->model, SLOT(slotCurrentNodeChanged(KisNodeSP)));

        m_d->canvasConnections.addConnection(
            m_d->model, SIGNAL(requestCurrentNodeChanged(KisNodeSP)),
            m_d->canvas->viewManager()->nodeManager(), SLOT(slotNonUiActivatedNode(KisNodeSP)));

        m_d->model->slotCurrentNodeChanged(m_d->canvas->viewManager()->activeNode());

        m_d->canvasConnections.addConnection(
                    m_d->canvas->viewManager()->mainWindow(), SIGNAL(themeChanged()),
                    this, SLOT(handleThemeChange()));

        m_d->canvasConnections.addConnection(
                    m_d->canvas, SIGNAL(sigCanvasEngineChanged()),
                    this, SLOT(updateFrameCache()));

        //connect(m_d->canvas->animationPlayer(), SIGNAL(sigFrameChanged()), this, SLOT(slotGlobalTimeChanged()));
        //connect(m_d->canvas->animationPlayer(), SIGNAL(sigPlaybackStopped()), this, SLOT(slotGlobalTimeChanged()));
        //connect(m_d->canvas->animationPlayer(), SIGNAL(sigPlaybackStopped()), this, SLOT(updatePlayPauseIcon()));
        //connect(m_d->canvas->animationPlayer(), SIGNAL(sigPlaybackStarted()), this, SLOT(updatePlayPauseIcon()));
        //connect(m_d->canvas->animationPlayer(), SIGNAL(sigPlaybackStatisticsUpdated()), this, SLOT(updateDropFramesIcon()));
    }

}

void TimelineDocker::handleThemeChange()
{
    if (m_d->view) {
        m_d->view->slotUpdateIcons();
    }
}

void TimelineDocker::updateFrameCache()
{
    m_d->model->setFrameCache(m_d->canvas->frameCache());
}

void TimelineDocker::unsetCanvas()
{
    setCanvas(0);
}

void TimelineDocker::setViewManager(KisViewManager *view)
{
    KisActionManager *actionManager = view->actionManager();
    m_d->view->setActionManager(actionManager);
    m_d->view->setPinToTimeline(actionManager->actionByName("pin_to_timeline"));

    KisAction *action = 0;

    TimelineDockerTitlebar* titleBar = static_cast<TimelineDockerTitlebar*>(titleBarWidget());

    action = actionManager->createAction("add_blank_frame");
    titleBar->btnAddKeyframe->setDefaultAction(action);

    action = actionManager->createAction("add_duplicate_frame");
    titleBar->btnDuplicateKeyframe->setDefaultAction(action);

    action = actionManager->createAction("remove_frames");
    titleBar->btnRemoveKeyframe->setDefaultAction(action);

    action = actionManager->createAction("toggle_playback");
    action->setActivationFlags(KisAction::ACTIVE_IMAGE);
    titleBar->transport->btnPlayPause->setDefaultAction(action);
    connect(action, SIGNAL(triggered(bool)), SLOT(playPause()));

    action = actionManager->createAction("stop_playback");
    action->setActivationFlags(KisAction::ACTIVE_IMAGE);
    titleBar->transport->btnStop->setDefaultAction(action);
    connect(action, SIGNAL(triggered(bool)), SLOT(stop()));

    action = actionManager->createAction("previous_frame");
    action->setActivationFlags(KisAction::ACTIVE_IMAGE);
    titleBar->transport->btnBack->setDefaultAction(action);
    connect(action, SIGNAL(triggered(bool)), SLOT(previousFrame()));

    action = actionManager->createAction("next_frame");
    action->setActivationFlags(KisAction::ACTIVE_IMAGE);
    titleBar->transport->btnForward->setDefaultAction(action);
    connect(action, SIGNAL(triggered(bool)), SLOT(nextFrame()));

    connect(titleBar->frameCounter, SIGNAL(valueChanged(int)), SLOT(goToFrame(int)));

//    action = actionManager->createAction("stop");
//    titleBar->getTransport()->btnStop->setDefaultAction(action);
}

void TimelineDocker::playPause()
{
    if (!m_d->canvas) return;
    ENTER_FUNCTION();

    if (m_d->canvas->animationPlayer()->isPlaying()) {
        m_d->canvas->animationPlayer()->stop();
    } else {
        m_d->canvas->animationPlayer()->play();
    }

    //updatePlayPauseIcon();
}

void TimelineDocker::stop()
{
    ENTER_FUNCTION();
    if (!m_d->canvas) return;

    if (m_d->canvas->animationPlayer()->isPlaying()) {
        m_d->canvas->animationPlayer()->stop();
    }
}

void TimelineDocker::previousFrame()
{
    ENTER_FUNCTION();
    if (!m_d->canvas) return;
    KisImageAnimationInterface *animInterface = m_d->canvas->image()->animationInterface();

    int time = animInterface->currentUITime() - 1;
    if (time >= 0) {
        animInterface->requestTimeSwitchWithUndo(time);
    }
}

void TimelineDocker::nextFrame()
{
    ENTER_FUNCTION();
    if (!m_d->canvas) return;
    KisImageAnimationInterface *animInterface = m_d->canvas->image()->animationInterface();

    int time = animInterface->currentUITime() + 1;
    animInterface->requestTimeSwitchWithUndo(time);
}

void TimelineDocker::goToFrame(int frameIndex)
{
    ENTER_FUNCTION();
    if (!m_d->canvas || !m_d->canvas->image())  return;

    KisImageAnimationInterface *animInterface = m_d->canvas->image()->animationInterface();

    if (m_d->canvas->animationPlayer()->isPlaying() ||
        frameIndex == animInterface->currentUITime()) {
        return;
    }

    animInterface->requestTimeSwitchWithUndo(frameIndex);
}
