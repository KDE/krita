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
#include <kis_icon.h>
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


#include "ui_wdg_animation.h"

AnimationDocker::AnimationDocker()
    : QDockWidget(i18n("Animation"))
    , m_canvas(0)
    , m_animationWidget(new Ui_WdgAnimation)
    , m_mainWindow(0)
{
    QWidget* mainWidget = new QWidget(this);
    setWidget(mainWidget);

    m_animationWidget->setupUi(mainWidget);


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

    m_playPauseAction = new KisAction(i18n("Play / Pause"), m_animationWidget->btnPlay);
    m_playPauseAction->setActivationFlags(KisAction::ACTIVE_IMAGE);
    m_animationWidget->btnPlay->setDefaultAction(m_playPauseAction);

    m_addBlankFrameAction = new KisAction(KisAnimationUtils::addFrameActionName, m_animationWidget->btnAddKeyframe);
    m_addBlankFrameAction->setActivationFlags(KisAction::ACTIVE_LAYER);
    m_animationWidget->btnAddKeyframe->setDefaultAction(m_addBlankFrameAction);

    m_addDuplicateFrameAction = new KisAction(KisAnimationUtils::duplicateFrameActionName, m_animationWidget->btnAddDuplicateFrame);
    m_addDuplicateFrameAction->setActivationFlags(KisAction::ACTIVE_LAYER);
    m_animationWidget->btnAddDuplicateFrame->setDefaultAction(m_addDuplicateFrameAction);

    m_deleteKeyframeAction = new KisAction(KisAnimationUtils::removeFrameActionName, m_animationWidget->btnDeleteKeyframe);
    m_deleteKeyframeAction->setActivationFlags(KisAction::ACTIVE_LAYER);
    m_animationWidget->btnDeleteKeyframe->setDefaultAction(m_deleteKeyframeAction);

    m_lazyFrameAction = new KisAction(KisAnimationUtils::lazyFrameCreationActionName, m_animationWidget->btnLazyFrame);
    m_lazyFrameAction->setActivationFlags(KisAction::ACTIVE_IMAGE);
    m_lazyFrameAction->setCheckable(true);

    {
        KisImageConfig cfg;
        m_lazyFrameAction->setChecked(cfg.lazyFrameCreationEnabled());
    }

    m_animationWidget->btnLazyFrame->setDefaultAction(m_lazyFrameAction);

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

    connect(m_addBlankFrameAction, SIGNAL(triggered()), this, SLOT(slotAddBlankFrame()));
    connect(m_addDuplicateFrameAction, SIGNAL(triggered()), this, SLOT(slotAddDuplicateFrame()));
    connect(m_deleteKeyframeAction, SIGNAL(triggered()), this, SLOT(slotDeleteKeyframe()));
    connect(m_lazyFrameAction, SIGNAL(toggled(bool)), this, SLOT(slotLazyFrameChanged(bool)));

    m_animationWidget->btnOnionSkinOptions->setToolTip(i18n("Onion Skins"));
    connect(m_animationWidget->btnOnionSkinOptions, SIGNAL(clicked()), this, SLOT(slotOnionSkinOptions()));

    connect(m_animationWidget->spinFromFrame, SIGNAL(valueChanged(int)), this, SLOT(slotUIRangeChanged()));
    connect(m_animationWidget->spinToFrame, SIGNAL(valueChanged(int)), this, SLOT(slotUIRangeChanged()));
    connect(m_animationWidget->intFramerate, SIGNAL(valueChanged(int)), this, SLOT(slotUIFramerateChanged()));

    connect(m_animationWidget->intCurrentTime, SIGNAL(valueChanged(int)), SLOT(slotTimeSpinBoxChanged()));
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
    }

    m_canvas = dynamic_cast<KisCanvas2*>(canvas);

    if (m_canvas && m_canvas->image()) {
        KisImageAnimationInterface *animation = m_canvas->image()->animationInterface();
        m_animationWidget->spinFromFrame->setValue(animation->currentRange().start());
        m_animationWidget->spinToFrame->setValue(animation->currentRange().end());
        m_animationWidget->intFramerate->setValue(animation->framerate());

        connect(animation, SIGNAL(sigTimeChanged(int)), this, SLOT(slotGlobalTimeChanged()));
        connect(m_canvas->animationPlayer(), SIGNAL(sigFrameChanged()), this, SLOT(slotGlobalTimeChanged()));
        connect(m_canvas->animationPlayer(), SIGNAL(sigPlaybackStopped()), this, SLOT(slotGlobalTimeChanged()));
        connect(m_canvas->animationPlayer(), SIGNAL(sigPlaybackStopped()), this, SLOT(updatePlayPauseIcon()));
        connect(m_animationWidget->doublePlaySpeed,
                SIGNAL(valueChanged(double)),
                m_canvas->animationPlayer(),
                SLOT(slotUpdatePlaybackSpeed(double)));

        slotGlobalTimeChanged();
    }
}

