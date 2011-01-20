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
}

#include <ConnectionPointWidget.moc>
