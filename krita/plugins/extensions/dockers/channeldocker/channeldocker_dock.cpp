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

#include "channeldocker_dock.h"

#include <QGridLayout>
#include <QTableView>
#include <QHeaderView>
#include <klocale.h>

#include <KoCanvasBase.h>
#include "channelmodel.h"
#include <kis_view2.h>
#include <kis_canvas2.h>
#include <kis_layer.h>
#include <kis_node_manager.h>

ChannelDockerDock::ChannelDockerDock( ) : QDockWidget(i18n("Channels")), m_canvas(0)
{
    m_channelTable = new QTableView(this);
    m_model = new ChannelModel(this);
    m_channelTable->setModel(m_model);
    m_channelTable->setShowGrid(false);
    m_channelTable->verticalHeader()->setVisible(false);
    setWidget(m_channelTable);
}

void ChannelDockerDock::setCanvas(KoCanvasBase * canvas)
{
    if (m_canvas && m_canvas->view()) {
        m_canvas->view()->nodeManager()->disconnect(m_model);
    }
    m_canvas = dynamic_cast<KisCanvas2*>(canvas);
    if (m_canvas) {
        KisView2* view = m_canvas->view();
        m_model->slotLayerActivated(view->activeLayer());
        connect(view->nodeManager(), SIGNAL(sigLayerActivated(KisLayerSP)), m_model, SLOT(slotLayerActivated(KisLayerSP)));
    }
}


#include "channeldocker_dock.moc"
