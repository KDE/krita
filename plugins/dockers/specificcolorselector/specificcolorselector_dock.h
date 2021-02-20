
/*
 *  SPDX-FileCopyrightText: 2008 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
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
