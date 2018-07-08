/*
 *  Copyright (c) 2018 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef _LOGDOCKER_DOCK_H_
#define _LOGDOCKER_DOCK_H_

#include <QDockWidget>

#include <KoCanvasObserverBase.h>

#include "ui_WdgLogDocker.h"

class LogDockerDock : public QDockWidget, public KoCanvasObserverBase, public Ui_WdgLogDocker {
    Q_OBJECT
public:
    LogDockerDock( );
    QString observerName() override { return "LogDockerDock"; }
    void setCanvas(KoCanvasBase *canvas) override;
    void unsetCanvas() override {}

private Q_SLOTS:
private:
};


#endif
