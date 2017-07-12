/*
 *  Copyright (c) 2017 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef TOUCHDOCKER_DOCK_H
#define TOUCHDOCKER_DOCK_H

#include <QDockWidget>
#include <KoCanvasObserverBase.h>

class KisCanvas2;

class TouchDockerDock : public QDockWidget, public KoCanvasObserverBase {
    Q_OBJECT
public:
    TouchDockerDock();
    ~TouchDockerDock() override;
    QString observerName() override { return "TouchDockerDock"; }
    void setCanvas(KoCanvasBase *canvas) override;
    void unsetCanvas() override;

private:
    KisCanvas2 *m_canvas {0};
};


#endif

