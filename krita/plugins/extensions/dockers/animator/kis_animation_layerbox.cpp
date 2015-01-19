/*
 *  Copyright (c) 2013 Somsubhra Bairi <somsubhra.bairi@gmail.com>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2.1 of the License, or (at your option)
 *  any later version.
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

#include "kis_animation_layerbox.h"

#include <QPainter>
#include <QModelIndex>
#include <QList>
#include <QLabel>

#include "kis_node_model.h"
#include "kis_node_manager.h"
#include "kis_timeline.h"
#include "kis_canvas2.h"
#include "KisViewManager.h"
#include "kis_action_manager.h"
#include "kis_action.h"
#include "kis_animation_layer_widget.h"
#include "kis_debug.h"
#include "kis_animation_doc.h"

KisAnimationLayerBox::KisAnimationLayerBox(KisTimelineWidget *parent)
{
    this->m_dock = parent;

    m_layerIndex = 1;
    m_nodeModel = new KisNodeModel(this);

    QLabel* lbl_Layers = new QLabel(this);
    lbl_Layers->setText(i18n("Animation Layers"));
    lbl_Layers->setGeometry(QRect(10, 0, 100, 20));

    QLabel* lblVisiblity = new QLabel(this);
    lblVisiblity->setText("V");
    lblVisiblity->setGeometry(QRect(120, 0, 20, 20));

    QLabel* lblLock = new QLabel(this);
    lblLock->setText("L");
    lblLock->setGeometry(QRect(140, 0, 20, 20));

    QLabel* lblOnionSkin = new QLabel(this);
    lblOnionSkin->setText("O");
    lblOnionSkin->setGeometry(QRect(160, 0, 20, 20));

    KisAnimationLayerWidget* firstLayer = new KisAnimationLayerWidget(this, m_layerIndex);
    m_layers << firstLayer;
    firstLayer->setGeometry(QRect(0, this->m_layers.length() * 20, 200, 20));
}

void KisAnimationLayerBox::addLayerUiUpdate()
{
    m_layerIndex++;

    this->setFixedHeight(this->height() + 20);

    KisAnimationLayerWidget* newLayer = new KisAnimationLayerWidget(this, m_layerIndex);
    m_layers << newLayer;
    int y;
    int noLayers = m_layers.length();

    for (int i = 0 ; i < noLayers - 1 ; i++) {
        y = m_layers.at(i)->geometry().y();
        m_layers.at(i)->setGeometry(QRect(0, y + 20, 200, 20));
    }

    newLayer->setGeometry(QRect(0, 20, 200, 20));
    newLayer->show();
}

void KisAnimationLayerBox::removeLayerUiUpdate(int layer)
{
    m_layers.at(layer)->hide();

    for (int i = 0 ; i < layer ; i++) {
        KisAnimationLayerWidget* l = m_layers.at(i);
        l->setGeometry(QRect(0, l->y() - 20, 200, 20));
    }

    m_layers.removeAt(layer);
}

void KisAnimationLayerBox::moveLayerDownUiUpdate(int layer)
{
    KisAnimationLayerWidget* l = m_layers.at(layer);
    KisAnimationLayerWidget* l_below = m_layers.at(layer - 1);

    l->setGeometry(QRect(0, l->y() + 20, 200, 20));
    l_below->setGeometry(QRect(0, l->y() - 20, 200, 20));

    m_layers.swap(layer, layer - 1);
}

void KisAnimationLayerBox::moveLayerUpUiUpdate(int layer)
{
    KisAnimationLayerWidget* l = m_layers.at(layer);
    KisAnimationLayerWidget* l_above = m_layers.at(layer + 1);

    l->setGeometry(QRect(0, l->y() - 20, 200, 20));
    l_above->setGeometry(QRect(0, l->y() + 20, 200, 20));

    m_layers.swap(layer, layer + 1);
}

void KisAnimationLayerBox::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);

    for (int i = 0; i < m_layers.length(); i++) {
        m_layers.at(i)->setFixedSize(200, 20);
    }
}

QList<KisAnimationLayerWidget*> KisAnimationLayerBox::getLayers()
{
    return this->m_layers;
}

int KisAnimationLayerBox::numberOfLayers()
{
    return m_layers.length();
}

void KisAnimationLayerBox::setOnionSkinState(int layer, bool state)
{
    m_onionSkinStates[layer] = state;
}

bool KisAnimationLayerBox::onionSkinstate(int layer)
{
    return m_onionSkinStates[layer];
}

QHash<int, bool> KisAnimationLayerBox::onionSkinStates()
{
    return m_onionSkinStates;
}

void KisAnimationLayerBox::setLockState(int layer, bool state)
{
    m_lockStates[layer] = state;
}

bool KisAnimationLayerBox::lockState(int layer)
{
    return m_lockStates[layer];
}

QHash<int, bool> KisAnimationLayerBox::lockStates()
{
    return m_lockStates;
}

void KisAnimationLayerBox::setVisibilityState(int layer, bool state)
{
    m_visibilityStates[layer] = state;
}

bool KisAnimationLayerBox::visibilityState(int layer)
{
    return m_visibilityStates[layer];
}

QHash<int, bool> KisAnimationLayerBox::visibilityStates()
{
    return m_visibilityStates;
}

int KisAnimationLayerBox::indexOf(KisAnimationLayerWidget *layer)
{
    return m_layers.indexOf(layer);
}

KisCanvas2* KisAnimationLayerBox::getCanvas()
{
    return m_dock->getCanvas();
}
