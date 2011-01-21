/* This file is part of the KDE project
 * 
 * Copyright (C) 2011 Jan Hambrecht <jaham@gmx.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "ConnectionPointWidget.h"

ConnectionPointWidget::ConnectionPointWidget(QWidget * parent)
    :QWidget(parent)
{
    widget.setupUi(this);
    widget.alignLeft->setIcon(KIcon("align-horizontal-left"));
    widget.alignCenterH->setIcon(KIcon("align-horizontal-center"));
    widget.alignRight->setIcon(KIcon("align-horizontal-right"));
    widget.alignTop->setIcon(KIcon("align-vertical-top"));
    widget.alignCenterV->setIcon(KIcon("align-vertical-center"));
    widget.alignBottom->setIcon(KIcon("align-vertical-bottom"));

    widget.alignLeft->setCheckable(true);
    widget.alignCenterH->setCheckable(true);
    widget.alignRight->setCheckable(true);
    widget.alignTop->setCheckable(true);
    widget.alignCenterV->setCheckable(true);
    widget.alignBottom->setCheckable(true);
    widget.alignPercent->setCheckable(true);

    m_horzGroup = new QButtonGroup(this);
    m_horzGroup->addButton(widget.alignLeft);
    m_horzGroup->addButton(widget.alignCenterH);
    m_horzGroup->addButton(widget.alignRight);
    m_horzGroup->setExclusive(true);

    m_vertGroup = new QButtonGroup(this);
    m_vertGroup->addButton(widget.alignTop);
    m_vertGroup->addButton(widget.alignCenterV);
    m_vertGroup->addButton(widget.alignBottom);
    m_vertGroup->setExclusive(true);

    connect(widget.alignPercent, SIGNAL(clicked(bool)), this, SLOT(alignPercentClicked(bool)));
    connect(m_horzGroup, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(alignClicked()));
    connect(m_vertGroup, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(alignClicked()));
}

void ConnectionPointWidget::alignPercentClicked(bool checked)
{
    m_horzGroup->setExclusive(!checked);
    m_vertGroup->setExclusive(!checked);
    if(checked) {
        if (m_horzGroup->checkedButton())
            m_horzGroup->checkedButton()->setChecked(false);
        if (m_vertGroup->checkedButton())
            m_vertGroup->checkedButton()->setChecked(false);
    } else {
        widget.alignCenterH->setChecked(true);
        widget.alignCenterV->setChecked(true);
    }
}

void ConnectionPointWidget::alignClicked()
{
    m_horzGroup->setExclusive(true);
    m_vertGroup->setExclusive(true);
    widget.alignPercent->setChecked(false);
    if (!m_horzGroup->checkedButton())
        widget.alignCenterH->setChecked(true);
    if (!m_vertGroup->checkedButton())
        widget.alignCenterV->setChecked(true);
}

#include <ConnectionPointWidget.moc>
