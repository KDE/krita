/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2008 Martin Pfeiffer <hubipete@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
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
