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
#include <KoTextSelectionHandler.h>
#include <KoText.h>

#include <KDebug>
#include <KLocale>
#include <KIcon>
#include <QAction>

Panel::Panel(QWidget *parent)
    :QDockWidget(i18n("Format"), parent),
    m_canvas(0),
    m_parent(0),
    m_handler(0)
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

    m_style1 = new QAction(i18n("Style Sans Serif"), this);
    m_style1->setToolTip( i18n("Set the current text to use a Sans Serif style") );
    m_style1->setCheckable( true );
    //connect (m_style1, SIGNAL(triggered()), 
}

void Panel::toolChangeDetected(const QString &toolId) {
    if (toolId != "TextToolFactory_ID")
        return;
    delete m_parent;
    m_parent = new QObject(this);
    // current tool is the text tool
    QHash<QString, QAction *> actions = m_canvas->toolProxy()->actions();
    applyAction(actions.value("format_bold"), widget.bold, "bold");
    applyAction(actions.value("format_italic"), widget.italic, "italic");
    applyAction(actions.value("format_underline"), widget.underline, "underline");
    applyAction(actions.value("format_aligncenter"), widget.center, "middle");
    if(QApplication::isRightToLeft()) {
        applyAction(actions.value("format_alignright"), widget.left, "left");
        applyAction(actions.value("format_alignleft"), widget.right, "right");
    }
    else {
        applyAction(actions.value("format_alignright"), widget.right, "left");
        applyAction(actions.value("format_alignleft"), widget.left, "right");
    }

    applyAction(actions.value("fontsizeup"), widget.sizeUp, "sizeup");
    applyAction(actions.value("fontsizedown"), widget.sizeDown, "sizedown");
    applyAction(m_style1, widget.style1, "sans");

    m_handler = 0;
    if (m_canvas)
        m_handler = qobject_cast<KoTextSelectionHandler*> (m_canvas->toolProxy()->selection());
}

void Panel::resourceChanged (int key, const QVariant &value) {
    if (key == KoText::CurrentTextDocument) {
        //kDebug() << "new document...";
    }
}

void Panel::applyAction(QAction *action, QToolButton *button, const QString &iconName) {
    Q_ASSERT(button);
    button->setEnabled(action);
    button->setMinimumSize(QSize(48, 48));
    button->setIconSize(QSize(48, 48));
    KIcon icon("koffice_simple_format_"+ iconName + (action ? "_active" : " _inactive"));
    if(action == 0) {
        button->setIcon(icon);
        return;
    }

    QAction *newAction = new QAction(m_parent);
    newAction->setToolTip(action->toolTip());
    newAction->setIcon(icon);
    newAction->setWhatsThis(action->whatsThis());
    newAction->setCheckable(action->isCheckable());
    button->setDefaultAction(newAction);
    connect(newAction, SIGNAL(triggered(bool)), action, SLOT(trigger()));
}

void Panel::style1ButtonClicked() {
    kDebug() << "style1ButtonClicked";
}

void Panel::style2ButtonClicked() {
}

void Panel::style3ButtonClicked() {
}


/* nice to haves
   Make the icon size 'configurable' using a context menu.
 */

#include "Panel.moc"
