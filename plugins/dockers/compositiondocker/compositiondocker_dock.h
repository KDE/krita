/*
 *  Copyright (c) 2012 Sven Langkamp <sven.langkamp@gmail.com>
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

#ifndef COMPOSITIONDOCKER_DOCK_H
#define COMPOSITIONDOCKER_DOCK_H

#include <QDockWidget>
#include <QModelIndex>
#include <QPointer>

#include <KoCanvasObserverBase.h>

#include <kis_canvas2.h>

#include "ui_wdgcompositiondocker.h"
#include <KisKineticScroller.h>

class CompositionModel;
class KisAction;

class CompositionDockerDock : public QDockWidget, public KoCanvasObserverBase, public Ui_WdgCompositionDocker {
    Q_OBJECT
public:
    CompositionDockerDock();
    ~CompositionDockerDock() override;
    QString observerName() override { return "CompositionDockerDock"; }
    void setCanvas(KoCanvasBase *canvas) override;
    void unsetCanvas() override;
    
    void updateModel();
protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

public Q_SLOTS:
    void slotScrollerStateChanged(QScroller::State state){ KisKineticScroller::updateCursor(this, state); }
    
private Q_SLOTS:
    void activated (const QModelIndex& index);
    void deleteClicked();
    void saveClicked();
    void exportClicked();
    void activateCurrentIndex();
    void customContextMenuRequested(QPoint pos);
    void updateComposition();
    void renameComposition();

private:
    QPointer<KisCanvas2> m_canvas;
    CompositionModel *m_model;

    QVector<KisAction*> m_actions;
};


#endif

