/*
 *  SPDX-FileCopyrightText: 2012 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
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
    void exportImageClicked();
    void exportAnimationClicked();
    void activateCurrentIndex();
    void customContextMenuRequested(QPoint pos);
    void updateComposition();
    void renameComposition();
    void moveCompositionUp();
    void moveCompositionDown();

private:
    QPointer<KisCanvas2> m_canvas;
    CompositionModel *m_model;

    QVector<KisAction*> m_actions;
};


#endif

