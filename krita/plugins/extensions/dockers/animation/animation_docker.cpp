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
#include "kis_onion_skin_dialog.h"
#include "kis_time_range.h"
#include "kundo2command.h"
#include "kis_post_execution_undo_adapter.h"
#include "kis_keyframe_channel.h"



#include "ui_wdg_animation.h"

AnimationDocker::AnimationDocker()
    : QDockWidget(i18n("Animation"))
    , m_canvas(0)
    , m_animationWidget(new Ui_WdgAnimation)
    , m_onionSkinOptions(new KisOnionSkinDialog())
{
    QWidget* mainWidget = new QWidget(this);
    setWidget(mainWidget);

    m_animationWidget->setupUi(mainWidget);

    m_previousFrameAction = new KisAction(i18n("Move to previous frame"), m_animationWidget->btnPreviousFrame);
    m_previousFrameAction->setActivationFlags(KisAction::ACTIVE_IMAGE);
    m_animationWidget->btnPreviousFrame->setDefaultAction(m_previousFrameAction);

    m_nextFrameAction = new KisAction(i18n("Move to next frame"), m_animationWidget->btnNextFrame);
    m_nextFrameAction->setActivationFlags(KisAction::ACTIVE_IMAGE);
    m_animationWidget->btnNextFrame->setDefaultAction(m_nextFrameAction);

    m_playPauseAction = new KisAction(i18n("Play / pause animation"), m_animationWidget->btnPlay);
    m_playPauseAction->setActivationFlags(KisAction::ACTIVE_IMAGE);
    m_animationWidget->btnPlay->setDefaultAction(m_playPauseAction);

    m_addBlankFrameAction = new KisAction(i18n("Add blank frame"), m_animationWidget->btnAddKeyframe);
    m_addBlankFrameAction->setActivationFlags(KisAction::ACTIVE_LAYER);
    m_animationWidget->btnAddKeyframe->setDefaultAction(m_addBlankFrameAction);

    m_addDuplicateFrameAction = new KisAction(i18n("Add duplicate frame"), m_animationWidget->btnAddDuplicateFrame);
    m_addDuplicateFrameAction->setActivationFlags(KisAction::ACTIVE_LAYER);
    m_animationWidget->btnAddDuplicateFrame->setDefaultAction(m_addDuplicateFrameAction);

    m_deleteKeyframeAction = new KisAction(i18n("Delete keyframe"), m_animationWidget->btnDeleteKeyframe);
    m_deleteKeyframeAction->setActivationFlags(KisAction::ACTIVE_LAYER);
    m_animationWidget->btnDeleteKeyframe->setDefaultAction(m_deleteKeyframeAction);

    connect(m_previousFrameAction, SIGNAL(triggered()), this, SLOT(slotPreviousFrame()));
    connect(m_nextFrameAction, SIGNAL(triggered()), this, SLOT(slotNextFrame()));
    connect(m_playPauseAction, SIGNAL(triggered()), this, SLOT(slotPlayPause()));

    connect(m_addBlankFrameAction, SIGNAL(triggered()), this, SLOT(slotAddBlankFrame()));
    connect(m_addDuplicateFrameAction, SIGNAL(triggered()), this, SLOT(slotAddDuplicateFrame()));
    connect(m_deleteKeyframeAction, SIGNAL(triggered()), this, SLOT(slotDeleteKeyframe()));

    connect(m_animationWidget->btnOnionSkinOptions, SIGNAL(clicked()), this, SLOT(slotOnionSkinOptions()));

    connect(m_animationWidget->spinFromFrame, SIGNAL(valueChanged(int)), this, SLOT(slotUIRangeChanged()));
    connect(m_animationWidget->spinToFrame, SIGNAL(valueChanged(int)), this, SLOT(slotUIRangeChanged()));
    connect(m_animationWidget->doubleFramerate, SIGNAL(valueChanged(double)), this, SLOT(slotUIFramerateChanged()));
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
        m_animationWidget->doubleFramerate->setValue(animation->framerate());

        connect(animation, SIGNAL(sigTimeChanged(int)), this, SLOT(slotTimeChanged()));
        connect(m_canvas->animationPlayer(), SIGNAL(sigFrameChanged()), this, SLOT(slotTimeChanged()));
        connect(m_canvas->animationPlayer(), SIGNAL(sigPlaybackStopped()), this, SLOT(slotTimeChanged()));

        slotTimeChanged();
    }
}

void AnimationDocker::unsetCanvas()
{
    setEnabled(false);
    m_canvas = 0;
}

void AnimationDocker::setMainWindow(KisViewManager *view)
{
    KisActionManager *actionManager = view->actionManager();

    actionManager->addAction("previous_frame", m_previousFrameAction);
    actionManager->addAction("next_frame", m_nextFrameAction);
    actionManager->addAction("toggle_playback", m_playPauseAction);
    actionManager->addAction("add_blank_frame", m_addBlankFrameAction);
    actionManager->addAction("add_duplicate_frame", m_addDuplicateFrameAction);
    actionManager->addAction("delete_keyframe", m_deleteKeyframeAction);

    connect(view->mainWindow(), SIGNAL(themeChanged()), this, SLOT(slotUpdateIcons()));
}

