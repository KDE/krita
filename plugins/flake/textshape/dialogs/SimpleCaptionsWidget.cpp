/* This file is part of the KDE project
* Copyright (C) 2010 C. Boemann <cbo@boemann.dk>
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
#include "SimpleCaptionsWidget.h"
#include "TextTool.h"

#include <QAction>
#include <QDebug>

#include <QWidget>

SimpleCaptionsWidget::SimpleCaptionsWidget(QWidget *parent)
    : QWidget(parent)
    , m_blockSignals(false)
{
    widget.setupUi(this);
//    widget.splitCells->setDefaultAction(tool->action("split_tablecells"));

//    connect(widget.addRowAbove, SIGNAL(clicked(bool)), this, SIGNAL(doneWithFocus()));
}

void SimpleCaptionsWidget::setStyleManager(KoStyleManager *sm)
{
    m_styleManager = sm;
}
