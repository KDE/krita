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

#include <QPointer>
#include "QHBoxLayout"
#include "QVBoxLayout"
#include "QFormLayout"
#include "QLabel"
#include "QPushButton"
#include "QToolButton"
#include "QMenu"
#include "QWidgetAction"

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
#include "kis_time_range.h"
#include "kis_animation_frame_cache.h"
#include "kis_image_animation_interface.h"
#include "kis_signal_auto_connection.h"
#include "kis_node_manager.h"
#include "kis_transport_controls.h"
#include "kis_int_parse_spin_box.h"
#include "kis_slider_spin_box.h"
#include "kis_signals_blocker.h"

TimelineDockerTitleBar::TimelineDockerTitleBar(QWidget* parent) :
    KisUtilityTitleBar(new QLabel(i18n("Animation Timeline"), parent), parent)
{
    // Transport Controls...
    transport = new KisTransportControls(this);
    widgetArea->addWidget(transport);

    widgetArea->addSpacing(SPACING_UNIT);

    // Frame Counter...
    frameCounter = new KisIntParseSpinBox(this);
    frameCounter->setPrefix("#  ");
    frameCounter->setRange(0, MAX_FRAMES);
    widgetArea->addWidget(frameCounter);

    widgetArea->addSpacing(SPACING_UNIT);

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

        widgetArea->addWidget(widget);
    }

    widgetArea->addSpacing(SPACING_UNIT);

    QPushButton *btnDropFrames = new QPushButton(KisIconUtils::loadIcon("dropframe"), "", this);
    btnDropFrames->setToolTip("TODO: drop frames button"); //todo
    btnDropFrames->setDisabled(true);
    widgetArea->addWidget(btnDropFrames);

    sbSpeed = new KisSliderSpinBox(this);
    sbSpeed->setRange(25, 200);
    sbSpeed->setSingleStep(25);
    sbSpeed->setValue(100);
    sbSpeed->setPrefix("Speed: ");
    sbSpeed->setSuffix(" %");
    sbSpeed->setToolTip(i18n("Preview playback speed."));
    widgetArea->addWidget(sbSpeed);

    widgetArea->addStretch();

    {   // Menus..
        QHBoxLayout *layout = new QHBoxLayout(this);
        layout->setSpacing(0);
        layout->setContentsMargins(0,0,0,0);

        // Onion skins menu.
        btnOnionSkinsMenu = new QPushButton(KisIconUtils::loadIcon("onion_skin_options"), "", this);
        layout->addWidget(btnOnionSkinsMenu);

        // Audio menu..
        btnAudioMenu = new QPushButton(KisIconUtils::loadIcon("audio-none"), "", this);
        btnAudioMenu->setDisabled(true);
        layout->addWidget(btnAudioMenu);

        {    // Settings menu..
            //KisIconUtils::loadIcon("configure"),
            btnSettingsMenu = new QToolButton(this);
            btnSettingsMenu->setIcon(KisIconUtils::loadIcon("configure"));

            QWidget *settingsMenuWidget = new QWidget(this);
            settingsMenuWidget->setLayout(new QHBoxLayout(settingsMenuWidget));

            QWidget *fields = new QWidget(settingsMenuWidget);
            QFormLayout *fieldsLayout = new QFormLayout(settingsMenuWidget);
            fields->setLayout(fieldsLayout);

            sbStartFrame = new KisIntParseSpinBox(settingsMenuWidget);
            sbStartFrame->setMaximum(10000);
            fieldsLayout->addRow(i18n("Clip Start: "), sbStartFrame);

            sbEndFrame = new KisIntParseSpinBox(settingsMenuWidget);
            sbEndFrame->setMaximum(10000);
            fieldsLayout->addRow(i18n("Clip End: "), sbEndFrame);

            sbFrameRate = new KisIntParseSpinBox(settingsMenuWidget);
            sbFrameRate->setMinimum(0);
            sbFrameRate->setMaximum(180);
            fieldsLayout->addRow(i18n("Frame Rate: "), sbFrameRate);

            QWidget *buttons = new QWidget(settingsMenuWidget);
            buttons->setLayout(new QVBoxLayout(settingsMenuWidget));

            btnAutoFrame = new QToolButton(settingsMenuWidget);
            buttons->layout()->addWidget(btnAutoFrame);
            buttons->layout()->setAlignment(Qt::AlignTop);

            settingsMenuWidget->layout()->addWidget(fields);
            settingsMenuWidget->layout()->addWidget(buttons);

            layout->addWidget(btnSettingsMenu);

            QMenu *settingsPopMenu = new QMenu(this);
            QWidgetAction *settingsMenuAction = new QWidgetAction(this);
            settingsMenuAction->setDefaultWidget(settingsMenuWidget);
            settingsPopMenu->addAction(settingsMenuAction);

            btnSettingsMenu->setPopupMode(QToolButton::InstantPopup);
            btnSettingsMenu->setMenu(settingsPopMenu);
        }

        QWidget *widget = new QWidget();
        widget->setLayout(layout);
        widgetArea->addWidget(widget);
    }
}

