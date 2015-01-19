/*
 *  Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2014 Wolthera van Hövell <griffinvalley@gmail.com>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2.1 of the License.
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

#ifndef _COLORSLIDER_DOCK_H_
#define _COLORSLIDER_DOCK_H_

#include <QDockWidget>
#include <KoCanvasObserverBase.h>

#include <kis_types.h>

class KisViewManager;
class KisCanvas2;
class KisColorSliderWidget;

class ColorSliderDock : public QDockWidget, public KoCanvasObserverBase
{
    Q_OBJECT
public:
    ColorSliderDock();
    QString observerName() { return "ColorSliderDock"; }
    /// reimplemented from KoCanvasObserverBase
    virtual void setCanvas(KoCanvasBase *canvas);
    virtual void unsetCanvas();
public slots:
    void layerChanged(const KisNodeSP);
    
private:
    KisCanvas2 *m_canvas;
    KisViewManager *m_view;
    KoCanvasBase * b_canvas;
    KisColorSliderWidget* m_colorSliders;
};


#endif
