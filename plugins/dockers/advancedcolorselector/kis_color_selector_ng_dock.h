/*
 *  SPDX-FileCopyrightText: 2008 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2010 Adam Celarek <kdedev at xibo dot at>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KIS_COLOR_SELECTOR_NG_DOCKER_H
#define KIS_COLOR_SELECTOR_NG_DOCKER_H

#include <kddockwidgets/DockWidget.h>
#include <KoCanvasObserverBase.h>

class KisColorSelectorNgDockerWidget;

class KisColorSelectorNgDock : public KDDockWidgets::DockWidget, public KoCanvasObserverBase
{
    Q_OBJECT
public:
    KisColorSelectorNgDock();
    QString observerName() override { return "KisColorSelectorNgDock"; }
    /// reimplemented from KoCanvasObserverBase
    void setCanvas(KoCanvasBase *canvas) override;
    void unsetCanvas() override;
private:
    KisColorSelectorNgDockerWidget *m_colorSelectorNgWidget;
};


#endif
