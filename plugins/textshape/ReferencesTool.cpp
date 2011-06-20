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

#include <KoInlineNote.h>
#include <KoTextDocumentLayout.h>
#include <KoInlineTextObjectManager.h>

#include <kdebug.h>

#include <KLocale>
#include <KAction>
#include <QTextDocument>

ReferencesTool::ReferencesTool(KoCanvasBase* canvas): TextTool(canvas)
{
    createActions();
}

ReferencesTool::~ReferencesTool()
{
}

void ReferencesTool::createActions()
{
    KAction *action;
    KAction *action1;
    KAction *action2;

    action = new KAction(i18n("Add ToC"), this);
    addAction("insert_tableofcentents", action);
    action->setToolTip(i18n("Insert a Table of Contents into the document."));
    connect(action, SIGNAL(triggered()), this, SLOT(insertTableOfContents()));
    action1 = new KAction(i18n("Add Footnote"),this);
    addAction("insert_footnote",action1);
    action1->setToolTip(i18n("Insert a Foot Note into the document."));
    connect(action1, SIGNAL(triggered()), this, SLOT(insertFootNote()));
    action2 = new KAction(i18n("Add Endnote"),this);
    addAction("insert_endnote",action2);
    action2->setToolTip(i18n("Insert an End Note into the document."));
    connect(action2, SIGNAL(triggered()), this, SLOT(insertEndNote()));
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

QList<QWidget*> ReferencesTool::createOptionWidgets()
{
    QList<QWidget *> widgets;
    stocw = new SimpleTableOfContentsWidget(this, 0);
    //SimpleCitationWidget *scw = new SimpleCitationWidget(0);
    sfenw = new SimpleFootEndNotesWidget(this,0);
    //SimpleCaptionsWidget *scapw = new SimpleCaptionsWidget(0);

    // Connect to/with simple table of contents option widget
    connect(stocw, SIGNAL(doneWithFocus()), this, SLOT(returnFocusToCanvas()));

    // Connect to/with simple citation index option widget
    //connect(scw, SIGNAL(doneWithFocus()), this, SLOT(returnFocusToCanvas()));

    // Connect to/with simple citation index option widget
    connect(sfenw, SIGNAL(doneWithFocus()), this, SLOT(returnFocusToCanvas()));

    stocw->setWindowTitle(i18n("Table of Contents"));
    widgets.append(stocw);
    sfenw->setWindowTitle(i18n("Footnotes & Endnotes"));
    widgets.append(sfenw);
    //widgets.insert(i18n("Citations"), scw);
    //widgets.insert(i18n("Captions"), scapw);
    return widgets;
}

void ReferencesTool::insertTableOfContents()
{
    textEditor()->insertTableOfContents();
}

void ReferencesTool::insertFootNote()
{
    note = textEditor()->insertFootNote();
    note->setAutoNumbering(sfenw->widget.autoNumbering->isChecked());
    if(note->autoNumbering())
    {
        note->setLabel(QString().number(note->manager()->footNotes().count()));
    }
    else
    {
        note->setLabel(sfenw->widget.characterEdit->text());
    }

    QTextCursor cursor = note->textCursor();
    QTextCharFormat *fmat = new QTextCharFormat();
    fmat->setVerticalAlignment(QTextCharFormat::AlignSuperScript);
    if(note->autoNumbering())
    {
        cursor.insertText(note->label(),*fmat);
    }
    else
    {
        cursor.insertText(sfenw->widget.characterEdit->text(),*fmat);
    }

    fmat->setVerticalAlignment(QTextCharFormat::AlignNormal);
    cursor.insertText(" ",*fmat);

    note->manager()->reNumbering(note->textFrame()->document()->begin());
}

void ReferencesTool::insertEndNote()
{
    textEditor()->insertEndNote();
}

#include <ReferencesTool.moc>
