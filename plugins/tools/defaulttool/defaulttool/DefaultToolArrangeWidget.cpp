/* This file is part of the KDE project
 * Copyright (C) 2007 Martin Pfeiffer <hubipete@gmx.net>
 * Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
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

#include "DefaultToolArrangeWidget.h"

#include <KoInteractionTool.h>
#include <QAction>

DefaultToolArrangeWidget::DefaultToolArrangeWidget(KoInteractionTool *tool, QWidget *parent) 
    : QWidget(parent)
{
    m_tool = tool;

    setupUi(this);

    bringToFront->setDefaultAction(m_tool->action("object_order_front"));
    raiseLevel->setDefaultAction(m_tool->action("object_order_raise"));
    lowerLevel->setDefaultAction(m_tool->action("object_order_lower"));
    sendBack->setDefaultAction(m_tool->action("object_order_back"));

    leftAlign->setDefaultAction(m_tool->action("object_align_horizontal_left"));
    hCenterAlign->setDefaultAction(m_tool->action("object_align_horizontal_center"));
    rightAlign->setDefaultAction(m_tool->action("object_align_horizontal_right"));
    topAlign->setDefaultAction(m_tool->action("object_align_vertical_top"));
    vCenterAlign->setDefaultAction(m_tool->action("object_align_vertical_center"));
    bottomAlign->setDefaultAction(m_tool->action("object_align_vertical_bottom"));

    group->setDefaultAction(m_tool->action("object_group"));
    ungroup->setDefaultAction(m_tool->action("object_ungroup"));
}
