/*
 *  Copyright (c) 2011 Sven Langkamp <sven.langkamp@gmail.com>
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

#ifndef CHANNELDOCKER_DOCK_H
#define CHANNELDOCKER_DOCK_H

#include <QPointer>
#include <QDockWidget>

#include <KoCanvasObserverBase.h>
#include <KisKineticScroller.h>

#include <kis_canvas2.h>

class ChannelModel;
class QTableView;
class KisSignalCompressor;
class KisIdleWatcher;

class ChannelDockerDock : public QDockWidget, public KoCanvasObserverBase {
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
