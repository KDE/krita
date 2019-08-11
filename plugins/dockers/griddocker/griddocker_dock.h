/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
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
