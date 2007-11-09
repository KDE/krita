/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
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

#include "Panel.h"

#include <KoDockRegistry.h>
#include <KoCanvasBase.h>
#include <KoToolProxy.h>
#include <KoCanvasResourceProvider.h>
#include <KoText.h>

#include <KDebug>
#include <KLocale>
#include <KIcon>
#include <QAction>

Panel::Panel(QWidget *parent)
    :QDockWidget(i18n("Format"), parent),
    m_canvas(0)
{
    QWidget *mainWidget = new QWidget(this);
    widget.setupUi(mainWidget);
    setWidget(mainWidget);

    QButtonGroup *alignmentGroup = new QButtonGroup(this);
    alignmentGroup->addButton(widget.left);
    alignmentGroup->addButton(widget.center);
    alignmentGroup->addButton(widget.right);

    QButtonGroup *stylesGroup = new QButtonGroup(this);
    stylesGroup->addButton(widget.style1);
    stylesGroup->addButton(widget.style2);
    stylesGroup->addButton(widget.style3);

    loadIcons();
}

Panel::~Panel()
{
}

void Panel::setCanvas (KoCanvasBase *canvas)
{
    m_canvas = canvas;
    Q_ASSERT(m_canvas);
    connect(m_canvas->toolProxy(), SIGNAL(toolChanged(const QString&)), this, SLOT(toolChangeDetected(const QString&)));
    connect(m_canvas->resourceProvider(), SIGNAL(resourceChanged(int,const QVariant &)),
            this, SLOT(resourceChanged(int,const QVariant&)));
}

void Panel::toolChangeDetected(const QString &toolId) {
    if (toolId != "TextToolFactory_ID")
        return;
    // current tool is the text tool
    m_actions = m_canvas->toolProxy()->actions();
    widget.bold->setDefaultAction(m_actions["format_bold"]);
    widget.italic->setDefaultAction(m_actions["format_italic"]);
    widget.underline->setDefaultAction(m_actions["format_underline"]);
    widget.center->setDefaultAction(m_actions["format_aligncenter"]);
    if(QApplication::isRightToLeft()) {
        widget.left->setDefaultAction(m_actions["format_alignright"]);
        widget.right->setDefaultAction(m_actions["format_alignleft"]);
    }
    else {
        widget.left->setDefaultAction(m_actions["format_alignleft"]);
        widget.right->setDefaultAction(m_actions["format_alignright"]);
    }

    loadIcons();
}


void Panel::resourceChanged (int key, const QVariant &value) {
    if (key == KoText::CurrentTextDocument) {
        kDebug() << "new document...";
    }
}

void Panel::loadIcons() {
    KIcon bold("koffice_simple_format_bold_active");
    //bold.actualSize(QSize(50,50));
    widget.bold->setMinimumSize(QSize(48, 48));
    widget.bold->setIcon(bold);
    widget.italic->setIcon(bold);
}

#include "Panel.moc"
