/*
 *  Copyright (c) 2013 Somsubhra Bairi <somsubhra.bairi@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License, or(at you option)
 *  any later version..
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

#ifndef ONIONSKIN_DOCK_H
#define ONIONSKIN_DOCK_H

#include <QDockWidget>
#include <KoCanvasObserverBase.h>
#include <QCheckBox>
#include <QSpinBox>
#include <KColorButton>
#include "kis_opacity_selector_view.h"
class KisCanvas2;
class KisAnimation;

class OnionSkinDock : public QDockWidget, public KoCanvasObserverBase
{
    Q_OBJECT
public:
    OnionSkinDock();
    virtual void setCanvas(KoCanvasBase *canvas);
    virtual void unsetCanvas(){ m_canvas = 0;}

private:
    KisCanvas2* m_canvas;
    KisAnimation* m_animation;
    QCheckBox *m_activeCheckBox;
    QSpinBox* m_previousFramesInput;
    QSpinBox* m_nextFramesInput;
    KColorButton* m_previousFramesColor;
    KColorButton* m_nextFramesColor;
    KisOpacitySelectorView* m_previousOpacitySelectorView;
    KisOpacitySelectorView* m_nextOpacitySelectorView;
};

#endif // ONIONSKIN_DOCK_H
