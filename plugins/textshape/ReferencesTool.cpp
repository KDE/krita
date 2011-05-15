/* This file is part of the KDE project
 * Copyright (C) 2011 Casper Boemann <cbo@boemann.dk>
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

#include "ReferencesTool.h"
#include "TextShape.h"
#include "dialogs/SimpleTableOfContentsWidget.h"
#include "dialogs/SimpleCitationWidget.h"
#include "dialogs/SimpleFootEndNotesWidget.h"
#include "dialogs/SimpleCaptionsWidget.h"

#include <KoTextLayoutRootArea.h>
#include <KoCanvasBase.h>
#include <KoTextEditor.h>

#include <kdebug.h>

#include <KLocale>
#include <KAction>

ReferencesTool::ReferencesTool(KoCanvasBase* canvas): TextTool(canvas)
{
    createActions();
}

ReferencesTool::~ReferencesTool()
{
}

void ReferencesTool::createActions()
{
    KAction *action = new KAction(i18n("Table of Contents..."), this);
    addAction("insert_tableofcentents", action);
    action->setToolTip(i18n("Insert a Table of Contents into the document."));
    connect(action, SIGNAL(triggered()), this, SLOT(insertTableOfContents()));
}

void ReferencesTool::activate(ToolActivation toolActivation, const QSet<KoShape*> &shapes)
{
    TextTool::activate(toolActivation, shapes);
}

void ReferencesTool::deactivate()
{
    TextTool::deactivate();
    canvas()->canvasWidget()->setFocus();
}

QMap<QString, QWidget*> ReferencesTool::createOptionWidgets()
{
    QMap<QString, QWidget *> widgets;
    SimpleTableOfContentsWidget *stocw = new SimpleTableOfContentsWidget(this, 0);
    SimpleCitationWidget *scw = new SimpleCitationWidget(0);
    SimpleFootEndNotesWidget *sfenw = new SimpleFootEndNotesWidget(0);
    SimpleCaptionsWidget *scapw = new SimpleCaptionsWidget(0);

    // Connect to/with simple table of contents option widget
    connect(stocw, SIGNAL(doneWithFocus()), this, SLOT(returnFocusToCanvas()));

    // Connect to/with simple citation index option widget
    connect(scw, SIGNAL(doneWithFocus()), this, SLOT(returnFocusToCanvas()));

    // Connect to/with simple citation index option widget
    connect(sfenw, SIGNAL(doneWithFocus()), this, SLOT(returnFocusToCanvas()));

    widgets.insert(i18n("Table of Contents"), stocw);
    widgets.insert(i18n("Footnotes & Endnotes"), sfenw);
    //widgets.insert(i18n("Citations"), scw);
    //widgets.insert(i18n("Captions"), scapw);
    return widgets;
}

void ReferencesTool::insertTableOfContents()
{
    textEditor()->insertTableOfContents();
}

#include <ReferencesTool.moc>
