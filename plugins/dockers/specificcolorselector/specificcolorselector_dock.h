
/*
 *  Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License, or
 *  (at your option) any later version.
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

#ifndef _SPECIFICCOLORSELECTOR_DOCK_H_
#define _SPECIFICCOLORSELECTOR_DOCK_H_

#include <QDockWidget>
#include <QPointer>

#include <kis_types.h>
#include <kis_mainwindow_observer.h>
#include <kis_canvas2.h>

class KisViewManager;
class KisSpecificColorSelectorWidget;

class SpecificColorSelectorDock : public QDockWidget, public KisMainwindowObserver
{
    Q_OBJECT
public:
    SpecificColorSelectorDock();
    QString observerName() override { return "SpecificColorSelectorDock"; }
    /// reimplemented from KoCanvasObserverBase/KisMainwindowObserver
    void setCanvas(KoCanvasBase *canvas) override;
    void unsetCanvas() override;
    void setViewManager(KisViewManager* kisview) override;

private:
    QPointer<KisCanvas2> m_canvas;
    KisViewManager *m_view;
    KisSpecificColorSelectorWidget* m_colorSelector;
};


#endif
