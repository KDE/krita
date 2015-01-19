/*
 *  Copyright (c) 2013 Somsubhra Bairi <somsubhra.bairi@gmail.com>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2.1 of the License, or (at your option)
 *  any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef ONIONSKIN_DOCK_H
#define ONIONSKIN_DOCK_H

#include <QDockWidget>

#include <KoCanvasObserverBase.h>

#include "kis_opacity_selector_view.h"

class KisCanvas2;
class KisAnimation;

/**
 * The onion skin docker class
 */
class OnionSkinDock : public QDockWidget, public KoCanvasObserverBase
{
    Q_OBJECT
public:
    OnionSkinDock();
    QString observerName() { return "OnionSkinDock"; }
    virtual void setCanvas(KoCanvasBase *canvas);
    virtual void unsetCanvas() {
        m_canvas = 0;
        setEnabled(false);
    }

private:
    void onCavasSet();

private slots:
    void enableOnionSkinning(bool enable);

    void setNumberOfPrevFrames(int frames);
    void setNumberOfNextFrames(int frames);

    void setPrevFramesOpacityValues();
    void setNextFramesOpacityValues();

    void setPrevFramesColor(QColor color);
    void setNextFramesColor(QColor color);

private:
    KisCanvas2* m_canvas;
    KisAnimation* m_animation;
    KisOpacitySelectorView* m_previousOpacitySelectorView;
    KisOpacitySelectorView* m_nextOpacitySelectorView;
    bool m_initialized;
};

#endif // ONIONSKIN_DOCK_H