struct TimelineDocker::Private
{
    Private(QWidget *parent)
        : framesModel(new TimelineFramesModel(parent)),
          framesView(new TimelineFramesView(parent)),
          titlebar(new TimelineDockerTitleBar(parent)),
          mainWindow(nullptr)
    {
        framesView->setModel(framesModel);
    }

    TimelineFramesModel *framesModel;
    TimelineFramesView *framesView;
    TimelineDockerTitleBar *titlebar;

    QPointer<KisCanvas2> canvas;

    KisSignalAutoConnectionsStore canvasConnections;
    KisMainWindow *mainWindow;
};

TimelineDocker::TimelineDocker()
    : QDockWidget(i18n("Animation Timeline"))
    , m_d(new Private(this))
{
    setWidget(m_d->framesView);
    setTitleBarWidget(m_d->titlebar);

    connect(m_d->titlebar->transport, SIGNAL(back()), this, SLOT(previousFrame()));
    connect(m_d->titlebar->transport, SIGNAL(stop()), this, SLOT(stop()));
    connect(m_d->titlebar->transport, SIGNAL(playPause()), this, SLOT(playPause()));
    connect(m_d->titlebar->transport, SIGNAL(forward()), this, SLOT(nextFrame()));

    connect(m_d->titlebar->frameCounter, SIGNAL(valueChanged(int)), SLOT(goToFrame(int)));
    connect(m_d->titlebar->sbStartFrame, SIGNAL(valueChanged(int)), SLOT(setStartFrame(int)));
    connect(m_d->titlebar->sbEndFrame, SIGNAL(valueChanged(int)), SLOT(setEndFrame(int)));
    connect(m_d->titlebar->sbFrameRate, SIGNAL(valueChanged(int)), SLOT(setFrameRate(int)));
    connect(m_d->titlebar->sbSpeed, SIGNAL(valueChanged(int)), SLOT(setPlaybackSpeed(int)));

    connect(m_d->titlebar->btnOnionSkinsMenu, &QPushButton::released, [this](){
        if (m_d->mainWindow) {
            QDockWidget *docker = m_d->mainWindow->dockWidget("OnionSkinsDocker");
            if (docker) {
                docker->setVisible(!docker->isVisible());
            }
        }
    });
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

    if (m_d->framesModel->hasConnectionToCanvas()) {
        m_d->canvasConnections.clear();
        m_d->framesModel->setDummiesFacade(0, 0, 0);
        m_d->framesModel->setFrameCache(0);
        m_d->framesModel->setAnimationPlayer(0);
        m_d->framesModel->setNodeManipulationInterface(0);
    }

    if (m_d->canvas) {
        m_d->canvas->disconnectCanvasObserver(this);
        m_d->canvas->animationPlayer()->disconnect(this);

        if(m_d->canvas->image()) {
            m_d->canvas->image()->animationInterface()->disconnect(this);
        }
    }


    m_d->canvas = dynamic_cast<KisCanvas2*>(canvas);
    setEnabled(m_d->canvas != 0);

    if(m_d->canvas) {
        KisDocument *doc = static_cast<KisDocument*>(m_d->canvas->imageView()->document());
        KisShapeController *kritaShapeController = dynamic_cast<KisShapeController*>(doc->shapeController());
        m_d->framesModel->setDummiesFacade(kritaShapeController,
                                     m_d->canvas->image(),
                                     m_d->canvas->viewManager()->nodeManager()->nodeDisplayModeAdapter());

        updateFrameCache();

        {
            KisSignalsBlocker blocker(m_d->titlebar->sbStartFrame,
                                      m_d->titlebar->sbEndFrame,
                                      m_d->titlebar->sbFrameRate);

            KisImageAnimationInterface *animinterface = m_d->canvas->image()->animationInterface();
            m_d->titlebar->sbStartFrame->setValue(animinterface->fullClipRange().start());
            m_d->titlebar->sbEndFrame->setValue(animinterface->fullClipRange().end());
            m_d->titlebar->sbFrameRate->setValue(animinterface->framerate());
            m_d->titlebar->sbSpeed->setValue(100);
            m_d->titlebar->frameCounter->setValue(animinterface->currentTime());
        }


        m_d->framesModel->setAnimationPlayer(m_d->canvas->animationPlayer());

        m_d->framesModel->setNodeManipulationInterface(
            new NodeManagerInterface(m_d->canvas->viewManager()->nodeManager()));

        m_d->canvasConnections.addConnection(
            m_d->canvas->viewManager()->nodeManager(), SIGNAL(sigNodeActivated(KisNodeSP)),
            m_d->framesModel, SLOT(slotCurrentNodeChanged(KisNodeSP)));

        m_d->canvasConnections.addConnection(
            m_d->framesModel, SIGNAL(requestCurrentNodeChanged(KisNodeSP)),
            m_d->canvas->viewManager()->nodeManager(), SLOT(slotNonUiActivatedNode(KisNodeSP)));

        m_d->framesModel->slotCurrentNodeChanged(m_d->canvas->viewManager()->activeNode());

        m_d->canvasConnections.addConnection(
                    m_d->canvas->viewManager()->mainWindow(), SIGNAL(themeChanged()),
                    this, SLOT(handleThemeChange()));

        m_d->canvasConnections.addConnection(
                    m_d->canvas, SIGNAL(sigCanvasEngineChanged()),
                    this, SLOT(updateFrameCache()));


        connect(m_d->canvas->image()->animationInterface(), SIGNAL(sigUiTimeChanged(int)), this, SLOT(updateFrameCounter()));
        connect(m_d->canvas->animationPlayer(), SIGNAL(sigFrameChanged()), this, SLOT(updateFrameCounter()));
        connect(m_d->canvas->animationPlayer(), SIGNAL(sigPlaybackStopped()), this, SLOT(updateFrameCounter()));

        connect(m_d->canvas->animationPlayer(), SIGNAL(sigPlaybackStateChanged(bool)), m_d->titlebar->frameCounter, SLOT(setDisabled(bool)));
        connect(m_d->canvas->animationPlayer(), SIGNAL(sigPlaybackStateChanged(bool)), m_d->titlebar->transport, SLOT(setPlaying(bool)));

        connect(m_d->canvas->image()->animationInterface(), SIGNAL(sigFullClipRangeChanged()), SLOT(handleClipRangeChange())); // TODO animationplayer seems to be missing emit for ClipRangeChanged
        connect(m_d->canvas->image()->animationInterface(), SIGNAL(sigFramerateChanged()), SLOT(handleFrameRateChange()));
        //connect(m_d->canvas->animationPlayer(), SIGNAL(sigPlaybackStatisticsUpdated()), this, SLOT(updateDropFramesIcon()));
    }
}

