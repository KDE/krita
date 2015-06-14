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

#include "kis_canvas2.h"
#include "kis_image.h"
#include <klocale.h>
#include <KoIcon.h>
#include "KisViewManager.h"
#include "kis_paint_layer.h"
#include "kis_action_manager.h"

#include "ui_wdg_animation.h"

AnimationDocker::AnimationDocker()
    : QDockWidget(i18n("Animation"))
    , m_canvas(0)
    , m_animationWidget(new Ui_WdgAnimation)
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
}

void AnimationDocker::setCanvas(KoCanvasBase * canvas)
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

    if (node->inherits("KisPaintLayer")) {
        KisPaintLayer *layer = qobject_cast<KisPaintLayer*>(node.data());

        layer->addNewFrame(m_canvas->image()->currentTime(), true);
    }
}

void AnimationDocker::slotAddDuplicateFrame()
{
    if (!m_canvas) return;

    KisNodeSP node = m_canvas->viewManager()->activeNode();
    if (!node) return;

    if (node->inherits("KisPaintLayer")) {
        KisPaintLayer *layer = qobject_cast<KisPaintLayer*>(node.data());

        layer->addNewFrame(m_canvas->image()->currentTime(), false);
    }
}

void AnimationDocker::slotDeleteKeyframe()
{
    if (!m_canvas) return;

    KisNodeSP node = m_canvas->viewManager()->activeNode();
    if (!node) return;

    if (node->inherits("KisPaintLayer")) {
        KisPaintLayer *layer = qobject_cast<KisPaintLayer*>(node.data());

        layer->deleteKeyfame(m_canvas->image()->currentTime());
    }
}

void AnimationDocker::slotPreviousFrame()
{
    if (!m_canvas) return;

    int time = m_canvas->image()->currentTime() - 1;
    m_canvas->image()->seekToTime(time);

    m_animationWidget->lblInfo->setText("Frame: " + QString::number(time));
}

void AnimationDocker::slotNextFrame()
{
    if (!m_canvas) return;

    int time = m_canvas->image()->currentTime() + 1;
    m_canvas->image()->seekToTime(time);

    m_animationWidget->lblInfo->setText("Frame: " + QString::number(time));
}

void AnimationDocker::slotPlayPause()
{
    if (!m_canvas) return;

    if (m_canvas->animationPlayer()->isPlaying()) {
        m_canvas->stopPlayback();
    } else {
        m_canvas->animationPlayer()->setFramerate(m_animationWidget->doubleFramerate->value());
        m_canvas->animationPlayer()->setRange(m_animationWidget->spinFromFrame->value(), m_animationWidget->spinToFrame->value());

        m_canvas->startPlayback();
    }
}

void AnimationDocker::slotUpdateIcons()
{
    m_previousFrameAction->setIcon(themedIcon("prevframe"));
    m_nextFrameAction->setIcon(themedIcon("nextframe"));
    m_playPauseAction->setIcon(themedIcon("playpause"));
    m_addBlankFrameAction->setIcon(themedIcon("addblankframe"));
    m_addDuplicateFrameAction->setIcon(themedIcon("addduplicateframe"));
    m_deleteKeyframeAction->setIcon(themedIcon("deletekeyframe"));

    m_animationWidget->btnPreviousFrame->setIconSize(QSize(22, 22));
    m_animationWidget->btnPlay->setIconSize(QSize(22, 22));
    m_animationWidget->btnNextFrame->setIconSize(QSize(22, 22));
    m_animationWidget->btnAddKeyframe->setIconSize(QSize(22, 22));
    m_animationWidget->btnAddDuplicateFrame->setIconSize(QSize(22, 22));
    m_animationWidget->btnDeleteKeyframe->setIconSize(QSize(22, 22));
}

#include "animation_docker.moc"