void AnimationDocker::slotAddBlankFrame()
{
    if (!m_canvas) return;

    KisNodeSP node = m_canvas->viewManager()->activeNode();
    if (!node) return;

    if (!node->isAnimated()) {
        node->enableAnimation();
    }

    KisKeyframeChannel *rasterChannel = node->getKeyframeChannel(KisKeyframeChannel::Content.id());
    if (rasterChannel) {
        KUndo2Command *cmd = new KUndo2Command(kundo2_i18n("Add Keyframe"));
        int time = m_canvas->image()->animationInterface()->currentTime();
        rasterChannel->addKeyframe(time, cmd);
        m_canvas->image()->postExecutionUndoAdapter()->addCommand(toQShared(cmd));
    }
}

void AnimationDocker::slotAddDuplicateFrame()
{
    if (!m_canvas) return;

    KisNodeSP node = m_canvas->viewManager()->activeNode();
    if (!node) return;

    if (!node->isAnimated()) {
        node->enableAnimation();
    }

    KisKeyframeChannel *rasterChannel = node->getKeyframeChannel(KisKeyframeChannel::Content.id());
    if (rasterChannel) {
        KUndo2Command *cmd = new KUndo2Command(kundo2_i18n("Add Keyframe"));
        int time = m_canvas->image()->animationInterface()->currentTime();

        KisKeyframeSP keyframe = rasterChannel->activeKeyframeAt(time);
        rasterChannel->copyKeyframe(keyframe.data(), time, cmd);
        m_canvas->image()->postExecutionUndoAdapter()->addCommand(toQShared(cmd));
    }
}

void AnimationDocker::slotDeleteKeyframe()
{
    if (!m_canvas) return;

    KisNodeSP node = m_canvas->viewManager()->activeNode();
    if (!node) return;

    KisKeyframeChannel *rasterChannel = node->getKeyframeChannel(KisKeyframeChannel::Content.id());
    if (rasterChannel) {
        KUndo2Command *cmd = new KUndo2Command(kundo2_i18n("Add Keyframe"));
        int time = m_canvas->image()->animationInterface()->currentTime();

        KisKeyframe *keyframe = rasterChannel->keyframeAt(time);
        if (keyframe) {
            rasterChannel->deleteKeyframe(keyframe, cmd);
            m_canvas->image()->postExecutionUndoAdapter()->addCommand(toQShared(cmd));
        }
    }
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

    m_canvas->image()->animationInterface()->setFramerate(m_animationWidget->doubleFramerate->value());
}

void AnimationDocker::slotOnionSkinOptions()
{
    m_onionSkinOptions->show();
}

void AnimationDocker::slotTimeChanged()
{
    int time = m_canvas->animationPlayer()->isPlaying() ?
                m_canvas->animationPlayer()->currentTime() :
                m_canvas->image()->animationInterface()->currentUITime();

    m_animationWidget->lblInfo->setText("Frame: " + QString::number(time));
}

void AnimationDocker::slotPreviousFrame()
{
    if (!m_canvas) return;

    int time = m_canvas->image()->animationInterface()->currentUITime() - 1;

    if (time >= 0) {
        m_canvas->image()->animationInterface()->requestTimeSwitchWithUndo(time);
    }
}

void AnimationDocker::slotNextFrame()
{
    if (!m_canvas) return;

    int time = m_canvas->image()->animationInterface()->currentUITime() + 1;
    m_canvas->image()->animationInterface()->requestTimeSwitchWithUndo(time);
}

void AnimationDocker::slotPlayPause()
{
    if (!m_canvas) return;

    if (m_canvas->animationPlayer()->isPlaying()) {
        m_canvas->animationPlayer()->stop();
    } else {
        m_canvas->animationPlayer()->play();
    }
}

void AnimationDocker::slotUpdateIcons()
{
    m_previousFrameAction->setIcon(KisIconUtils::loadIcon("prevframe"));
    m_nextFrameAction->setIcon(KisIconUtils::loadIcon("nextframe"));
    m_playPauseAction->setIcon(KisIconUtils::loadIcon("playpause"));
    m_addBlankFrameAction->setIcon(KisIconUtils::loadIcon("addblankframe"));
    m_addDuplicateFrameAction->setIcon(KisIconUtils::loadIcon("addduplicateframe"));
    m_deleteKeyframeAction->setIcon(KisIconUtils::loadIcon("deletekeyframe"));

    m_animationWidget->btnOnionSkinOptions->setIcon(KisIconUtils::loadIcon("onion_skin_options"));
    m_animationWidget->btnOnionSkinOptions->setIconSize(QSize(22, 22));

    m_animationWidget->btnPreviousFrame->setIconSize(QSize(22, 22));
    m_animationWidget->btnPlay->setIconSize(QSize(22, 22));
    m_animationWidget->btnNextFrame->setIconSize(QSize(22, 22));
    m_animationWidget->btnAddKeyframe->setIconSize(QSize(22, 22));
    m_animationWidget->btnAddDuplicateFrame->setIconSize(QSize(22, 22));
    m_animationWidget->btnDeleteKeyframe->setIconSize(QSize(22, 22));
}

#include "animation_docker.moc"
