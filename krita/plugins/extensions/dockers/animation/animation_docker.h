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

#ifndef _ANIMATION_DOCKER_H_
#define _ANIMATION_DOCKER_H_

#include "krita_export.h"

#include <QDockWidget>

#include <kis_mainwindow_observer.h>
#include "kis_action.h"


class KisCanvas2;
class Ui_WdgAnimation;

class AnimationDocker : public QDockWidget, public KisMainwindowObserver {
    Q_OBJECT
public:
    AnimationDocker();
    QString observerName() { return "AnimationDocker"; }
    virtual void setCanvas(KoCanvasBase *canvas);
    virtual void unsetCanvas();
    void setMainWindow(KisViewManager *kisview);

private slots:
    void slotPreviousFrame();
    void slotNextFrame();
    void slotPlayPause();

    void slotAddBlankFrame();
    void slotAddDuplicateFrame();
    void slotDeleteKeyframe();

    void slotUpdateIcons();

private:

    KisCanvas2 *m_canvas;
    Ui_WdgAnimation *m_animationWidget;

    KisAction *m_previousFrameAction;
    KisAction *m_nextFrameAction;
    KisAction *m_playPauseAction;

    KisAction *m_addBlankFrameAction;
    KisAction *m_addDuplicateFrameAction;
    KisAction *m_deleteKeyframeAction;

};


#endif