void AnimationDocker::unsetCanvas()
{
    setCanvas(0);
}

void AnimationDocker::setMainWindow(KisViewManager *view)
{
    KisActionManager *actionManager = view->actionManager();

    actionManager->addAction("previous_frame", m_previousFrameAction);
    actionManager->addAction("next_frame", m_nextFrameAction);

    actionManager->addAction("previous_keyframe", m_previousKeyFrameAction);
    actionManager->addAction("next_keyframe", m_nextKeyFrameAction);

    actionManager->addAction("first_frame", m_firstFrameAction);
    actionManager->addAction("last_frame", m_lastFrameAction);

    actionManager->addAction("lazy_frame", m_lazyFrameAction);

    actionManager->addAction("toggle_playback", m_playPauseAction);
    actionManager->addAction("add_blank_frame", m_addBlankFrameAction);
    actionManager->addAction("add_duplicate_frame", m_addDuplicateFrameAction);
    actionManager->addAction("delete_keyframe", m_deleteKeyframeAction);

    connect(view->mainWindow(), SIGNAL(themeChanged()), this, SLOT(slotUpdateIcons()));

    m_mainWindow = view->mainWindow();
}

void AnimationDocker::slotAddBlankFrame()
{
    if (!m_canvas) return;

    KisNodeSP node = m_canvas->viewManager()->activeNode();
    if (!node) return;

    const int time = m_canvas->image()->animationInterface()->currentTime();
    KisAnimationUtils::createKeyframeLazy(m_canvas->image(), node, time, false);
}

void AnimationDocker::slotAddDuplicateFrame()
{
    if (!m_canvas) return;

    KisNodeSP node = m_canvas->viewManager()->activeNode();
    if (!node) return;

    const int time = m_canvas->image()->animationInterface()->currentTime();
    KisAnimationUtils::createKeyframeLazy(m_canvas->image(), node, time, true);
}

void AnimationDocker::slotDeleteKeyframe()
{
    if (!m_canvas) return;

    KisNodeSP node = m_canvas->viewManager()->activeNode();
    if (!node) return;

    const int time = m_canvas->image()->animationInterface()->currentTime();
    KisAnimationUtils::removeKeyframe(m_canvas->image(), node, time);
}

void AnimationDocker::slotUIRangeChanged()
{
    if (!m_canvas || !m_canvas->image()) return;

    int fromTime = m_animationWidget->spinFromFrame->value();
    int toTime = m_animationWidget->spinToFrame->value();

    m_canvas->image()->animationInterface()->setRange(KisTimeRange::fromTime(fromTime, toTime));
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
    KisImageConfig cfg;

    const bool value = cfg.lazyFrameCreationEnabled();

    m_lazyFrameAction->setIcon(value ?
                               KisIconUtils::loadIcon("lazyframeOn") :
                               KisIconUtils::loadIcon("lazyframeOff"));

    m_lazyFrameAction->setText(QString("%1 (%2)")
                               .arg(KisAnimationUtils::lazyFrameCreationActionName)
                               .arg(KritaUtils::toLocalizedOnOff(value)));
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
    m_addBlankFrameAction->setIcon(KisIconUtils::loadIcon("addblankframe"));
    m_addDuplicateFrameAction->setIcon(KisIconUtils::loadIcon("addduplicateframe"));
    m_deleteKeyframeAction->setIcon(KisIconUtils::loadIcon("deletekeyframe"));

    updateLazyFrameIcon();

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
}

void AnimationDocker::slotLazyFrameChanged(bool value)
{
    KisImageConfig cfg;

    if (value != cfg.lazyFrameCreationEnabled()) {
        cfg.setLazyFrameCreationEnabled(value);
        updateLazyFrameIcon();
    }
}

#include "animation_docker.moc"
