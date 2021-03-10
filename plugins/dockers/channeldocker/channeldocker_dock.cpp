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

#include "channeldocker_dock.h"

#include <QGridLayout>
#include <QTableView>
#include <QHeaderView>
#include <klocalizedstring.h>

#include <KoCanvasBase.h>
#include "channelmodel.h"
#include <KisViewManager.h>
#include <kis_canvas2.h>
#include <kis_layer.h>
#include <kis_node_manager.h>
#include <kis_image.h>
#include <kis_group_layer.h>
#include <kis_paint_device.h>
#include "kis_signal_compressor.h"
#include <KisView.h>
#include <kis_idle_watcher.h>

ChannelDockerDock::ChannelDockerDock( ) :
    QDockWidget(i18n("Channels")),
    m_imageIdleWatcher(new KisIdleWatcher(250, this)),
    m_canvas(0)
{
    m_channelTable = new QTableView(this);
    m_model = new ChannelModel(this);
    m_channelTable->setModel(m_model);
    m_channelTable->setShowGrid(false);
    m_channelTable->horizontalHeader()->setStretchLastSection(true);
    m_channelTable->verticalHeader()->setVisible(false);
    m_channelTable->horizontalHeader()->setVisible(false);
    m_channelTable->setSelectionBehavior( QAbstractItemView::SelectRows );

    QScroller *scroller = KisKineticScroller::createPreconfiguredScroller(m_channelTable);
    if (scroller){
        connect(scroller, SIGNAL(stateChanged(QScroller::State)),
                this, SLOT(slotScrollerStateChanged(QScroller::State)));
    }

    setWidget(m_channelTable);

    connect(m_channelTable,&QTableView::activated, m_model, &ChannelModel::rowActivated);
}

void ChannelDockerDock::setCanvas(KoCanvasBase * canvas)
{
    if(m_canvas == canvas)
        return;

    setEnabled(canvas != 0);

    if (m_canvas) {
        m_canvas->disconnectCanvasObserver(this);
        m_canvas->image()->disconnect(this);
    }

    if (!canvas) {
        m_canvas = 0;
        return;
    }

    m_canvas = dynamic_cast<KisCanvas2*>(canvas);
    if ( m_canvas && m_canvas->image() ) {
        m_model->slotSetCanvas(m_canvas);

        KisPaintDeviceSP dev = m_canvas->image()->projection();

        m_imageIdleWatcher->setTrackedImage(m_canvas->image());
        connect(m_imageIdleWatcher, &KisIdleWatcher::startedIdleMode, this, &ChannelDockerDock::updateChannelTable);

        connect(m_canvas->image(), SIGNAL(sigImageUpdated(QRect)), this, SLOT(startUpdateCanvasProjection()), Qt::UniqueConnection);

        connect(dev, SIGNAL(colorSpaceChanged(const KoColorSpace*)), m_model, SLOT(slotColorSpaceChanged(const KoColorSpace*)));
        connect(dev, SIGNAL(colorSpaceChanged(const KoColorSpace*)), m_canvas, SLOT(channelSelectionChanged()));
        connect(m_model, SIGNAL(channelFlagsChanged()), m_canvas, SLOT(channelSelectionChanged()));
        m_imageIdleWatcher->startCountdown();
    }

}

void ChannelDockerDock::unsetCanvas()
{
    setEnabled(false);
    m_canvas = 0;
    m_model->unsetCanvas();
}

void ChannelDockerDock::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);
    m_imageIdleWatcher->startCountdown();
}

void ChannelDockerDock::startUpdateCanvasProjection()
{
    m_imageIdleWatcher->startCountdown();
}

void ChannelDockerDock::updateChannelTable()
{
    if (isVisible() && m_canvas && m_canvas->image()){
        m_model->updateData(m_canvas);
        m_channelTable->resizeRowsToContents();
        m_channelTable->resizeColumnsToContents();
    }
}




