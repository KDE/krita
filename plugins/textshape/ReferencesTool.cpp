/* This file is part of the KDE project
 * Copyright (C) 2011 C. Boemann <cbo@boemann.dk>
 * Copyright (C) 2013 Aman Madaan <madaan.amanmadaan@gmail.com>
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
#include "dialogs/SimpleLinksWidget.h"
#include "dialogs/TableOfContentsConfigure.h"
#include "dialogs/NotesConfigurationDialog.h"
#include "dialogs/CitationInsertionDialog.h"
#include "dialogs/InsertBibliographyDialog.h"
#include "dialogs/BibliographyConfigureDialog.h"
#include "dialogs/LinkInsertionDialog.h"

#include <KoTextLayoutRootArea.h>
#include <KoCanvasBase.h>
#include <KoTextEditor.h>
#include <KoParagraphStyle.h>
#include <KoTableOfContentsGeneratorInfo.h>
#include <KoBookmark.h>
#include <KoInlineNote.h>
#include <KoTextDocumentLayout.h>
#include <KoIcon.h>
#include "kis_action_registry.h"

#include <QMessageBox>
#include <QDebug>
#include <QAction>
#include <QTextDocument>
#include <QLineEdit>
#include <QBoxLayout>
#include <QMenu>
#include <QWidgetAction>

LabeledWidget::LabeledWidget(QAction *action, const QString &label, LabelPosition lb, bool warningLabelRequired)
    : QWidget()
    , m_action(action)
{
    setMouseTracking(true);
    QBoxLayout *layout;
    QLabel *l = new QLabel(label);
    l->setWordWrap(true);
    m_lineEdit = new QLineEdit();
    if (lb == LabeledWidget::INLINE) { // label followed by line edit
        layout = new QHBoxLayout();
        l->setIndent(l->style()->pixelMetric(QStyle::PM_SmallIconSize)
                     + l->style()->pixelMetric(QStyle::PM_MenuPanelWidth) + 4);
    } else { //Label goes above the text edit
        layout = new QVBoxLayout();
        m_lineEdit->setFixedWidth(300); //TODO : assuming a reasonable width, is there a better way?
    }
    layout->addWidget(l);
    layout->addWidget(m_lineEdit);
    if (warningLabelRequired) {
        m_warningLabel[0] = new QLabel("");
        m_warningLabel[1] = new QLabel("");
        m_warningLabel[0]->setWordWrap(true);
        m_warningLabel[1]->setWordWrap(true);
        layout->addWidget(m_warningLabel[0]);
        layout->addWidget(m_warningLabel[1]);
    }
    layout->setMargin(0);
    setLayout(layout);
    connect(m_lineEdit, SIGNAL(returnPressed()), this, SLOT(returnPressed()));
    connect(m_lineEdit, SIGNAL(textChanged(QString)), this, SIGNAL(lineEditChanged(QString)));
}

void LabeledWidget::returnPressed()
{
    emit triggered(m_lineEdit->text());
}

void LabeledWidget::enterEvent(QEvent *event)
{
    m_action->activate(QAction::Hover);
    QWidget::enterEvent(event);
}

void LabeledWidget::setWarningText(int pos, const QString &warning)
{
    if ((m_warningLabel[pos] == NULL)) {
        return;
    }
    m_warningLabel[pos]->setText(warning);
}

void LabeledWidget::clearLineEdit()
{
    m_lineEdit->setText("");
}

ReferencesTool::ReferencesTool(KoCanvasBase *canvas): TextTool(canvas),
    m_configure(0),
    m_stocw(0),
    m_canvas(canvas)
{
    createActions();
}

ReferencesTool::~ReferencesTool()
{
}

void ReferencesTool::createActions()
{
    KisActionRegistry *actionRegistry = KisActionRegistry::instance();

    QAction *action = actionRegistry->makeQAction("insert_tableofcontents", this);
    addAction("insert_tableofcontents", action);

    action = actionRegistry->makeQAction("insert_configure_tableofcontents", this);
    addAction("insert_configure_tableofcontents", action);

    action = actionRegistry->makeQAction("format_tableofcontents", this);
    addAction("format_tableofcontents", action);
    connect(action, SIGNAL(triggered()), this, SLOT(formatTableOfContents()));

    action = actionRegistry->makeQAction("insert_autofootnote", this);
    addAction("insert_autofootnote", action);
    connect(action, SIGNAL(triggered()), this, SLOT(insertAutoFootNote()));

    QWidgetAction *wAction = new QWidgetAction(this);
    wAction->setText(i18n("Insert Labeled Footnote"));
    QWidget *w = new LabeledWidget(wAction, i18n("Insert with label:"), LabeledWidget::INLINE, false);
    wAction->setDefaultWidget(w);
    addAction("insert_labeledfootnote", wAction);
    connect(w, SIGNAL(triggered(QString)), this, SLOT(insertLabeledFootNote(QString)));

    action = actionRegistry->makeQAction("insert_autoendnote", this);
    addAction("insert_autoendnote", action);
    connect(action, SIGNAL(triggered()), this, SLOT(insertAutoEndNote()));

    wAction = new QWidgetAction(this);
    wAction->setText(i18n("Insert Labeled Endnote"));
    w = new LabeledWidget(wAction, i18n("Insert with label:"), LabeledWidget::INLINE, false);
    wAction->setDefaultWidget(w);
    addAction("insert_labeledendnote", wAction); connect(w, SIGNAL(triggered(QString)), this, SLOT(insertLabeledEndNote(QString)));

    action = actionRegistry->makeQAction("format_footnotes", this);
    addAction("format_footnotes", action);
    connect(action, SIGNAL(triggered()), this, SLOT(showFootnotesConfigureDialog()));

    action = actionRegistry->makeQAction("format_endnotes", this);
    addAction("format_endnotes", action);
    connect(action, SIGNAL(triggered()), this, SLOT(showEndnotesConfigureDialog()));

    action = actionRegistry->makeQAction("insert_citation", this);
    addAction("insert_citation", action);
    connect(action, SIGNAL(triggered()), this, SLOT(insertCitation()));

    action = actionRegistry->makeQAction("insert_bibliography", this);
    addAction("insert_bibliography", action);

    action = actionRegistry->makeQAction("insert_custom_bibliography", this);
    addAction("insert_custom_bibliography", action);

    action = actionRegistry->makeQAction("configure_bibliography", this);
    addAction("configure_bibliography", action);
    connect(action, SIGNAL(triggered()), this, SLOT(configureBibliography()));

    action = actionRegistry->makeQAction("insert_link", this);
    addAction("insert_link", action);
    connect(action, SIGNAL(triggered()), this, SLOT(insertLink()));

    wAction = new QWidgetAction(this);
    wAction->setText(i18n("Add Bookmark"));
    m_bmark = new LabeledWidget(wAction, i18n("Add Bookmark :"), LabeledWidget::ABOVE, true);
    connect(m_bmark, SIGNAL(lineEditChanged(QString)), this, SLOT(validateBookmark(QString)));
    wAction->setDefaultWidget(m_bmark);
    addAction("insert_bookmark", wAction);
    connect(m_bmark, SIGNAL(triggered(QString)), this, SLOT(insertBookmark(QString)));
    wAction->setToolTip(i18n("Insert a Bookmark. This is useful to create links that point to areas within the document"));

    action = actionRegistry->makeQAction("invoke_bookmark_handler", this);
    addAction("invoke_bookmark_handler", action);

    action = actionRegistry->makeQAction("manage_bookmarks", this);
    addAction("manage_bookmarks", action);
}

void ReferencesTool::activate(ToolActivation toolActivation, const QSet<KoShape *> &shapes)
{
    TextTool::activate(toolActivation, shapes);
}

void ReferencesTool::deactivate()
{
    TextTool::deactivate();
    canvas()->canvasWidget()->setFocus();
}

QList<QPointer<QWidget> > ReferencesTool::createOptionWidgets()
{
    QList<QPointer<QWidget> > widgets;
    m_stocw = new SimpleTableOfContentsWidget(this, 0);

    m_sfenw = new SimpleFootEndNotesWidget(this, 0);

    m_scbw = new SimpleCitationBibliographyWidget(this, 0);

    m_slw = new SimpleLinksWidget(this, 0);
    // Connect to/with simple table of contents option widget
    connect(m_stocw, SIGNAL(doneWithFocus()), this, SLOT(returnFocusToCanvas()));

    // Connect to/with simple citation index option widget
    //connect(scw, SIGNAL(doneWithFocus()), this, SLOT(returnFocusToCanvas()));

    // Connect to/with simple citation index option widget
    connect(m_sfenw, SIGNAL(doneWithFocus()), this, SLOT(returnFocusToCanvas()));

    connect(m_slw, SIGNAL(doneWithFocus()), this, SLOT(returnFocusToCanvas()));

    m_stocw->setWindowTitle(i18nc("as in table of contents, list of pictures, index", "Tables, Lists & Indexes"));
    widgets.append(m_stocw);

    m_sfenw->setWindowTitle(i18n("Footnotes and Endnotes"));
    widgets.append(m_sfenw);

    m_scbw->setWindowTitle(i18n("Citations and Bibliography"));
    widgets.append(m_scbw);

    m_slw->setWindowTitle(i18n("Links and Bookmarks"));
    widgets.append(m_slw);
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
    if (textEditor()->block().blockFormat().hasProperty(KoParagraphStyle::TableOfContentsData)) {
        m_configure = new TableOfContentsConfigure(textEditor(), textEditor()->block(), m_stocw);
        connect(m_configure, SIGNAL(finished(int)), this, SLOT(hideCofigureDialog()));
    }
}

void ReferencesTool::showConfigureDialog(QAction *action)
{
    m_configure = new TableOfContentsConfigure(textEditor(), action->data().value<QTextBlock>(), m_stocw);
    connect(m_configure, SIGNAL(finished(int)), this, SLOT(hideCofigureDialog()));
}

void ReferencesTool::hideCofigureDialog()
{
    disconnect(m_configure, SIGNAL(finished(int)), this, SLOT(hideCofigureDialog()));
    m_configure->deleteLater();
}

void ReferencesTool::insertAutoFootNote()
{
    m_note = textEditor()->insertFootNote();
    m_note->setAutoNumbering(true);
}

void ReferencesTool::insertLabeledFootNote(const QString &label)
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

void ReferencesTool::insertLabeledEndNote(const QString &label)
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
    if (textEditor()->block().blockFormat().hasProperty(KoParagraphStyle::TableOfContentsData)) {
        action("format_tableofcontents")->setEnabled(true);
    } else {
        action("format_tableofcontents")->setEnabled(false);
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
    connect(m_configure, SIGNAL(finished(int)), this, SLOT(hideCofigureDialog()));
}

void ReferencesTool::customToCGenerated()
{
    if (m_configure) {
        textEditor()->insertTableOfContents(m_configure->currentToCData());
    }
}

void ReferencesTool::insertLink()
{
    new LinkInsertionDialog(textEditor(), m_slw);
}

bool ReferencesTool::validateBookmark(QString bookmarkName)
{
    bookmarkName = bookmarkName.trimmed();
    if (bookmarkName.isEmpty()) {
        m_bmark->setWarningText(0, i18n("Bookmark cannot be empty"));
        return false;
    }
    const KoBookmarkManager *manager = KoTextDocument(editor()->document()).textRangeManager()->bookmarkManager();
    QStringList existingBookmarks = manager->bookmarkNameList();
    int position = existingBookmarks.indexOf(bookmarkName);
    if (position != -1) {
        m_bmark->setWarningText(0, i18n("Duplicate Name. Click \"Manage Bookmarks\""));
        m_bmark->setWarningText(1, i18n("to Rename or Delete Bookmarks"));
        return false;
    } else {
        m_bmark->setWarningText(0, "");
        m_bmark->setWarningText(1, "");
        return true;
    }
}

void ReferencesTool::insertBookmark(QString bookMarkName)
{
    bookMarkName = bookMarkName.trimmed();
    m_bmark->setWarningText(0, "");
    m_bmark->setWarningText(1, "");
    if (validateBookmark(bookMarkName)) {
        editor()->addBookmark(bookMarkName);
        m_bmark->clearLineEdit();
    }
}
