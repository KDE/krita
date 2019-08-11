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

#ifndef _ANIMATION_DOCKER_H_
#define _ANIMATION_DOCKER_H_

#include "kritaimage_export.h"

#include <QDockWidget>
#include <QPointer>

#include <kis_mainwindow_observer.h>
#include <kis_action.h>
#include <kis_canvas2.h>

class Ui_WdgAnimation;
class KisMainWindow;

class AnimationDocker : public QDockWidget, public KisMainwindowObserver {
    Q_OBJECT
public:
    AnimationDocker();
    ~AnimationDocker() override;
    QString observerName() override { return "AnimationDocker"; }
    void setCanvas(KoCanvasBase *canvas) override;
    void unsetCanvas() override;
    void setViewManager(KisViewManager *kisview) override;    

private Q_SLOTS:
    void slotPreviousFrame();
    void slotNextFrame();

    void slotPreviousKeyFrame();
    void slotNextKeyFrame();

    void slotFirstFrame();
    void slotLastFrame();

    void slotPlayPause();

    void slotAddOpacityKeyframe();
    void slotDeleteOpacityKeyframe();

    void slotAddTransformKeyframe();
    void slotDeleteTransformKeyframe();

    void slotUIRangeChanged();
    void slotUIFramerateChanged();

    void slotUpdateIcons();

    void slotOnionSkinOptions();

    void slotGlobalTimeChanged();
    void slotTimeSpinBoxChanged();

    /** Update the frame rate UI field in the case it gets
     *  out of sync with the data model
     */
    void slotFrameRateChanged();

    void updatePlayPauseIcon();
    void updateLazyFrameIcon();
    void updateDropFramesIcon();

    void slotLazyFrameChanged(bool value);
    void slotDropFramesChanged(bool value);

    void slotCurrentNodeChanged(KisNodeSP node);

    void updateClipRange();

private:

    QPointer<KisCanvas2> m_canvas;
    QPointer<KisActionManager> m_actionManager;
    Ui_WdgAnimation *m_animationWidget;

    KisAction *m_previousFrameAction;
    KisAction *m_nextFrameAction;

    KisAction *m_previousKeyFrameAction;
    KisAction *m_nextKeyFrameAction;

    KisAction *m_firstFrameAction;
    KisAction *m_lastFrameAction;

    KisAction *m_playPauseAction;

    KisAction *m_lazyFrameAction;
    KisAction *m_dropFramesAction;

    QMenu *m_newKeyframeMenu;
    KisAction *m_addOpacityKeyframeAction;
    KisAction *m_addTransformKeyframeAction;
    QMenu *m_deleteKeyframeMenu;
    KisAction *m_deleteOpacityKeyframeAction;
    KisAction *m_deleteTransformKeyframeAction;

    KisMainWindow *m_mainWindow;

    void addKeyframe(const QString &channel, bool copy);
    void deleteKeyframe(const QString &channel);

    void setActions(KisActionManager* actionManager);
};


#endif
