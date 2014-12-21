/*
 *  Copyright (c) 2013 Somsubhra Bairi <somsubhra.bairi@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License, or(at you option)
 *  any later version.
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

#include "kis_animation_layer_widget.h"
#include "kis_canvas2.h"
#include "kis_animation_doc.h"
#include "KisPart.h"
#include "KisViewManager.h"

#include <KoIcon.h>

#include <QPainter>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QPushButton>
#include <QButtonGroup>

KisAnimationLayerWidget::KisAnimationLayerWidget(KisAnimationLayerBox *parent, int index)
{
    this->setParent(parent);
    this->m_layerBox = parent;

    m_inputLayerName = new QLineEdit(this);
    m_inputLayerName->setGeometry(QRect(10, 0, 100, 20));
    m_inputLayerName->hide();

    m_lblLayerName = new QLabel(this);
    m_lblLayerName->setText("Layer " + QString::number(index));
    m_lblLayerName->setGeometry(QRect(10, 0, 100, 20));

    m_visibilityToggle = new QPushButton(this);
    m_visibilityToggle->setIcon(koIcon("visible"));
    m_visibilityToggle->setGeometry(QRect(110, 0, 20, 20));
    connect(m_visibilityToggle, SIGNAL(clicked()), this, SLOT(visibilityToggleClicked()));

    m_lockToggle = new QPushButton(this);
    m_lockToggle->setIcon(koIcon("unlocked"));
    m_lockToggle->setGeometry(QRect(130, 0, 20, 20));
    connect(m_lockToggle, SIGNAL(clicked()), this, SLOT(lockToggleClicked()));

    m_onionSkinToggle = new QPushButton(this);
    m_onionSkinToggle->setIcon(koIcon("onionA"));
    m_onionSkinToggle->setGeometry(QRect(150, 0, 20, 20));
    connect(m_onionSkinToggle, SIGNAL(clicked()), this, SLOT(onionSkinToggleClicked()));

    this->setFixedSize(200, 20);

    connect(m_inputLayerName, SIGNAL(returnPressed()), this, SLOT(onLayerNameEdited()));
}

void KisAnimationLayerWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setPen(Qt::darkGray);
    painter.setBrush(Qt::lightGray);
    painter.drawRect(QRect(0, 0, 200, height()));
}

void KisAnimationLayerWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    Q_UNUSED(event);

    m_lblLayerName->hide();
    m_inputLayerName->setText(m_lblLayerName->text());
    m_inputLayerName->show();
}

void KisAnimationLayerWidget::onLayerNameEdited()
{
    m_inputLayerName->hide();
    m_lblLayerName->setText(m_inputLayerName->text());
    m_lblLayerName->show();
}

void KisAnimationLayerWidget::onionSkinToggleClicked()
{
    int layer = m_layerBox->indexOf(this);
    bool onionSkinState = m_layerBox->onionSkinstate(layer);

    onionSkinState = !onionSkinState;
    m_layerBox->setOnionSkinState(layer, onionSkinState);

    if (onionSkinState) {
        m_onionSkinToggle->setIcon(koIcon("onionB"));
    } else {
        m_onionSkinToggle->setIcon(koIcon("onionA"));
    }

    // Make onion skin changes in the canvas
    dynamic_cast<KisAnimationDoc*>(m_layerBox->getCanvas()->viewManager()->document())->onionSkinStateToggled(m_layerBox->onionSkinStates());
}

void KisAnimationLayerWidget::lockToggleClicked()
{
    int layer = m_layerBox->indexOf(this);
    bool lockState = m_layerBox->lockState(layer);

    lockState = !lockState;
    m_layerBox->setLockState(layer, lockState);

    if (lockState) {
        m_lockToggle->setIcon(koIcon("locked"));
    } else {
        m_lockToggle->setIcon(koIcon("unlocked"));
    }

    // Make lock state changes in the canvas
    dynamic_cast<KisAnimationDoc*>(m_layerBox->getCanvas()->viewManager()->document())->lockStateToggled(m_layerBox->lockStates());
}

void KisAnimationLayerWidget::visibilityToggleClicked()
{
    int layer = m_layerBox->indexOf(this);
    bool visibilityState = m_layerBox->visibilityState(layer);

    visibilityState = !visibilityState;
    m_layerBox->setVisibilityState(layer, visibilityState);

    if (visibilityState) {
        m_visibilityToggle->setIcon(koIcon("novisible"));
    } else {
        m_visibilityToggle->setIcon(koIcon("visible"));
    }

    // Make the visibilty state changes in the canvas
    dynamic_cast<KisAnimationDoc*>(m_layerBox->getCanvas()->viewManager()->document())->visibilityStateToggled(m_layerBox->visibilityStates());
}
