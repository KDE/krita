/*
 *  Copyright (c) 2011 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef TASKSETDOCKER_DOCK_H
#define TASKSETDOCKER_DOCK_H

#include <QDockWidget>
#include <QModelIndex>
#include <KoCanvasObserverBase.h>
#include "ui_wdgtasksetdocker.h"
#include <KoResourceServer.h>
#include "taskset_resource.h"


class KoResourceLoaderThread;
class TasksetModel;
class QListView;
class QThread;
class KisCanvas2;

class TasksetDockerDock : public QDockWidget, public KoCanvasObserverBase, public Ui_WdgTasksetDocker {
    Q_OBJECT
public:
    TasksetDockerDock();
    ~TasksetDockerDock();

    virtual void setCanvas(KoCanvasBase *canvas);
    virtual void unsetCanvas();
    
private slots:
    void actionTriggered(QAction* action);
    void activated (const QModelIndex& index);
    void recordClicked();
    void saveClicked();
    void clearClicked();
    void resourceSelected( KoResource * resource );

private:
    KisCanvas2 *m_canvas;
    TasksetModel *m_model;
    bool m_blocked;
    KoResourceServer<TasksetResource>* m_rserver;
    KoResourceLoaderThread *m_taskThread;
};


#endif

