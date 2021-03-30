/*
 * SPDX-FileCopyrightText: 2005-2009 Thomas Zander <zander@kde.org>
 * SPDX-FileCopyrightText: 2009 Peter Simonsson <peter.simonsson@gmail.com>
 * SPDX-FileCopyrightText: 2010 Cyrille Berger <cberger@cberger.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KoToolBoxDocker_p.h"
#include "KoToolBox_p.h"
#include "KoToolBoxScrollArea_p.h"
#include "KoDockRegistry.h"
#include <KoDockWidgetTitleBar.h>
#include <klocalizedstring.h>
#include <QLabel>
#include <QFontMetrics>
#include <QFrame>

KoToolBoxDocker::KoToolBoxDocker(KoToolBox *toolBox)
    : QDockWidget(i18n("Toolbox"))
    , m_toolBox(toolBox)
    , m_scrollArea(new KoToolBoxScrollArea(toolBox, this))
{
    setWidget(m_scrollArea);

    QLabel *w = new QLabel(" ", this);
    w->setFrameShape(QFrame::StyledPanel);
    w->setFrameShadow(QFrame::Raised);
    w->setFrameStyle(QFrame::Panel | QFrame::Raised);
    w->setMinimumWidth(16);
    w->setFixedHeight(QFontMetrics(KoDockRegistry::dockFont()).height());
    setTitleBarWidget(w);

    connect(this, SIGNAL(dockLocationChanged(Qt::DockWidgetArea)),
            this, SLOT(updateToolBoxOrientation(Qt::DockWidgetArea)));
    connect(this, SIGNAL(topLevelChanged(bool)),
            this, SLOT(updateFloating(bool)));
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