void TimelineDocker::handleThemeChange()
{
    if (m_d->framesView) {
        m_d->framesView->slotUpdateIcons();
    }
}

void TimelineDocker::updateFrameCache()
{
    m_d->framesModel->setFrameCache(m_d->canvas->frameCache());
}

void TimelineDocker::updateFrameCounter()
{
    if (!m_d->canvas && !m_d->canvas->image()) {
        return;
    }

    const int frame = m_d->canvas->animationPlayer()->isPlaying() ?
                      m_d->canvas->animationPlayer()->visibleFrame() :
                      m_d->canvas->image()->animationInterface()->currentUITime();

    m_d->titlebar->frameCounter->setValue(frame);
}

void TimelineDocker::unsetCanvas()
{
    setCanvas(0);
}

void TimelineDocker::setViewManager(KisViewManager *view)
{
    m_d->mainWindow = view->mainWindow();

    KisActionManager *actionManager = view->actionManager();
    m_d->framesView->setActionManager(actionManager);

    KisAction *action = 0;

    TimelineDockerTitleBar* titleBar = static_cast<TimelineDockerTitleBar*>(titleBarWidget());

    action = actionManager->actionByName("add_blank_frame");
    titleBar->btnAddKeyframe->setDefaultAction(action);

    action = actionManager->actionByName("add_duplicate_frame");
    titleBar->btnDuplicateKeyframe->setDefaultAction(action);

    action = actionManager->actionByName("remove_frames");
    titleBar->btnRemoveKeyframe->setDefaultAction(action);

    action = actionManager->createAction("toggle_playback");
    action->setActivationFlags(KisAction::ACTIVE_IMAGE);
    connect(action, SIGNAL(triggered(bool)), SLOT(playPause()));

    action = actionManager->createAction("stop_playback");
    action->setActivationFlags(KisAction::ACTIVE_IMAGE);
    connect(action, SIGNAL(triggered(bool)), SLOT(stop()));

    action = actionManager->createAction("previous_frame");
    action->setActivationFlags(KisAction::ACTIVE_IMAGE);
    connect(action, SIGNAL(triggered(bool)), SLOT(previousFrame()));

    action = actionManager->createAction("next_frame");
    action->setActivationFlags(KisAction::ACTIVE_IMAGE);
    connect(action, SIGNAL(triggered(bool)), SLOT(nextFrame()));

    action = actionManager->createAction("lazy_frame");
    m_d->titlebar->btnAutoFrame->setDefaultAction(action);
}

