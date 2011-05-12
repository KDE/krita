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

#include <KoResourceManager.h>
#include <KoCanvasBase.h>
#include "channelmodel.h"
#include <kis_view2.h>
#include <kis_canvas2.h>
#include <kis_layer.h>
#include <kis_layer_manager.h>

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
    m_canvas = canvas;
        
    KisView2* view = static_cast<KisCanvas2*>(m_canvas)->view();
    m_model->slotLayerActivated(view->activeLayer());
    connect(view->layerManager(), SIGNAL(sigLayerActivated(KisLayerSP)), m_model, SLOT(slotLayerActivated(KisLayerSP)));
}


#include "channeldocker_dock.moc"
