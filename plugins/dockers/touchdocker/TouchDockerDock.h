/*
 *  SPDX-FileCopyrightText: 2009 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2021 Deif Lou <ginoba@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef _TOUCHDOCKER_DOCK_H_
#define _TOUCHDOCKER_DOCK_H_

#include <QPointer>
#include <QDockWidget>

#include <KoCanvasObserverBase.h>
#include <kis_canvas2.h>

class TouchDockerWidget;

class TouchDockerDock : public QDockWidget, public KoCanvasObserverBase {
    Q_OBJECT
public:
    TouchDockerDock();
    ~TouchDockerDock() override;
    QString observerName() override { return "TouchDockerDock"; }
    void setCanvas(KoCanvasBase *canvas) override;
    void unsetCanvas() override;

private:
    TouchDockerWidget *m_page {nullptr};
    QPointer<KisCanvas2> m_canvas;
};


#endif /* _TOUCHDOCKER_DOCK_H_ */
