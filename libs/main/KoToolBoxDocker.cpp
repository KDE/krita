/*
 * Copyright (c) 2005-2009 Thomas Zander <zander@kde.org>
 * Copyright (c) 2009 Peter Simonsson <peter.simonsson@gmail.com>
 * Copyright (c) 2010 Cyrille Berger <cberger@cberger.net>
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

#include "KoToolBoxDocker_p.h"
#include "KoToolBox_p.h"
#include <KoDockWidgetTitleBar.h>
#include <KLocale>


KoToolBoxDocker::KoToolBoxDocker(KoToolBox *toolBox)
    : m_toolBox(toolBox)
{
    setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    setWidget(toolBox);
    setWindowTitle(i18n("Tools"));

    connect(this, SIGNAL(dockLocationChanged(Qt::DockWidgetArea)),
            this, SLOT(updateToolBoxOrientation(Qt::DockWidgetArea)));
    connect(this, SIGNAL(topLevelChanged(bool)),
            this, SLOT(updateFloating(bool)));
    KoDockWidgetTitleBar* titleBar = new KoDockWidgetTitleBar(this);
    titleBar->setTextVisibilityMode(KoDockWidgetTitleBar::TextCanBeInvisible);
    titleBar->setToolTip(windowTitle());
    setTitleBarWidget(titleBar);
}

void KoToolBoxDocker::setCanvas(KoCanvasBase *canvas)
{
    m_toolBox->setCanvas(canvas);
}

void KoToolBoxDocker::unsetCanvas()
{
    m_toolBox->unsetCanvas();
}

void KoToolBoxDocker::updateToolBoxOrientation(Qt::DockWidgetArea area)
{
    if (area == Qt::TopDockWidgetArea || area == Qt::BottomDockWidgetArea) {
        m_toolBox->setOrientation(Qt::Horizontal);
    } else {
        m_toolBox->setOrientation(Qt::Vertical);
    }
    m_toolBox->setFloating(area == Qt::NoDockWidgetArea);
}

void KoToolBoxDocker::updateFloating(bool v)
{
    m_toolBox->setFloating(v);
}
