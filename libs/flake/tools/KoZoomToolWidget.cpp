/* This file is part of the KDE project
 * Copyright (C) 2008 Martin Pfeiffer <hubipete@gmx.net>
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

#include "KoZoomToolWidget.h"
#include <QPainter>
#include <QMouseEvent>
#include <KoIcon.h>
#include "KoZoomTool.h"

KoZoomToolWidget::KoZoomToolWidget(KoZoomTool* tool, QWidget* parent)
    : QWidget(parent)
    , m_tool(tool)
{
    setupUi(this);

    zoomInButton->setIcon(koIcon("zoom-in"));
    zoomOutButton->setIcon(koIcon("zoom-out"));

    connect(zoomInButton, SIGNAL(toggled(bool)), this, SLOT(changeZoomMode()));
    connect(zoomOutButton, SIGNAL(toggled(bool)), this, SLOT(changeZoomMode()));

    zoomInButton->click();
}

KoZoomToolWidget::~KoZoomToolWidget()
{
}

void KoZoomToolWidget::changeZoomMode()
{
    m_tool->setZoomInMode(zoomInButton->isChecked());
}
