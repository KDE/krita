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
#include "ActionHelper.h"

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

    m_style1 = new QAction(i18n("Style Sans Serif"), this);
    m_style1->setToolTip( i18n("Set the current text to use a Sans Serif style") );
    m_style1->setCheckable( true );
    connect (m_style1, SIGNAL(triggered()), this, SLOT(style1ButtonClicked()));

    m_style2 = new QAction(i18n("Style Serif"), this);
    m_style2->setToolTip( i18n("Set the current text to use a Serif style") );
    m_style2->setCheckable( true );
    connect (m_style2, SIGNAL(triggered()), this, SLOT(style2ButtonClicked()));

    m_style3 = new QAction(i18n("Style Script"), this);
    m_style3->setToolTip( i18n("Set the current text to use a Script style") );
    m_style3->setCheckable( true );
    connect (m_style3, SIGNAL(triggered()), this, SLOT(style3ButtonClicked()));

    // TODO enable 'font color'
    widget.color->setVisible(false);
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
    applyAction(m_style2, widget.style2, "serif");
    applyAction(m_style3, widget.style3, "script");

    m_handler = 0;
    if (m_canvas)
        m_handler = qobject_cast<KoTextSelectionHandler*> (m_canvas->toolProxy()->selection());
    m_style1->setEnabled(m_handler);
    m_style2->setEnabled(m_handler);
    m_style3->setEnabled(m_handler);
}

void Panel::resourceChanged (int key, const QVariant &value) {
    if (key == KoText::CurrentTextDocument) {
        //kDebug() << "new document...";
        if (value.isNull() && m_parent) {
            delete m_parent;
            m_parent = 0;
            // load 'disabled' widgets.
            applyAction(0, widget.bold, "bold");
            applyAction(0, widget.italic, "italic");
            applyAction(0, widget.underline, "underline");
            applyAction(0, widget.center, "middle");
            if(QApplication::isRightToLeft()) {
                applyAction(0, widget.left, "left");
                applyAction(0, widget.right, "right");
            }
            else {
                applyAction(0, widget.right, "left");
                applyAction(0, widget.left, "right");
            }

            applyAction(0, widget.sizeUp, "sizeup");
            applyAction(0, widget.sizeDown, "sizedown");
            applyAction(0, widget.style1, "sans");
            applyAction(0, widget.style2, "serif");
            applyAction(0, widget.style3, "script");
        }
    }
}

void Panel::applyAction(QAction *action, QToolButton *button, const QString &iconName) {
    Q_ASSERT(button);
    button->setEnabled(action);
    button->setIconSize(QSize(42, 42));
    KIcon icon("koffice_simple_format_"+ iconName + (action ? "_active" : "_inactive"));
    if(action == 0) {
        button->setIcon(icon);
        button->setEnabled(false);
        return;
    }

    QAction *newAction = new QAction(m_parent);
    newAction->setToolTip(action->toolTip());
    newAction->setIcon(icon);
    newAction->setWhatsThis(action->whatsThis());
    newAction->setCheckable(action->isCheckable());
    button->setDefaultAction(newAction);
    new ActionHelper(m_parent, action, newAction);
}

void Panel::style1ButtonClicked() {
    if (m_handler == 0) return;
    m_handler->setFontFamily("Sans Serif");
}

void Panel::style2ButtonClicked() {
    if (m_handler == 0) return;
    m_handler->setFontFamily("Serif");
}

void Panel::style3ButtonClicked() {
    if (m_handler == 0) return;
    m_handler->setFontFamily("Script");
}


/* nice to haves
   Make the icon size 'configurable' using a context menu.
 */

#include "Panel.moc"
