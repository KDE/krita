/*
 *  Copyright (c) 2013 Somsubhra Bairi <somsubhra.bairi@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License, or(at you option)
 *  any later version..
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

#include "kis_animation_layerbox.h"
#include <QPainter>
#include <QModelIndex>
#include <QList>
#include <kis_node_model.h>
#include <kis_node_manager.h>
#include "kis_timeline.h"
#include <kis_canvas2.h>
#include <kis_view2.h>
#include <kis_action_manager.h>
#include <kis_action.h>
#include "kis_animation_layer.h"
#include <kis_debug.h>
#include <QLabel>

KisAnimationLayerBox::KisAnimationLayerBox(KisTimeline *parent)
{
    this->m_dock = parent;
    m_nodeModel = new KisNodeModel(this);

    QLabel *lbl_Layers = new QLabel(this);
    lbl_Layers->setText("Animation Layers");
    lbl_Layers->setGeometry(QRect(0, 0, width(), 20));

    KisAnimationLayer* firstLayer = new KisAnimationLayer(this);
    m_layers << firstLayer;
    firstLayer->setGeometry(QRect(0,this->m_layers.length()*20,width(),20));
}

inline void KisAnimationLayerBox::connectActionToButton(QAction *button, const QString &id){
    Q_ASSERT(m_dock->getCanvas());

    KisAction *action = m_dock->getCanvas()->view()->actionManager()->actionByName(id);
    connect(button, SIGNAL(triggered()), action, SLOT(trigger()));
    connect(action, SIGNAL(sigEnableSlaves(bool)), button, SLOT(setEnabled(bool)));
}

void KisAnimationLayerBox::onCanvasReady(){
    this->connectActionToButton(m_dock->m_addPaintLayerAction, "add_new_paint_layer");
    connect(m_dock->m_addPaintLayerAction, SIGNAL(triggered()), this, SLOT(updateUI()));

    this->connectActionToButton(m_dock->m_addVectorLayerAction, "add_new_shape_layer");
    connect(m_dock->m_addVectorLayerAction, SIGNAL(triggered()), this, SLOT(updateUI()));

    m_nodeManager = m_dock->getCanvas()->view()->nodeManager();
}

void KisAnimationLayerBox::updateUI(){
    KisAnimationLayer* newLayer = new KisAnimationLayer(this);
    m_layers << newLayer;
    int y;
    int noLayers = m_layers.length();
    for(int i = 0; i < noLayers-1; i++){
        y = m_layers.at(i)->geometry().y();
        m_layers.at(i)->setGeometry(QRect(0, y+20, width(), 20));
    }
    newLayer->setGeometry(QRect(0, 20, width(), 20));
    newLayer->show();
}

void KisAnimationLayerBox::resizeEvent(QResizeEvent *event){
    for(int i = 0; i < m_layers.length(); i++){
        m_layers.at(i)->setFixedSize(width(), 20);
    }
}

QList<KisAnimationLayer*> KisAnimationLayerBox::getLayers(){
    return this->m_layers;
}
