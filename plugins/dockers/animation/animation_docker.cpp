/*
 *  Copyright (c) 2015 Jouni Pentikäinen <joupent@gmail.com>
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

#include "animation_docker.h"

#include "kis_global.h"

#include "kis_canvas2.h"
#include "kis_image.h"
#include <QMenu>
#include <kis_icon.h>
#include <kis_transform_mask.h>
#include "KisViewManager.h"
#include "kis_action_manager.h"
#include "kis_image_animation_interface.h"
#include "kis_animation_player.h"
#include "kis_time_range.h"
#include "kundo2command.h"
#include "kis_post_execution_undo_adapter.h"
#include "kis_keyframe_channel.h"
#include "kis_animation_utils.h"
#include "krita_utils.h"
#include "kis_image_config.h"
#include "kis_config.h"
#include "kis_signals_blocker.h"
#include "kis_node_manager.h"
#include "kis_transform_mask_params_factory_registry.h"


#include "ui_wdg_animation.h"

void setupActionButton(const QString &text,
                       KisAction::ActivationFlags flags,
                       bool defaultValue,
                       QToolButton *button,
                       KisAction **action)
{
    *action = new KisAction(text, button);
    (*action)->setActivationFlags(flags);
    (*action)->setCheckable(true);
    (*action)->setChecked(defaultValue);
    button->setDefaultAction(*action);
}

AnimationDocker::AnimationDocker()
    : QDockWidget(i18n("Animation"))
    , m_canvas(0)
    , m_animationWidget(new Ui_WdgAnimation)
    , m_mainWindow(0)
{
    QWidget* mainWidget = new QWidget(this);
    setWidget(mainWidget);

    m_animationWidget->setupUi(mainWidget);
}

AnimationDocker::~AnimationDocker()
{
    delete m_animationWidget;
}

void AnimationDocker::setCanvas(KoCanvasBase * canvas)
{
    if(m_canvas == canvas)
        return;

    setEnabled(canvas != 0);

    if (m_canvas) {
        m_canvas->disconnectCanvasObserver(this);
        m_canvas->image()->disconnect(this);
        m_canvas->image()->animationInterface()->disconnect(this);
        m_canvas->animationPlayer()->disconnect(this);
        m_canvas->viewManager()->nodeManager()->disconnect(this);
    }

    m_canvas = dynamic_cast<KisCanvas2*>(canvas);

    if (m_canvas && m_canvas->image()) {

        KisImageAnimationInterface *animation = m_canvas->image()->animationInterface();
        {
            KisSignalsBlocker bloker(m_animationWidget->spinFromFrame,
                                     m_animationWidget->spinToFrame,
                                     m_animationWidget->intFramerate);

            m_animationWidget->spinFromFrame->setValue(animation->fullClipRange().start());
            m_animationWidget->spinToFrame->setValue(animation->fullClipRange().end());
            m_animationWidget->intFramerate->setValue(animation->framerate());
        }


        connect(animation, SIGNAL(sigUiTimeChanged(int)), this, SLOT(slotGlobalTimeChanged()));
        connect(m_canvas->animationPlayer(), SIGNAL(sigFrameChanged()), this, SLOT(slotGlobalTimeChanged()));
        connect(m_canvas->animationPlayer(), SIGNAL(sigPlaybackStopped()), this, SLOT(slotGlobalTimeChanged()));
        connect(m_canvas->animationPlayer(), SIGNAL(sigPlaybackStopped()), this, SLOT(updatePlayPauseIcon()));
        connect(m_canvas->animationPlayer(), SIGNAL(sigPlaybackStarted()), this, SLOT(updatePlayPauseIcon()));
        connect(m_canvas->animationPlayer(), SIGNAL(sigPlaybackStatisticsUpdated()), this, SLOT(updateDropFramesIcon()));
        connect(m_animationWidget->doublePlaySpeed,
                SIGNAL(valueChanged(double)),
                m_canvas->animationPlayer(),
                SLOT(slotUpdatePlaybackSpeed(double)));

        connect(m_canvas->viewManager()->nodeManager(), SIGNAL(sigNodeActivated(KisNodeSP)),
                this, SLOT(slotCurrentNodeChanged(KisNodeSP)));

        connect (animation, SIGNAL(sigFullClipRangeChanged()), this, SLOT(updateClipRange()));



        slotGlobalTimeChanged();
        slotCurrentNodeChanged(m_canvas->viewManager()->nodeManager()->activeNode());
    }

    slotUpdateIcons();
}

void AnimationDocker::unsetCanvas()
{
    setCanvas(0);
}

void AnimationDocker::setViewManager(KisViewManager *view)
{
    setActions(view->actionManager());

    slotUpdateIcons();
    connect(view->mainWindow(), SIGNAL(themeChanged()), this, SLOT(slotUpdateIcons()));
    m_mainWindow = view->mainWindow();
}

void AnimationDocker::slotAddOpacityKeyframe()
{
    addKeyframe(KisKeyframeChannel::Opacity.id(), false);
}

void AnimationDocker::slotDeleteOpacityKeyframe()
{
    deleteKeyframe(KisKeyframeChannel::Opacity.id());
}

void AnimationDocker::slotAddTransformKeyframe()
{
    if (!m_canvas) return;

    KisTransformMask *mask = dynamic_cast<KisTransformMask*>(m_canvas->viewManager()->activeNode().data());
    if (!mask) return;

    const int time = m_canvas->image()->animationInterface()->currentTime();

    KUndo2Command *command = new KUndo2Command(kundo2_i18n("Add transform keyframe"));
    KisTransformMaskParamsFactoryRegistry::instance()->autoAddKeyframe(mask, time, KisTransformMaskParamsInterfaceSP(), command);

    command->redo();
    m_canvas->currentImage()->postExecutionUndoAdapter()->addCommand(toQShared(command));
}

void AnimationDocker::slotDeleteTransformKeyframe()
{
    deleteKeyframe(KisKeyframeChannel::TransformArguments.id());
}

void AnimationDocker::slotUIRangeChanged()
{
    if (!m_canvas || !m_canvas->image()) return;

    int fromTime = m_animationWidget->spinFromFrame->value();
    int toTime = m_animationWidget->spinToFrame->value();

    m_canvas->image()->animationInterface()->setFullClipRange(KisTimeRange::fromTime(fromTime, toTime));
}

void AnimationDocker::slotUIFramerateChanged()
{
    if (!m_canvas || !m_canvas->image()) return;

    m_canvas->image()->animationInterface()->setFramerate(m_animationWidget->intFramerate->value());
}

void AnimationDocker::slotOnionSkinOptions()
{
    if (m_mainWindow) {
        QDockWidget *docker = m_mainWindow->dockWidget("OnionSkinsDocker");
        if (docker) {
            docker->setVisible(!docker->isVisible());
        }
    }
}

void AnimationDocker::slotGlobalTimeChanged()
{
    int time = m_canvas->animationPlayer()->isPlaying() ?
                m_canvas->animationPlayer()->currentTime() :
                m_canvas->image()->animationInterface()->currentUITime();

    m_animationWidget->intCurrentTime->setValue(time);

    const int frameRate = m_canvas->image()->animationInterface()->framerate();
    const int msec = 1000 * time / frameRate;

    QTime realTime;
    realTime = realTime.addMSecs(msec);

    QString realTimeString = realTime.toString("hh:mm:ss.zzz");
    m_animationWidget->intCurrentTime->setToolTip(realTimeString);
}

void AnimationDocker::slotTimeSpinBoxChanged()
{
    if (!m_canvas || !m_canvas->image()) return;

    int newTime = m_animationWidget->intCurrentTime->value();
    KisImageAnimationInterface *animation = m_canvas->image()->animationInterface();

    if (m_canvas->animationPlayer()->isPlaying() ||
        newTime == animation->currentUITime()) {

        return;
    }

    animation->requestTimeSwitchWithUndo(newTime);
}

void AnimationDocker::slotPreviousFrame()
{
    if (!m_canvas) return;
    KisImageAnimationInterface *animation = m_canvas->image()->animationInterface();

    int time = animation->currentUITime() - 1;
    if (time >= 0) {
        animation->requestTimeSwitchWithUndo(time);
    }
}

void AnimationDocker::slotNextFrame()
{
    if (!m_canvas) return;
    KisImageAnimationInterface *animation = m_canvas->image()->animationInterface();

    int time = animation->currentUITime() + 1;
    animation->requestTimeSwitchWithUndo(time);
}

void AnimationDocker::slotPreviousKeyFrame()
{
    if (!m_canvas) return;

    KisNodeSP node = m_canvas->viewManager()->activeNode();
    if (!node) return;

    KisImageAnimationInterface *animation = m_canvas->image()->animationInterface();
    int time = animation->currentUITime();

    KisKeyframeChannel *content =
        node->getKeyframeChannel(KisKeyframeChannel::Content.id());

    if (!content) return;

    KisKeyframeSP dstKeyframe;
    KisKeyframeSP keyframe = content->keyframeAt(time);

    if (!keyframe) {
        dstKeyframe = content->activeKeyframeAt(time);
    } else {
        dstKeyframe = content->previousKeyframe(keyframe);
    }

    if (dstKeyframe) {
        animation->requestTimeSwitchWithUndo(dstKeyframe->time());
    }
}

void AnimationDocker::slotNextKeyFrame()
{
    if (!m_canvas) return;

    KisNodeSP node = m_canvas->viewManager()->activeNode();
    if (!node) return;

    KisImageAnimationInterface *animation = m_canvas->image()->animationInterface();
    int time = animation->currentUITime();

    KisKeyframeChannel *content =
        node->getKeyframeChannel(KisKeyframeChannel::Content.id());

    if (!content) return;

    KisKeyframeSP dstKeyframe;
    KisKeyframeSP keyframe = content->activeKeyframeAt(time);

    if (keyframe) {
        dstKeyframe = content->nextKeyframe(keyframe);
    }

    if (dstKeyframe) {
        animation->requestTimeSwitchWithUndo(dstKeyframe->time());
    }
}


void AnimationDocker::slotFirstFrame()
{
    if (!m_canvas) return;

    KisImageAnimationInterface *animation = m_canvas->image()->animationInterface();
    animation->requestTimeSwitchWithUndo(0);
}

void AnimationDocker::slotLastFrame()
{
    if (!m_canvas) return;

    KisImageAnimationInterface *animation = m_canvas->image()->animationInterface();
    animation->requestTimeSwitchWithUndo(animation->totalLength() - 1);
}


void AnimationDocker::slotPlayPause()
{
    if (!m_canvas) return;

    if (m_canvas->animationPlayer()->isPlaying()) {
        m_canvas->animationPlayer()->stop();
    } else {
        m_canvas->animationPlayer()->play();
    }

    updatePlayPauseIcon();
}

void AnimationDocker::updatePlayPauseIcon()
{
    bool isPlaying = m_canvas && m_canvas->animationPlayer() && m_canvas->animationPlayer()->isPlaying();

    m_playPauseAction->setIcon(isPlaying ?
                               KisIconUtils::loadIcon("animation_stop") :
                               KisIconUtils::loadIcon("animation_play"));
}

void AnimationDocker::updateLazyFrameIcon()
{
    KisImageConfig cfg(true);

    const bool value = cfg.lazyFrameCreationEnabled();

    m_lazyFrameAction->setIcon(value ?
                               KisIconUtils::loadIcon("lazyframeOn") :
                               KisIconUtils::loadIcon("lazyframeOff"));

    m_lazyFrameAction->setText(QString("%1 (%2)")
                               .arg(KisAnimationUtils::lazyFrameCreationActionName)
                               .arg(KritaUtils::toLocalizedOnOff(value)));
}

void AnimationDocker::updateDropFramesIcon()
{
    qreal effectiveFps = 0.0;
    qreal realFps = 0.0;
    qreal framesDropped = 0.0;
    bool isPlaying = false;

    KisAnimationPlayer *player =
        m_canvas && m_canvas->animationPlayer() ?
        m_canvas->animationPlayer() : 0;

    if (player) {
        effectiveFps = player->effectiveFps();
        realFps = player->realFps();
        framesDropped = player->framesDroppedPortion();
        isPlaying = player->isPlaying();
    }

    KisConfig cfg(true);
    const bool value = cfg.animationDropFrames();

    m_dropFramesAction->setIcon(value ?
                                KisIconUtils::loadIcon(framesDropped > 0.05 ? "droppedframes" : "dropframe") :
                                KisIconUtils::loadIcon("dropframe"));


    QString text;

    if (!isPlaying) {
        text = QString("%1 (%2)")
            .arg(KisAnimationUtils::dropFramesActionName)
            .arg(KritaUtils::toLocalizedOnOff(value));
    } else {
        text = QString("%1 (%2)\n"
                       "%3\n"
                       "%4\n"
                       "%5")
            .arg(KisAnimationUtils::dropFramesActionName)
            .arg(KritaUtils::toLocalizedOnOff(value))
            .arg(i18n("Effective FPS:\t%1", effectiveFps))
            .arg(i18n("Real FPS:\t%1", realFps))
            .arg(i18n("Frames dropped:\t%1\%", framesDropped * 100));
    }

    m_dropFramesAction->setText(text);
}

void AnimationDocker::slotUpdateIcons()
{
    m_previousFrameAction->setIcon(KisIconUtils::loadIcon("prevframe"));
    m_nextFrameAction->setIcon(KisIconUtils::loadIcon("nextframe"));

    m_previousKeyFrameAction->setIcon(KisIconUtils::loadIcon("prevkeyframe"));
    m_nextKeyFrameAction->setIcon(KisIconUtils::loadIcon("nextkeyframe"));

    m_firstFrameAction->setIcon(KisIconUtils::loadIcon("firstframe"));
    m_lastFrameAction->setIcon(KisIconUtils::loadIcon("lastframe"));

    updatePlayPauseIcon();
    updateLazyFrameIcon();
    updateDropFramesIcon();

    m_animationWidget->btnOnionSkinOptions->setIcon(KisIconUtils::loadIcon("onion_skin_options"));
    m_animationWidget->btnOnionSkinOptions->setIconSize(QSize(22, 22));


    m_animationWidget->btnNextKeyFrame->setIconSize(QSize(22, 22));
    m_animationWidget->btnPreviousKeyFrame->setIconSize(QSize(22, 22));
    m_animationWidget->btnFirstFrame->setIconSize(QSize(22, 22));
    m_animationWidget->btnLastFrame->setIconSize(QSize(22, 22));

    m_animationWidget->btnPreviousFrame->setIconSize(QSize(22, 22));
    m_animationWidget->btnPlay->setIconSize(QSize(22, 22));
    m_animationWidget->btnNextFrame->setIconSize(QSize(22, 22));
    m_animationWidget->btnAddKeyframe->setIconSize(QSize(22, 22));
    m_animationWidget->btnAddDuplicateFrame->setIconSize(QSize(22, 22));
    m_animationWidget->btnDeleteKeyframe->setIconSize(QSize(22, 22));
    m_animationWidget->btnLazyFrame->setIconSize(QSize(22, 22));
    m_animationWidget->btnDropFrames->setIconSize(QSize(22, 22));
}

void AnimationDocker::slotLazyFrameChanged(bool value)
{
    KisImageConfig cfg(false);

    if (value != cfg.lazyFrameCreationEnabled()) {
        cfg.setLazyFrameCreationEnabled(value);
        updateLazyFrameIcon();
    }
}

void AnimationDocker::slotDropFramesChanged(bool value)
{
    KisConfig cfg(false);

    if (value != cfg.animationDropFrames()) {
        cfg.setAnimationDropFrames(value);
        updateDropFramesIcon();
    }
}

void AnimationDocker::slotCurrentNodeChanged(KisNodeSP node)
{
    bool isNodeAnimatable = false;

    m_newKeyframeMenu->clear();
    m_deleteKeyframeMenu->clear();

    if (!node.isNull()) {
        if (KisAnimationUtils::supportsContentFrames(node)) {
            isNodeAnimatable = true;
            KisActionManager::safePopulateMenu(m_newKeyframeMenu, "add_blank_frame", m_actionManager);
            KisActionManager::safePopulateMenu(m_deleteKeyframeMenu, "remove_frames", m_actionManager);
        }

        if (node->inherits("KisLayer")) {
            isNodeAnimatable = true;
            m_newKeyframeMenu->addAction(m_addOpacityKeyframeAction);
            m_deleteKeyframeMenu->addAction(m_deleteOpacityKeyframeAction);
        }

        /*
        if (node->inherits("KisTransformMask")) {
            isNodeAnimatable = true;
            m_newKeyframeMenu->addAction(m_addTransformKeyframeAction);
            m_deleteKeyframeMenu->addAction(m_deleteTransformKeyframeAction);
        }
        */
    }

    m_animationWidget->btnAddKeyframe->setEnabled(isNodeAnimatable);
    m_animationWidget->btnAddDuplicateFrame->setEnabled(isNodeAnimatable);
    m_animationWidget->btnDeleteKeyframe->setEnabled(isNodeAnimatable);
}

