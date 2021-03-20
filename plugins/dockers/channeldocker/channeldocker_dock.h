/*
 *  SPDX-FileCopyrightText: 2011 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef CHANNELDOCKER_DOCK_H
#define CHANNELDOCKER_DOCK_H

#include <QPointer>
#include <kddockwidgets/DockWidget.h>

#include <KoCanvasObserverBase.h>
#include <KisKineticScroller.h>

#include <kis_canvas2.h>

class ChannelModel;
class QTableView;
class KisSignalCompressor;
class KisIdleWatcher;

class ChannelDockerDock : public KDDockWidgets::DockWidget, public KoCanvasObserverBase {
    Q_OBJECT
public:
    ChannelDockerDock();

    QString observerName() override { return "ChannelDockerDock"; }
    void setCanvas(KoCanvasBase *canvas) override;
    void unsetCanvas() override;
    void showEvent(QShowEvent *event) override;

public Q_SLOTS:
    void startUpdateCanvasProjection();
    void slotScrollerStateChanged(QScroller::State state){KisKineticScroller::updateCursor(this, state);}

private Q_SLOTS:
    void updateChannelTable(void);

private:
    KisIdleWatcher* m_imageIdleWatcher;
    KisSignalCompressor *m_compressor;
    QPointer<KisCanvas2> m_canvas;
    QTableView *m_channelTable;
    ChannelModel *m_model;
};


#endif
