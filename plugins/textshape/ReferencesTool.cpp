/* This file is part of the KDE project
 * Copyright (C) 2011 C. Boemann <cbo@boemann.dk>
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
#include "dialogs/SimpleCitationBibliographyWidget.h"
#include "dialogs/SimpleFootEndNotesWidget.h"
#include "dialogs/SimpleCaptionsWidget.h"
#include "dialogs/TableOfContentsConfigure.h"
#include "dialogs/NotesConfigurationDialog.h"
#include "dialogs/CitationInsertionDialog.h"
#include "dialogs/InsertBibliographyDialog.h"
#include "dialogs/BibliographyConfigureDialog.h"

#include <KoTextLayoutRootArea.h>
#include <KoCanvasBase.h>
#include <KoTextEditor.h>
#include <KoParagraphStyle.h>
#include <KoTableOfContentsGeneratorInfo.h>
#include <KoBookmark.h>
#include <KoInlineNote.h>
#include <KoTextDocumentLayout.h>
#include <KoIcon.h>

#include <kdebug.h>

#include <KLocale>
#include <KAction>
#include <QTextDocument>
#include <QLineEdit>
#include <QBoxLayout>
#include <QMenu>

LabeledNoteWidget::LabeledNoteWidget(KAction *action)
    : QWidget()
    , m_action(action)
{
    setMouseTracking(true);
    QHBoxLayout *layout = new QHBoxLayout();
    QLabel *l = new QLabel(i18n("Insert with label:"));
    l->setIndent(l->style()->pixelMetric(QStyle::PM_SmallIconSize)
        + l->style()->pixelMetric(QStyle::PM_MenuPanelWidth)
        + 4);
    layout->addWidget(l);
    m_lineEdit = new QLineEdit();
    layout->addWidget(m_lineEdit);
    layout->setMargin(0);
    setLayout(layout);

    connect(m_lineEdit, SIGNAL(returnPressed()), this, SLOT(returnPressed()));
}

void LabeledNoteWidget::returnPressed()
{
    emit triggered(m_lineEdit->text());
}


void LabeledNoteWidget::enterEvent(QEvent *event)
{
    m_action->activate(QAction::Hover);
    QWidget::enterEvent(event);
}

ReferencesTool::ReferencesTool(KoCanvasBase* canvas): TextTool(canvas),
    m_configure(0),
    m_stocw(0)
{
    createActions();
}

ReferencesTool::~ReferencesTool()
{
}

void ReferencesTool::createActions()
{
    KAction *action = new KAction(i18n("Insert"), this);
    addAction("insert_tableofcontents", action);
    action->setToolTip(i18n("Insert a Table of Contents into the document."));

    action = new KAction(i18n("Insert Custom ..."), this);
    addAction("insert_configure_tableofcontents", action);
    action->setToolTip(i18n("Insert a custom Table of Contents into the document."));

    action = new KAction(i18n("Configure..."), this);
    addAction("format_tableofcontents", action);
    action->setToolTip(i18n("Configure the Table of Contents"));
    connect(action, SIGNAL(triggered()), this, SLOT(formatTableOfContents()));

    action = new KAction(i18n("Insert with auto number"),this);
    addAction("insert_autofootnote",action);
    connect(action, SIGNAL(triggered()), this, SLOT(insertAutoFootNote()));

    action = new KAction(i18n("Insert Labeled Footnote"), this);
    QWidget *w = new LabeledNoteWidget(action);
    action->setDefaultWidget(w);
    addAction("insert_labeledfootnote", action);
    connect(w, SIGNAL(triggered(QString)), this, SLOT(insertLabeledFootNote(QString)));

    action = new KAction(i18n("Insert with auto number"),this);
    addAction("insert_autoendnote",action);
    connect(action, SIGNAL(triggered()), this, SLOT(insertAutoEndNote()));

    action = new KAction(i18n("Insert Labeled Endnote"), this);
    w = new LabeledNoteWidget(action);
    action->setDefaultWidget(w);
    addAction("insert_labeledendnote", action);
    connect(w, SIGNAL(triggered(QString)), this, SLOT(insertLabeledEndNote(QString)));

    action = new KAction(koIcon("configure"), i18n("Settings..."), this);
    addAction("format_footnotes",action);
    connect(action, SIGNAL(triggered()), this, SLOT(showFootnotesConfigureDialog()));

    action = new KAction(koIcon("configure"), i18n("Settings..."), this);
    addAction("format_endnotes",action);
    connect(action, SIGNAL(triggered()), this, SLOT(showEndnotesConfigureDialog()));

    action = new KAction(i18n("Insert Citation"),this);
    addAction("insert_citation",action);
    action->setToolTip(i18n("Insert a citation into the document."));
    connect(action, SIGNAL(triggered()), this, SLOT(insertCitation()));

    action = new KAction(i18n("Insert Bibliography"),this);
    addAction("insert_bibliography",action);
    action->setToolTip(i18n("Insert a bibliography into the document."));

    action = new KAction(i18n("Insert Custom Bibliography"), this);
    addAction("insert_custom_bibliography", action);
    action->setToolTip(i18n("Insert a custom Bibliography into the document."));
    action = new KAction(i18n("Configure"),this);
    addAction("configure_bibliography",action);
    action->setToolTip(i18n("Configure the bibliography"));
    connect(action, SIGNAL(triggered()), this, SLOT(configureBibliography()));
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
    m_stocw = new SimpleTableOfContentsWidget(this, 0);

    m_sfenw = new SimpleFootEndNotesWidget(this,0);

    m_scbw = new SimpleCitationBibliographyWidget(this,0);
    // Connect to/with simple table of contents option widget
    connect(m_stocw, SIGNAL(doneWithFocus()), this, SLOT(returnFocusToCanvas()));

    // Connect to/with simple citation index option widget
    //connect(scw, SIGNAL(doneWithFocus()), this, SLOT(returnFocusToCanvas()));

    // Connect to/with simple citation index option widget
    connect(m_sfenw, SIGNAL(doneWithFocus()), this, SLOT(returnFocusToCanvas()));

    m_stocw->setWindowTitle(i18n("Table of Contents"));
    widgets.append(m_stocw);

    m_sfenw->setWindowTitle(i18n("Footnotes and Endnotes"));
    widgets.append(m_sfenw);

    m_scbw->setWindowTitle(i18n("Citations and Bibliography"));
    widgets.append(m_scbw);
    //widgets.insert(i18n("Captions"), scapw);
    connect(textEditor(), SIGNAL(cursorPositionChanged()), this, SLOT(updateButtons()));
    return widgets;
}

void ReferencesTool::insertCitation()
{
    new CitationInsertionDialog(textEditor(), m_scbw);
}

void ReferencesTool::insertCustomBibliography(KoBibliographyInfo *defaultTemplate)
{
    Q_UNUSED(defaultTemplate);
    new InsertBibliographyDialog(textEditor(), m_scbw);
}

void ReferencesTool::configureBibliography()
{
    new BibliographyConfigureDialog(textEditor()->document(), m_scbw);
}

void ReferencesTool::formatTableOfContents()
{
    const QTextDocument *document = textEditor()->document();
    QMenu *tocList = new QMenu(m_stocw);
    int i = 0;
    QTextBlock firstToCTextBlock;
    for (QTextBlock it = document->begin(); it != document->end(); it = it.next())
    {
        if (it.blockFormat().hasProperty(KoParagraphStyle::TableOfContentsData)) {
            KoTableOfContentsGeneratorInfo *info = it.blockFormat().property(KoParagraphStyle::TableOfContentsData).value<KoTableOfContentsGeneratorInfo*>();
            if (i == 0) {
                firstToCTextBlock = it;
            }
            QAction *action = new QAction(info->m_indexTitleTemplate.text, tocList);
            action->setData(QVariant::fromValue<QTextBlock>(it));
            tocList->addAction(action);
            i++;
        }
    }

    if (i == 0) {
        //no ToCs in the document
        return;
    } else if (i == 1 && firstToCTextBlock.isValid()) {
        m_configure = new TableOfContentsConfigure(textEditor(), firstToCTextBlock, m_stocw);
        connect(m_configure, SIGNAL(finished(int)), this, SLOT(hideCofigureDialog(int)));
    } else {
        m_stocw->setToCConfigureMenu(tocList);
        connect(m_stocw->ToCConfigureMenu(), SIGNAL(triggered(QAction *)), SLOT(showConfigureDialog(QAction*)));
        m_stocw->showMenu();
    }
}

void ReferencesTool::showConfigureDialog(QAction *action)
{
    m_configure = new TableOfContentsConfigure(textEditor(), action->data().value<QTextBlock>(), m_stocw);
    connect(m_configure, SIGNAL(finished(int)), this, SLOT(hideCofigureDialog(int)));
}

void ReferencesTool::hideCofigureDialog(int result)
{
    disconnect(m_configure, SIGNAL(finished(int)), this, SLOT(hideCofigureDialog(int)));
    m_configure->deleteLater();
}

void ReferencesTool::insertAutoFootNote()
{
    m_note = textEditor()->insertFootNote();
    m_note->setAutoNumbering(true);
}

void ReferencesTool::insertLabeledFootNote(QString label)
{
    m_note = textEditor()->insertFootNote();
    m_note->setAutoNumbering(false);
    m_note->setLabel(label);
}

void ReferencesTool::insertAutoEndNote()
{
    m_note = textEditor()->insertEndNote();
    m_note->setAutoNumbering(true);
}

void ReferencesTool::insertLabeledEndNote(QString label)
{
    m_note = textEditor()->insertEndNote();
    m_note->setAutoNumbering(false);
    m_note->setLabel(label);
}

void ReferencesTool::showFootnotesConfigureDialog()
{
    NotesConfigurationDialog *dialog = new NotesConfigurationDialog((QTextDocument *)textEditor()->document(), true);
    dialog->exec();
}

void ReferencesTool::showEndnotesConfigureDialog()
{
    NotesConfigurationDialog *dialog = new NotesConfigurationDialog((QTextDocument *)textEditor()->document(), false);
    dialog->exec();
}

void ReferencesTool::updateButtons()
{
    if (textEditor()->currentFrame()->format().intProperty(KoText::SubFrameType) == KoText::NoteFrameType) {
        m_sfenw->widget.addFootnote->setEnabled(false);
        m_sfenw->widget.addEndnote->setEnabled(false);
    } else {
        m_sfenw->widget.addFootnote->setEnabled(true);
        m_sfenw->widget.addEndnote->setEnabled(true);
    }
}

KoTextEditor *ReferencesTool::editor()
{
    return textEditor();
}

void ReferencesTool::insertCustomToC(KoTableOfContentsGeneratorInfo *defaultTemplate)
{
    m_configure = new TableOfContentsConfigure(textEditor(), defaultTemplate, m_stocw);
    connect(m_configure, SIGNAL(accepted()), this, SLOT(customToCGenerated()));
    connect(m_configure, SIGNAL(finished(int)), this, SLOT(hideCofigureDialog(int)));
}

void ReferencesTool::customToCGenerated()
{
    if (m_configure) {
        textEditor()->insertTableOfContents(m_configure->currentToCData());
    }
}

#include <ReferencesTool.moc>
