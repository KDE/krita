/*
 *  Copyright (c) 2012 Sven Langkamp <sven.langkamp@gmail.com>
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

#ifndef COMPOSITIONDOCKER_DOCK_H
#define COMPOSITIONDOCKER_DOCK_H

#include <QDockWidget>
#include <QModelIndex>
#include <KoCanvasObserverBase.h>
#include "ui_wdgcompositiondocker.h"

class CompositionModel;
class QListView;
class KisCanvas2;
class KisAction;

class CompositionDockerDock : public QDockWidget, public KoCanvasObserverBase, public Ui_WdgCompositionDocker {
    Q_OBJECT
public:
    CompositionDockerDock();
    ~CompositionDockerDock();
    QString observerName() { return "CompositionDockerDock"; }
    virtual void setCanvas(KoCanvasBase *canvas);
    virtual void unsetCanvas();
    
    void updateModel();
protected:
    bool eventFilter(QObject *obj, QEvent *event);
    
private slots:
    void activated (const QModelIndex& index);
    void deleteClicked();
    void saveClicked();
    void exportClicked();
    void activateCurrentIndex();
    void customContextMenuRequested(QPoint pos);
    void updateComposition();
    void renameComposition();

private:
    KisCanvas2 *m_canvas;
    CompositionModel *m_model;

    QVector<KisAction*> m_actions;
    KisAction* updateAction;
    KisAction* renameAction;
};


#endif

