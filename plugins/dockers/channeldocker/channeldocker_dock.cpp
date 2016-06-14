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
#include <kis_layer.h>
#include <kis_paint_device.h>
#include "kis_signal_compressor.h"
#include <KisView.h>

ChannelDockerDock::ChannelDockerDock( ) :
    QDockWidget(i18n("Channels")),
    m_compressor(new KisSignalCompressor(500, KisSignalCompressor::POSTPONE, this)),
    m_canvas(0)
{
    m_channelTable = new QTableView(this);
    m_model = new ChannelModel(this);
    m_channelTable->setModel(m_model);
    m_channelTable->setShowGrid(false);
    m_channelTable->verticalHeader()->setVisible(false);
    m_channelTable->horizontalHeader()->setVisible(false);
    setWidget(m_channelTable);
    connect(m_compressor, SIGNAL(timeout()),SLOT(startUpdateCanvasProjection()));
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
        KisPaintDeviceSP dev = m_canvas->image()->projection();

        connect(m_canvas->image(), SIGNAL(sigImageUpdated(QRect)), m_compressor, SLOT(start()), Qt::UniqueConnection);
        connect(dev, SIGNAL(colorSpaceChanged(const KoColorSpace*)), m_model, SLOT(slotColorSpaceChanged(const KoColorSpace*)));
        connect(dev, SIGNAL(colorSpaceChanged(const KoColorSpace*)), m_canvas, SLOT(channelSelectionChanged()));
        connect(m_model, SIGNAL(channelFlagsChanged()), m_canvas, SLOT(channelSelectionChanged()));

        m_compressor->start();
    }

}

void ChannelDockerDock::startUpdateCanvasProjection()
{
    if (m_canvas && m_canvas->image()){
        if( m_canvas->image()->currentLevelOfDetail() !=0 ){
            m_compressor->start();
            qDebug() << "Wrong LOD";
            return;
        }

        m_model->updateData(m_canvas);
        m_channelTable->resizeRowsToContents();
        m_channelTable->resizeColumnsToContents();
    }
}



