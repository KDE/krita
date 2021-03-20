/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef _GRID_DOCK_H_
#define _GRID_DOCK_H_

#include <kddockwidgets/DockWidget.h>
#include <KoCanvasObserverBase.h>
#include "kis_signal_auto_connection.h"

class KisCanvas2;
class ArrangeDockerWidget;
class KisSignalAutoConnection;

class ArrangeDockerDock : public KDDockWidgets::DockWidget, public KoCanvasObserverBase {
    Q_OBJECT
public:
    ArrangeDockerDock();
    ~ArrangeDockerDock() override;
    QString observerName() override { return "ArrangeDockerDock"; }
    void setCanvas(KoCanvasBase *canvas) override;
    void unsetCanvas() override;

private Q_SLOTS:
    void slotToolChanged();
    void slotToolChanged(QString toolId);

private:
    ArrangeDockerWidget *m_configWidget;
    QPointer<KisCanvas2> m_canvas;
    KisSignalAutoConnectionsStore m_canvasConnections;
};


#endif
