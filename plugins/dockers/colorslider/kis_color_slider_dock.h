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
#include <QPointer>
#include <KoCanvasObserverBase.h>
#include "kis_signal_auto_connection.h"

#include <kis_types.h>
#include <kis_canvas2.h>

class KisViewManager;
class KisColorSliderWidget;

class ColorSliderDock : public QDockWidget, public KoCanvasObserverBase
{
    Q_OBJECT
public:
    ColorSliderDock();
    QString observerName() override { return "ColorSliderDock"; }
    /// reimplemented from KoCanvasObserverBase
    void setCanvas(KoCanvasBase *canvas) override;
    void unsetCanvas() override;
public Q_SLOTS:
    void udpateSliders();

private:
    QPointer<KisCanvas2> m_canvas;
    KisViewManager *m_view;
    KisColorSliderWidget* m_colorSliders;
    KisSignalAutoConnectionsStore m_canvasConnections;
};


#endif