void TimelineDocker::playPause()
{
    if (!m_d->canvas) return;

    if (m_d->canvas->animationPlayer()->isPlaying()) {
        m_d->canvas->animationPlayer()->pause();
    } else {
        m_d->canvas->animationPlayer()->play();
    }
}

void TimelineDocker::stop()
{
    if (!m_d->canvas) return;

    if( m_d->canvas->animationPlayer()->isStopped()) {
        m_d->canvas->animationPlayer()->goToStartFrame();
    } else {
        m_d->canvas->animationPlayer()->stop();
        m_d->canvas->animationPlayer()->goToPlaybackOrigin();
    }
}

void TimelineDocker::previousFrame()
{
    if (!m_d->canvas) return;
    KisImageAnimationInterface *animInterface = m_d->canvas->image()->animationInterface();

    int time = animInterface->currentUITime() - 1;
    if (time >= 0) {
        animInterface->requestTimeSwitchWithUndo(time);
    }
}

void TimelineDocker::nextFrame()
{
    if (!m_d->canvas) return;
    KisImageAnimationInterface *animInterface = m_d->canvas->image()->animationInterface();

    int time = animInterface->currentUITime() + 1;
    animInterface->requestTimeSwitchWithUndo(time);
}

void TimelineDocker::goToFrame(int frameIndex)
{
    if (!m_d->canvas || !m_d->canvas->image()) return;

    KisImageAnimationInterface *animInterface = m_d->canvas->image()->animationInterface();

    if (m_d->canvas->animationPlayer()->isPlaying() ||
        frameIndex == animInterface->currentUITime()) {
        return;
    }

    animInterface->requestTimeSwitchWithUndo(frameIndex);
}

void TimelineDocker::setStartFrame(int frame)
{
    if (!m_d->canvas || !m_d->canvas->image()) return;

    KisImageAnimationInterface *animInterface = m_d->canvas->image()->animationInterface();

    animInterface->setFullClipRangeStartTime(frame);
}

void TimelineDocker::setEndFrame(int frame)
{
    if (!m_d->canvas || !m_d->canvas->image()) return;

    KisImageAnimationInterface *animInterface = m_d->canvas->image()->animationInterface();

    animInterface->setFullClipRangeEndTime(frame);
}

void TimelineDocker::setFrameRate(int framerate)
{
    if (!m_d->canvas || !m_d->canvas->image()) return;

    KisImageAnimationInterface *animInterface = m_d->canvas->image()->animationInterface();

    animInterface->setFramerate(framerate);
}

void TimelineDocker::setPlaybackSpeed(int playbackSpeed)
{
    if (!m_d->canvas || !m_d->canvas->image()) return;

    const float normalizedSpeed = playbackSpeed / 100.0;
    m_d->canvas->animationPlayer()->slotUpdatePlaybackSpeed(normalizedSpeed);
}

void TimelineDocker::handleClipRangeChange()
{
    ENTER_FUNCTION() << "FULL CLIP CHANGE";
    if (!m_d->canvas || !m_d->canvas->image()) return;

    KisImageAnimationInterface *animInterface = m_d->canvas->image()->animationInterface();

    m_d->titlebar->sbStartFrame->setValue(animInterface->fullClipRange().start());
    m_d->titlebar->sbEndFrame->setValue(animInterface->fullClipRange().end());
}

void TimelineDocker::handleFrameRateChange()
{
    if (!m_d->canvas || !m_d->canvas->image()) return;

    KisImageAnimationInterface *animInterface = m_d->canvas->image()->animationInterface();

    m_d->titlebar->sbFrameRate->setValue( m_d->canvas->image()->animationInterface()->framerate() );
}


