/* This file is part of the KDE project
 * Copyright (C) 2010 Casper Boemann <cbo@boemann.dk>
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
#include "SimpleTableOfContentsWidget.h"
#include "ReferencesTool.h"
#include "TableOfContentsConfigure.h"

#include <KAction>
#include <KDebug>

#include <QWidget>
#include <QMenu>

SimpleTableOfContentsWidget::SimpleTableOfContentsWidget(ReferencesTool *tool, QWidget *parent)
        : QWidget(parent),
        m_blockSignals(false)
{
    widget.setupUi(this);
    widget.addToC->setDefaultAction(tool->action("insert_tableofcontents"));
    widget.configureToC->setDefaultAction(tool->action("format_tableofcontents"));

    connect(widget.addToC, SIGNAL(clicked(bool)), this, SIGNAL(doneWithFocus()));
    connect(widget.configureToC, SIGNAL(clicked(bool)), this, SIGNAL(showConfgureOptions()));
}

void SimpleTableOfContentsWidget::setStyleManager(KoStyleManager *sm)
{
    m_styleManager = sm;
}

void SimpleTableOfContentsWidget::setToCConfigureMenu(QMenu *tocMenu)
{
    if (widget.configureToC->menu()) {
        widget.configureToC->menu()->disconnect();
    }

    widget.configureToC->setMenu(tocMenu);
}

QMenu *SimpleTableOfContentsWidget::ToCConfigureMenu()
{
    return widget.configureToC->menu();
}

void SimpleTableOfContentsWidget::showMenu()
{
    widget.configureToC->showMenu();
}

#include <SimpleTableOfContentsWidget.moc>
