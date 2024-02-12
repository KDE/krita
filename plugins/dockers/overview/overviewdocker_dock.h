/*
 *  SPDX-FileCopyrightText: 2009 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2021 Deif Lou <ginoba@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef _OVERVIEW_DOCK_H_
#define _OVERVIEW_DOCK_H_

#include "overviewdocker_page.h"
#include <QPointer>
#include <QDockWidget>

#include <KoCanvasObserverBase.h>
#include <kis_canvas2.h>

class OverviewWidget;

class OverviewDockerDock : public QDockWidget, public KoCanvasObserverBase {
    Q_OBJECT
public:
    OverviewDockerDock();
    ~OverviewDockerDock() override;
    QString observerName() override { return "OverviewDockerDock"; }
    void setCanvas(KoCanvasBase *canvas) override;
    void unsetCanvas() override;

private:
    OverviewDockerPage *m_page {nullptr};
    QPointer<KisCanvas2> m_canvas;
};


#endif
