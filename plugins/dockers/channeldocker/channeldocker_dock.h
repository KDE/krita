/*
 *  SPDX-FileCopyrightText: 2011 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef CHANNELDOCKER_DOCK_H
#define CHANNELDOCKER_DOCK_H

#include <QPointer>
#include <QDockWidget>

#include <KoCanvasObserverBase.h>
#include <KisKineticScroller.h>

#include "KisWidgetWithIdleTask.h"
#include <kis_canvas2.h>

class ChannelModel;
class QTableView;

class ChannelDockerDock : public KisWidgetWithIdleTask<QDockWidget>, public KoCanvasObserverBase {
    Q_OBJECT
public:
    ChannelDockerDock();

    QString observerName() override { return "ChannelDockerDock"; }
    void setCanvas(KoCanvasBase *canvas) override;
    void unsetCanvas() override;

public Q_SLOTS:
    void startUpdateCanvasProjection();
    void slotScrollerStateChanged(QScroller::State state){KisKineticScroller::updateCursor(this, state);}

private Q_SLOTS:
    void updateImageThumbnails(const QVector<QImage> &channels, const KoColorSpace *cs);

private:
    KisIdleTasksManager::TaskGuard registerIdleTask(KisCanvas2 *canvas) override;
    void clearCachedState() override;

private:
    QTableView *m_channelTable {nullptr};
    ChannelModel *m_model {nullptr};
};


#endif
