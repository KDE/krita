/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef _GRID_DOCK_H_
#define _GRID_DOCK_H_

#include <QDockWidget>
#include <KoCanvasObserverBase.h>
#include "kis_signal_auto_connection.h"

class QVBoxLayout;
class KisCanvas2;
class GridConfigWidget;
class KisSignalAutoConnection;
class KisGridConfig;
class KisGuidesConfig;

class GridDockerDock : public QDockWidget, public KoCanvasObserverBase {
    Q_OBJECT
public:
    GridDockerDock();
    ~GridDockerDock() override;
    QString observerName() override { return "GridDockerDock"; }
    void setCanvas(KoCanvasBase *canvas) override;
    void unsetCanvas() override;

public Q_SLOTS:
    void slotGuiGridConfigChanged();
    void slotGridConfigUpdateRequested(const KisGridConfig &config);

    void slotGuiGuidesConfigChanged();
    void slotGuidesConfigUpdateRequested(const KisGuidesConfig &config);

private:
    GridConfigWidget *m_configWidget;
    QPointer<KisCanvas2> m_canvas;
    KisSignalAutoConnectionsStore m_canvasConnections;
};


#endif
