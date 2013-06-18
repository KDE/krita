/*
 *  Copyright (c) 2013 Somsubhra Bairi <somsubhra.bairi@gmail.com>
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

#include "kis_frame_box.h"
#include <QPainter>
#include "kis_animation_layerbox.h"
#include "kis_layer_contents.h"
#include <kis_debug.h>

KisFrameBox::KisFrameBox(KisTimeline *parent)
{
    this->m_dock = parent;
    m_layers = this->m_dock->getLayerBox()->getLayers();

    KisLayerContents* firstContents = new KisLayerContents(this);
    m_layerContents << firstContents;
    firstContents->setGeometry(QRect(0, m_layerContents.length()*20,10000, 20));
}

void KisFrameBox::onCanvasReady(){
    connect(m_dock->m_addPaintLayerAction, SIGNAL(triggered()), this, SLOT(updateUI()));
    connect(m_dock->m_addVectorLayerAction, SIGNAL(triggered()), this, SLOT(updateUI()));
}

void KisFrameBox::updateUI(){
    KisLayerContents* newContents = new KisLayerContents(this);
    m_layerContents << newContents;
    int y;
    int noLayers = m_layerContents.length();

    for(int i = 0; i < noLayers - 1; i++){
        y = m_layerContents.at(i)->geometry().y();
        m_layerContents.at(i)->setGeometry(QRect(0, y+20, 10000, 20));
    }
    newContents->setGeometry(QRect(0, 20, 10000, 20));
    newContents->show();
}

void KisFrameBox::setSelectedFrame(KisAnimationFrame *selectedFrame){
    this->m_selectedFrame = selectedFrame;
}

KisAnimationFrame* KisFrameBox::getSelectedFrame(){
    return m_selectedFrame;
}
