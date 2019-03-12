/*
 *  Copyright (c) 2011 Sven Langkamp <sven.langkamp@gmail.com>
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

#ifndef TASKSETDOCKER_DOCK_H
#define TASKSETDOCKER_DOCK_H

#include <QDockWidget>
#include <QModelIndex>
#include <QPointer>

#include <KoCanvasObserverBase.h>
#include <KoResourceServer.h>

#include <kis_canvas2.h>

#include "taskset_resource.h"
#include "ui_wdgtasksetdocker.h"

class TasksetModel;

class TasksetDockerDock : public QDockWidget, public KoCanvasObserverBase, public Ui_WdgTasksetDocker {
    Q_OBJECT
public:
    TasksetDockerDock();
    ~TasksetDockerDock() override;
    QString observerName() override { return "TasksetDockerDock"; }
    void setCanvas(KoCanvasBase *canvas) override;
    void unsetCanvas() override;

private Q_SLOTS:
    void actionTriggered(QAction* action);
    void activated (const QModelIndex& index);
    void recordClicked();
    void saveClicked();
    void clearClicked();
    void resourceSelected( KoResourceSP resource );

private:
    QPointer<KisCanvas2> m_canvas;
    TasksetModel *m_model;
    bool m_blocked;
    KoResourceServer<TasksetResource> *m_rserver {0};
};


#endif

