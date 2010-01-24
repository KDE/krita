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
#include <KoResourceManager.h>
#include <KoTextEditor.h>
#include <KoText.h>

#include <KDebug>
#include <KLocale>
#include <KIcon>
#include <KAction>

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

    m_style1 = new KAction(i18n("Style Sans Serif"), this);
    m_style1->setToolTip( i18n("Set the current text to use a Sans Serif style") );
    m_style1->setCheckable( true );
    connect (m_style1, SIGNAL(triggered()), this, SLOT(style1ButtonClicked()));

    m_style2 = new KAction(i18n("Style Serif"), this);
    m_style2->setToolTip( i18n("Set the current text to use a Serif style") );
    m_style2->setCheckable( true );
    connect (m_style2, SIGNAL(triggered()), this, SLOT(style2ButtonClicked()));

    m_style3 = new KAction(i18n("Style Script"), this);
    m_style3->setToolTip( i18n("Set the current text to use a Script style") );
    m_style3->setCheckable( true );
    connect (m_style3, SIGNAL(triggered()), this, SLOT(style3ButtonClicked()));

    setInitialButtonIcon(widget.bold, "bold");
    setInitialButtonIcon(widget.italic, "italic");
    setInitialButtonIcon(widget.underline, "underline");
    setInitialButtonIcon(widget.center, "middle");
    setInitialButtonIcon(widget.sizeUp, "sizeup");
    setInitialButtonIcon(widget.sizeDown, "sizedown");
    setInitialButtonIcon(widget.style1, "sans");
    setInitialButtonIcon(widget.style2, "serif");
    setInitialButtonIcon(widget.style3, "script");
    if (QApplication::isRightToLeft()) {
        setInitialButtonIcon(widget.right, "left");
        setInitialButtonIcon(widget.left, "right");
    } else {
        setInitialButtonIcon(widget.left, "left");
        setInitialButtonIcon(widget.right, "right");
    }

    // TODO enable 'font color'
    widget.color->setVisible(false);
}

Panel::~Panel()
{
}

void Panel::setInitialButtonIcon(QToolButton *button, const QString &name) const
{
    button->setIconSize(QSize(42, 42));
    button->setEnabled(false);
    button->setIcon(KIcon("koffice_simple_format_"+ name +"_inactive"));
}

void Panel::setCanvas (KoCanvasBase *canvas)
{
    m_canvas = canvas;
    Q_ASSERT(m_canvas);
    connect(m_canvas->toolProxy(), SIGNAL(toolChanged(const QString&)), this, SLOT(toolChangeDetected(const QString&)));
    connect(m_canvas->resourceManager(), SIGNAL(resourceChanged(int,const QVariant &)),
            this, SLOT(resourceChanged(int,const QVariant&)));
}

void Panel::toolChangeDetected(const QString &toolId)
{
    if (toolId != "TextToolFactory_ID")
        return;
    delete m_parent;
    m_parent = new QObject(this);
    // current tool is the text tool
    QHash<QString, KAction *> actions = m_canvas->toolProxy()->actions();
    applyAction(actions.value("format_bold"), widget.bold, "bold", false);
    applyAction(actions.value("format_italic"), widget.italic, "italic", false);
    applyAction(actions.value("format_underline"), widget.underline, "underline", false);
    applyAction(actions.value("format_aligncenter"), widget.center, "middle", true);
    if (QApplication::isRightToLeft()) {
        applyAction(actions.value("format_alignright"), widget.left, "right", true);
        applyAction(actions.value("format_alignleft"), widget.right, "left", true);
    } else {
        applyAction(actions.value("format_alignright"), widget.right, "right", true);
        applyAction(actions.value("format_alignleft"), widget.left, "left", true);
    }

    applyAction(actions.value("fontsizeup"), widget.sizeUp, "sizeup", false);
    applyAction(actions.value("fontsizedown"), widget.sizeDown, "sizedown", false);
    applyAction(m_style1, widget.style1, "sans", true);
    applyAction(m_style2, widget.style2, "serif", true);
    applyAction(m_style3, widget.style3, "script", true);

    m_handler = 0;
    if (m_canvas)
        m_handler = qobject_cast<KoTextEditor*> (m_canvas->toolProxy()->selection());
    m_style1->setEnabled(m_handler);
    m_style2->setEnabled(m_handler);
    m_style3->setEnabled(m_handler);
}

void Panel::resourceChanged (int key, const QVariant &value)
{
    if (key == KoText::CurrentTextDocument) {
        if (value.isNull() && m_parent) {
            delete m_parent;
            m_parent = 0;
            // load 'disabled' widgets.
            applyAction(0, widget.bold, "bold", false);
            applyAction(0, widget.italic, "italic", false);
            applyAction(0, widget.underline, "underline", false);
            applyAction(0, widget.center, "middle", true);
            if (QApplication::isRightToLeft()) {
                applyAction(0, widget.left, "left", true);
                applyAction(0, widget.right, "right", true);
            }
            else {
                applyAction(0, widget.right, "left", true);
                applyAction(0, widget.left, "right", true);
            }

            applyAction(0, widget.sizeUp, "sizeup", false);
            applyAction(0, widget.sizeDown, "sizedown", false);
            applyAction(0, widget.style1, "sans", true);
            applyAction(0, widget.style2, "serif", true);
            applyAction(0, widget.style3, "script", true);
        }
    }
}

void Panel::applyAction(KAction *action, QToolButton *button, const QString &iconName, bool partOfGroup)
{
    Q_ASSERT(button);
    button->setEnabled(action);
    button->setIconSize(QSize(42, 42));
    KIcon icon("koffice_simple_format_"+ iconName + (action ? "_active" : "_inactive"));
    if (action == 0) {
        button->setIcon(icon);
        button->setEnabled(false);
        return;
    }

    KAction *newAction = new KAction(m_parent);
    newAction->setToolTip(action->toolTip());
    newAction->setIcon(icon);
    newAction->setWhatsThis(action->whatsThis());
    newAction->setCheckable(action->isCheckable());
    button->setDefaultAction(newAction);
    new ActionHelper(m_parent, action, newAction, partOfGroup);
}

void Panel::style1ButtonClicked()
{
    if (m_handler == 0) return;
    m_handler->setFontFamily("Sans Serif");
}

void Panel::style2ButtonClicked()
{
    if (m_handler == 0) return;
    m_handler->setFontFamily("Serif");
}

void Panel::style3ButtonClicked()
{
    if (m_handler == 0) return;
    m_handler->setFontFamily("Script");
}


/* nice to haves
   Make the icon size 'configurable' using a context menu.
 */

#include <Panel.moc>
