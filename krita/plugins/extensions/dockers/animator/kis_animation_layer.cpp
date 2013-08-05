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

#include "kis_animation_layer.h"
#include <QPainter>
#include <QLineEdit>
#include <QHBoxLayout>

KisAnimationLayer::KisAnimationLayer(KisAnimationLayerBox *parent)
{
    this->setParent(parent);
    this->m_layerBox = parent;

    m_inputLayerName = new QLineEdit();
    m_inputLayerName->setFixedHeight(20);

    m_lblLayerName = new QLabel(this);
    m_lblLayerName->setText("Layer");
    m_lblLayerName->setFixedHeight(20);

    lay = new QHBoxLayout();
    lay->addWidget(m_lblLayerName);
    this->setLayout(lay);

    this->setFixedSize(m_layerBox->width(), 20);

    connect(m_inputLayerName, SIGNAL(returnPressed()), this, SLOT(onLayerNameEdited()));
}

void KisAnimationLayer::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setPen(Qt::darkGray);
    painter.setBrush(Qt::lightGray);
    painter.drawRect(QRect(0,0,width(), height()));
}

void KisAnimationLayer::mouseDoubleClickEvent(QMouseEvent *event)
{
    lay->removeWidget(m_lblLayerName);
    m_lblLayerName->hide();
    m_inputLayerName->setText(m_lblLayerName->text());
    lay->addWidget(m_inputLayerName);
    m_inputLayerName->show();
}

void KisAnimationLayer::onLayerNameEdited()
{
    lay->removeWidget(m_inputLayerName);
    m_inputLayerName->hide();
    m_lblLayerName->setText(m_inputLayerName->text());
    lay->addWidget(m_lblLayerName);
    m_lblLayerName->show();
}
