/*
 *  Copyright (c) 2013 Somsubhra Bairi <somsubhra.bairi@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _ANIMATOR_DOCK_H_
#define _ANIMATOR_DOCK_H_

#include <QDockWidget>
#include <QSpinBox>
#include <KoCanvasObserverBase.h>

class QLabel;
class KisCanvas2;
class KisAnimation;
class KisTimeline;

class AnimatorDock : public QDockWidget, public KoCanvasObserverBase {
    Q_OBJECT
public:
    AnimatorDock();
    virtual void setCanvas(KoCanvasBase *canvas);
    virtual void unsetCanvas(){ m_canvas = 0;}

public slots:
    void updateNumberOfFrames();

private:
    KisCanvas2 *m_canvas;
    KisAnimation *m_animation;
    KisTimeline* m_mainWidget;
};


#endif
