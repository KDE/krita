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
#include "KoToolBoxScrollArea_p.h"
#include <KoDockWidgetTitleBar.h>
#include <klocalizedstring.h>


KoToolBoxDocker::KoToolBoxDocker(KoToolBox *toolBox)
    : QDockWidget(i18n("Toolbox"))
    , m_toolBox(toolBox)
    , m_scrollArea(new KoToolBoxScrollArea(toolBox, this))
{
    setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    setWidget(m_scrollArea);

    connect(this, SIGNAL(dockLocationChanged(Qt::DockWidgetArea)),
            this, SLOT(updateToolBoxOrientation(Qt::DockWidgetArea)));
    connect(this, SIGNAL(topLevelChanged(bool)),
            this, SLOT(updateFloating(bool)));
    KoDockWidgetTitleBar* titleBar = new KoDockWidgetTitleBar(this);
    titleBar->setTextVisibilityMode(KoDockWidgetTitleBar::TextCanBeInvisible);
    titleBar->setToolTip(i18n("Tools"));
    setTitleBarWidget(titleBar);
}

void KoToolBoxDocker::setCanvas(KoCanvasBase *canvas)
{
    Q_UNUSED(canvas);
}

void KoToolBoxDocker::unsetCanvas()
{
}

void KoToolBoxDocker::resizeEvent(QResizeEvent *event)
{
    QDockWidget::resizeEvent(event);
    if (isFloating()) {
        if (m_scrollArea->width() > m_scrollArea->height()) {
            m_scrollArea->setOrientation(Qt::Horizontal);
        } else {
            m_scrollArea->setOrientation(Qt::Vertical);
        }
    }
}

void KoToolBoxDocker::updateToolBoxOrientation(Qt::DockWidgetArea area)
{
    if (area == Qt::TopDockWidgetArea || area == Qt::BottomDockWidgetArea) {
        m_scrollArea->setOrientation(Qt::Horizontal);
    } else {
        m_scrollArea->setOrientation(Qt::Vertical);
    }
}

void KoToolBoxDocker::updateFloating(bool v)
{
    m_toolBox->setFloating(v);
}
