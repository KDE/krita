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

#include "kis_frame_box.h"
#include "kis_animation_layerbox.h"
#include "kis_layer_contents.h"
#include "kis_debug.h"
#include "kis_timeline_header.h"
#include "kis_animation_frame_widget.h"
#include "kis_animation_doc.h"
#include "KisViewManager.h"

#include <QPainter>

KisFrameBox::KisFrameBox(KisTimelineWidget *parent)
    : m_selectedFrame(0)
{
    this->m_dock = parent;
    //m_layers = this->m_dock->getLayerBox()->getLayers();

    m_timelineHeader = new KisTimelineHeader(this);
    m_timelineHeader->setGeometry(QRect(0, 0, 100000, 20));

    KisLayerContentsWidget* firstContents = new KisLayerContentsWidget(this);
    m_layerContents << firstContents;
    firstContents->setGeometry(QRect(0, m_layerContents.length() * 20, 100000, 20));

    this->setSelectedFrame(0, firstContents);
}

void KisFrameBox::addLayerUiUpdate()
{
    this->setFixedHeight(this->height() + 20);

    KisLayerContentsWidget* newContents = new KisLayerContentsWidget(this);
    m_layerContents << newContents;
    int y = 0;
    int noLayers = m_layerContents.length();

    for (int i = 0; i < noLayers - 1; i++) {
        y = m_layerContents.at(i)->geometry().y();
        m_layerContents.at(i)->setGeometry(QRect(0, y + 20, 100000, 20));
    }

    newContents->setGeometry(QRect(0, 20, 100000, 20));
    newContents->show();

    KisAnimationFrameWidget* currSelection = this->getSelectedFrame();
    currSelection->hide();

    KisAnimationFrameWidget* newSelection = new KisAnimationFrameWidget(newContents, KisAnimationFrameWidget::SELECTION, 10);
    newSelection->setGeometry(0, 0, 10, 20);

    // Don't call setSelectedFrame here because it will emit frameSelectionChanged
    // which will lead to crash in KisAnimationDoc
    this->m_selectedFrame = newSelection;
    newSelection->show();
}

void KisFrameBox::removeLayerUiUpdate(int layer)
{
    m_layerContents.at(layer)->hide();

    for (int i = 0 ; i < layer ; i++) {
        KisLayerContentsWidget* l = m_layerContents.at(i);
        l->setGeometry(QRect(0, l->y() - 20, 100000, 20));
    }

    m_layerContents.removeAt(layer);
}

void KisFrameBox::moveLayerDownUiUpdate(int layer)
{
    KisLayerContentsWidget* l = m_layerContents.at(layer);
    KisLayerContentsWidget* l_below = m_layerContents.at(layer - 1);

    l->setGeometry(QRect(0, l->y() + 20, width(), 20));
    l_below->setGeometry(QRect(0, l->y() - 20, width(), 20));

    m_layerContents.swap(layer, layer - 1);
}

void KisFrameBox::moveLayerUpUiUpdate(int layer)
{
    KisLayerContentsWidget* l = m_layerContents.at(layer);
    KisLayerContentsWidget* l_above = m_layerContents.at(layer + 1);

    l->setGeometry(QRect(0, l->y() - 20, width(), 20));
    l_above->setGeometry(QRect(0, l->y() + 20, width(), 20));

    m_layerContents.swap(layer, layer + 1);
}

void KisFrameBox::setSelectedFrame(int x, KisLayerContentsWidget* parent, int width)
{
    if (x < 0) {
        x = m_selectedFrame->geometry().x();
    }

    if (!parent) {
        parent = m_selectedFrame->getParent();
    }

    if (m_selectedFrame) {
        m_selectedFrame->hide();
        delete m_selectedFrame;
        m_selectedFrame = 0;
    }

    this->m_selectedFrame = new KisAnimationFrameWidget(parent, KisAnimationFrameWidget::SELECTION, width);
    this->m_selectedFrame->setGeometry(x, 0, width, 20);
    this->m_selectedFrame->show();

    int layerIndex = this->m_selectedFrame->getParent()->getLayerIndex();
    QRect globalPosition(this->m_selectedFrame->x(),
                         layerIndex * 20,
                         this->m_selectedFrame->width(),
                         this->m_selectedFrame->height());
    emit frameSelectionChanged(globalPosition);
}

KisAnimationFrameWidget* KisFrameBox::getSelectedFrame()
{
    return m_selectedFrame;
}

KisLayerContentsWidget* KisFrameBox::getFirstLayer()
{

    if (m_layerContents.isEmpty()) {
        return 0;
    }

    return m_layerContents.at(0);
}

QList<KisLayerContentsWidget*> KisFrameBox::getLayerContents()
{
    return this->m_layerContents;
}
