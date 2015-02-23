/*
 *  Copyright (c) 2015 Jouni Pentikäinen <mctyyppi42@gmail.com>
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

#include "ui_wdg_animation.h"

AnimationDocker::AnimationDocker()
    : QDockWidget(i18n("Animation"))
    , m_canvas(0)
    , m_animationWidget(new Ui_WdgAnimation)
{
    QWidget* mainWidget = new QWidget(this);
    setWidget(mainWidget);

    m_animationWidget->setupUi(mainWidget);

    m_animationWidget->btnPreviousFrame->setIcon(themedIcon("prevframe"));
    m_animationWidget->btnPreviousFrame->setIconSize(QSize(22, 22));

    m_animationWidget->btnPlay->setIcon(themedIcon("playpause"));
    m_animationWidget->btnPlay->setIconSize(QSize(22, 22));

    m_animationWidget->btnNextFrame->setIcon(themedIcon("nextframe"));
    m_animationWidget->btnNextFrame->setIconSize(QSize(22, 22));

    m_animationWidget->btnAddKeyframe->setIcon(themedIcon("addblankframe"));
    m_animationWidget->btnAddKeyframe->setIconSize(QSize(22, 22));

    m_animationWidget->btnAddDuplicateFrame->setIcon(themedIcon("addduplicateblankframe"));
    m_animationWidget->btnAddDuplicateFrame->setIconSize(QSize(22, 22));

    m_animationWidget->btnDeleteKeyframe->setIcon(themedIcon("deletekeyframe"));
    m_animationWidget->btnDeleteKeyframe->setIconSize(QSize(22, 22));

    connect(m_animationWidget->btnPreviousFrame, SIGNAL(clicked()), this, SLOT(slotPreviousFrame()));
    connect(m_animationWidget->btnPlay, SIGNAL(clicked()), this, SLOT(slotPlayPause()));
    connect(m_animationWidget->btnNextFrame, SIGNAL(clicked()), this, SLOT(slotNextFrame()));

    connect(m_animationWidget->btnAddKeyframe, SIGNAL(clicked()), this, SLOT(slotAddBlankFrame()));
    connect(m_animationWidget->btnAddDuplicateFrame, SIGNAL(clicked()), this, SLOT(slotAddDuplicateFrame()));
    connect(m_animationWidget->btnDeleteKeyframe, SIGNAL(clicked()), this, SLOT(slotDeleteKeyframe()));
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

#include "animation_docker.moc"