void AnimationDocker::updateClipRange()
{
    m_animationWidget->spinFromFrame->setValue(m_canvas->image()->animationInterface()->fullClipRange().start());
    m_animationWidget->spinToFrame->setValue(m_canvas->image()->animationInterface()->fullClipRange().end());
}

void AnimationDocker::addKeyframe(const QString &channel, bool copy)
{
    if (!m_canvas) return;

    KisNodeSP node = m_canvas->viewManager()->activeNode();
    if (!node) return;

    const int time = m_canvas->image()->animationInterface()->currentTime();
    KisAnimationUtils::createKeyframeLazy(m_canvas->image(), node, channel, time, copy);
}

void AnimationDocker::deleteKeyframe(const QString &channel)
{
    if (!m_canvas) return;

    KisNodeSP node = m_canvas->viewManager()->activeNode();
    if (!node) return;

    const int time = m_canvas->image()->animationInterface()->currentTime();
    KisAnimationUtils::removeKeyframe(m_canvas->image(), node, channel, time);
}

void AnimationDocker::setActions(KisActionManager *actionMan)
{
    m_actionManager = actionMan;
    if (!m_actionManager) return;

    m_previousFrameAction = new KisAction(i18n("Previous Frame"), m_animationWidget->btnPreviousFrame);
    m_previousFrameAction->setActivationFlags(KisAction::ACTIVE_IMAGE);
    m_animationWidget->btnPreviousFrame->setDefaultAction(m_previousFrameAction);

    m_nextFrameAction = new KisAction(i18n("Next Frame"), m_animationWidget->btnNextFrame);
    m_nextFrameAction->setActivationFlags(KisAction::ACTIVE_IMAGE);
    m_animationWidget->btnNextFrame->setDefaultAction(m_nextFrameAction);

    m_previousKeyFrameAction = new KisAction(i18n("Previous Key Frame"), m_animationWidget->btnPreviousKeyFrame);
    m_previousKeyFrameAction->setActivationFlags(KisAction::ACTIVE_IMAGE);
    m_animationWidget->btnPreviousKeyFrame->setDefaultAction(m_previousKeyFrameAction);

    m_nextKeyFrameAction = new KisAction(i18n("Next Key Frame"), m_animationWidget->btnNextKeyFrame);
    m_nextKeyFrameAction->setActivationFlags(KisAction::ACTIVE_IMAGE);
    m_animationWidget->btnNextKeyFrame->setDefaultAction(m_nextKeyFrameAction);

    m_firstFrameAction = new KisAction(i18n("First Frame"), m_animationWidget->btnFirstFrame);
    m_firstFrameAction->setActivationFlags(KisAction::ACTIVE_IMAGE);
    m_animationWidget->btnFirstFrame->setDefaultAction(m_firstFrameAction);

    m_lastFrameAction = new KisAction(i18n("Last Frame"), m_animationWidget->btnLastFrame);
    m_lastFrameAction->setActivationFlags(KisAction::ACTIVE_IMAGE);
    m_animationWidget->btnLastFrame->setDefaultAction(m_lastFrameAction);


    m_playPauseAction = new KisAction(i18n("Play / Stop"), m_animationWidget->btnPlay);
    m_playPauseAction->setActivationFlags(KisAction::ACTIVE_IMAGE);
    m_animationWidget->btnPlay->setDefaultAction(m_playPauseAction);

    KisAction *action = 0;

    action = m_actionManager->createAction("add_blank_frame");
    m_animationWidget->btnAddKeyframe->setDefaultAction(action);

    action = m_actionManager->createAction("add_duplicate_frame");
    m_animationWidget->btnAddDuplicateFrame->setDefaultAction(action);

    action = m_actionManager->createAction("remove_frames");
    m_animationWidget->btnDeleteKeyframe->setDefaultAction(action);

    m_newKeyframeMenu = new QMenu(this);
    m_animationWidget->btnAddKeyframe->setMenu(m_newKeyframeMenu);
    m_animationWidget->btnAddKeyframe->setPopupMode(QToolButton::MenuButtonPopup);

    m_deleteKeyframeMenu = new QMenu(this);
    m_animationWidget->btnDeleteKeyframe->setMenu(m_deleteKeyframeMenu);
    m_animationWidget->btnDeleteKeyframe->setPopupMode(QToolButton::MenuButtonPopup);

    m_addOpacityKeyframeAction = new KisAction(KisAnimationUtils::addOpacityKeyframeActionName);
    m_deleteOpacityKeyframeAction = new KisAction(KisAnimationUtils::removeOpacityKeyframeActionName);

    m_addTransformKeyframeAction = new KisAction(KisAnimationUtils::addTransformKeyframeActionName);
    m_deleteTransformKeyframeAction = new KisAction(KisAnimationUtils::removeTransformKeyframeActionName);


    // other new stuff
    m_actionManager->addAction("previous_frame", m_previousFrameAction);
    m_actionManager->addAction("next_frame", m_nextFrameAction);

    m_actionManager->addAction("previous_keyframe", m_previousKeyFrameAction);
    m_actionManager->addAction("next_keyframe", m_nextKeyFrameAction);

    m_actionManager->addAction("first_frame", m_firstFrameAction);
    m_actionManager->addAction("last_frame", m_lastFrameAction);

    {
        KisImageConfig cfg(true);
        setupActionButton(KisAnimationUtils::lazyFrameCreationActionName,
                          KisAction::ACTIVE_IMAGE,
                          cfg.lazyFrameCreationEnabled(),
                          m_animationWidget->btnLazyFrame,
                          &m_lazyFrameAction);
    }

    {
        KisConfig cfg(true);
        setupActionButton(KisAnimationUtils::dropFramesActionName,
                          KisAction::ACTIVE_IMAGE,
                          cfg.animationDropFrames(),
                          m_animationWidget->btnDropFrames,
                          &m_dropFramesAction);
    }


     // these actions are created in the setupActionButton() above, so we need to add actions after that
     m_actionManager->addAction("lazy_frame", m_lazyFrameAction);
     m_actionManager->addAction("drop_frames", m_dropFramesAction);

     m_actionManager->addAction("toggle_playback", m_playPauseAction);

    QFont font;
    font.setPointSize(1.7 * font.pointSize());
    font.setBold(true);
    m_animationWidget->intCurrentTime->setFont(font);

    connect(m_previousFrameAction, SIGNAL(triggered()), this, SLOT(slotPreviousFrame()));
    connect(m_nextFrameAction, SIGNAL(triggered()), this, SLOT(slotNextFrame()));

    connect(m_previousKeyFrameAction, SIGNAL(triggered()), this, SLOT(slotPreviousKeyFrame()));
    connect(m_nextKeyFrameAction, SIGNAL(triggered()), this, SLOT(slotNextKeyFrame()));

    connect(m_firstFrameAction, SIGNAL(triggered()), this, SLOT(slotFirstFrame()));
    connect(m_lastFrameAction, SIGNAL(triggered()), this, SLOT(slotLastFrame()));

    connect(m_playPauseAction, SIGNAL(triggered()), this, SLOT(slotPlayPause()));

    connect(m_lazyFrameAction, SIGNAL(toggled(bool)), this, SLOT(slotLazyFrameChanged(bool)));
    connect(m_dropFramesAction, SIGNAL(toggled(bool)), this, SLOT(slotDropFramesChanged(bool)));

    connect(m_addOpacityKeyframeAction, SIGNAL(triggered(bool)), this, SLOT(slotAddOpacityKeyframe()));
    connect(m_deleteOpacityKeyframeAction, SIGNAL(triggered(bool)), this, SLOT(slotDeleteOpacityKeyframe()));

    connect(m_addTransformKeyframeAction, SIGNAL(triggered(bool)), this, SLOT(slotAddTransformKeyframe()));
    connect(m_deleteTransformKeyframeAction, SIGNAL(triggered(bool)), this, SLOT(slotDeleteTransformKeyframe()));

    m_animationWidget->btnOnionSkinOptions->setToolTip(i18n("Onion Skins"));
    connect(m_animationWidget->btnOnionSkinOptions, SIGNAL(clicked()), this, SLOT(slotOnionSkinOptions()));

    connect(m_animationWidget->spinFromFrame, SIGNAL(valueChanged(int)), this, SLOT(slotUIRangeChanged()));
    connect(m_animationWidget->spinToFrame, SIGNAL(valueChanged(int)), this, SLOT(slotUIRangeChanged()));
    connect(m_animationWidget->intFramerate, SIGNAL(valueChanged(int)), this, SLOT(slotUIFramerateChanged()));

    connect(m_animationWidget->intCurrentTime, SIGNAL(valueChanged(int)), SLOT(slotTimeSpinBoxChanged()));
}
