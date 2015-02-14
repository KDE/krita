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

#ifndef _ANIMATION_DOCK_H_
#define _ANIMATION_DOCK_H_

#include <QDockWidget>

#include <KoCanvasObserverBase.h>

class KisCanvas2;
class Ui_WdgAnimation;

class AnimationDockerDock : public QDockWidget, public KoCanvasObserverBase {
    Q_OBJECT
public:
    AnimationDockerDock();
    QString observerName() { return "AnimationDockerDock"; }
    virtual void setCanvas(KoCanvasBase *canvas);
    virtual void unsetCanvas();

private slots:
    void slotAddBlankFrame();
    void slotPreviousFrame();
    void slotNextFrame();

private:

    KisCanvas2 *m_canvas;
    Ui_WdgAnimation *m_animationWidget;
};


#endif
