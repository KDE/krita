/* This file is part of the KDE project
 * Copyright (C) 2008 Girish Ramakrishnan <girish@forwardbias.in>
 * Copyright (C) 2009 Thomas Zander <zander@kde.org>
 * Copyright (C) 2008 Pierre Stirnweiss \pierre.stirnweiss_koffice@gadz.org>
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

#include <QTextDocument>
#include <QTextCursor>
#include <QUrl>
#include <QVariant>

#include <kdebug.h>
#include <KUndoStack>

#include "KoTextDocument.h"
#include "KoTextEditor.h"
#include "styles/KoStyleManager.h"
#include "KoInlineTextObjectManager.h"
#include "KoTextDocumentLayout.h"
#include "styles/KoParagraphStyle.h"
#include "KoList.h"
#include "KoOdfLineNumberingConfiguration.h"
#include "KoOdfNotesConfiguration.h"

const QUrl KoTextDocument::StyleManagerURL = QUrl("kotext://stylemanager");
const QUrl KoTextDocument::ListsURL = QUrl("kotext://lists");
const QUrl KoTextDocument::InlineObjectTextManagerURL = QUrl("kotext://inlineObjectTextManager");
const QUrl KoTextDocument::UndoStackURL = QUrl("kotext://undoStack");
const QUrl KoTextDocument::ChangeTrackerURL = QUrl("kotext://changetracker");
const QUrl KoTextDocument::TextEditorURL = QUrl("kotext://textEditor");
const QUrl KoTextDocument::EndNotesConfigurationURL = QUrl("kotext://endnotesconfiguration");
const QUrl KoTextDocument::FootNotesConfigurationURL = QUrl("kotext://footnotesconfiguration");
const QUrl KoTextDocument::LineNumberingConfigurationURL = QUrl("kotext://linenumberingconfiguration");

KoTextDocument::KoTextDocument(QTextDocument *document)
    : m_document(document)
{
    Q_ASSERT(m_document);
}

KoTextDocument::KoTextDocument(const QTextDocument *document)
    : m_document(const_cast<QTextDocument *>(document))
{
    Q_ASSERT(m_document);
}

KoTextDocument::~KoTextDocument()
{
}

QTextDocument *KoTextDocument::document() const
{
    return m_document;
}

void KoTextDocument::setTextEditor (KoTextEditor* textEditor)
{
    QVariant v;
    v.setValue(textEditor);
    m_document->addResource(KoTextDocument::TextEditor, TextEditorURL, v);
}

KoTextEditor* KoTextDocument::textEditor()
{
    QVariant resource = m_document->resource(KoTextDocument::TextEditor, TextEditorURL);
    return resource.value<KoTextEditor *>();
}

void KoTextDocument::setStyleManager(KoStyleManager *sm)
{
    QVariant v;
    v.setValue(sm);
    m_document->addResource(KoTextDocument::StyleManager, StyleManagerURL, v);
    if (sm)
        sm->add(m_document);
}

void KoTextDocument::setInlineTextObjectManager(KoInlineTextObjectManager *manager)
{
    QVariant v;
    v.setValue(manager);
    m_document->addResource(KoTextDocument::InlineTextManager, InlineObjectTextManagerURL, v);

    KoTextDocumentLayout *lay = qobject_cast<KoTextDocumentLayout*>(m_document->documentLayout());
    if (lay)
        lay->setInlineTextObjectManager(manager);
}

KoStyleManager *KoTextDocument::styleManager() const
{
    QVariant resource = m_document->resource(KoTextDocument::StyleManager, StyleManagerURL);
    return resource.value<KoStyleManager *>();
}

void KoTextDocument::setChangeTracker(KoChangeTracker *changeTracker)
{
    QVariant v;
    v.setValue(changeTracker);
    m_document->addResource(KoTextDocument::ChangeTrackerResource, ChangeTrackerURL, v);
}

KoChangeTracker *KoTextDocument::changeTracker() const
{
    QVariant resource = m_document->resource(KoTextDocument::ChangeTrackerResource, ChangeTrackerURL);
    return resource.value<KoChangeTracker *>();
}


void KoTextDocument::setNotesConfiguration(KoOdfNotesConfiguration *notesConfiguration)
{
    QVariant v;
    v.setValue(notesConfiguration);
    if (notesConfiguration->noteClass() == KoOdfNotesConfiguration::Footnote) {
        m_document->addResource(KoTextDocument::FootNotesConfiguration, FootNotesConfigurationURL, v);
    }
    else {
        m_document->addResource(KoTextDocument::EndNotesConfiguration, EndNotesConfigurationURL, v);
    }
}

KoOdfNotesConfiguration *KoTextDocument::notesConfiguration(KoOdfNotesConfiguration::NoteClass noteClass) const
{
    if (noteClass == KoOdfNotesConfiguration::Endnote) {
        return m_document->resource(KoTextDocument::EndNotesConfiguration, EndNotesConfigurationURL)
                .value<KoOdfNotesConfiguration*>();
    }
    else {
        return m_document->resource(KoTextDocument::FootNotesConfiguration, FootNotesConfigurationURL)
                .value<KoOdfNotesConfiguration*>();
    }
}

void KoTextDocument::setLineNumberingConfiguration(KoOdfLineNumberingConfiguration *lineNumberingConfiguration)
{
    QVariant v;
    v.setValue(lineNumberingConfiguration);
    m_document->addResource(KoTextDocument::LineNumberingConfiguration, LineNumberingConfigurationURL, v);
}

KoOdfLineNumberingConfiguration *KoTextDocument::lineNumberingConfiguration() const
{
    return m_document->resource(KoTextDocument::LineNumberingConfiguration, LineNumberingConfigurationURL)
            .value<KoOdfLineNumberingConfiguration*>();
}

void KoTextDocument::setUndoStack(KUndoStack *undoStack)
{
    QVariant v;
    v.setValue<void*>(undoStack);
    m_document->addResource(KoTextDocument::UndoStack, UndoStackURL, v);
}

KUndoStack *KoTextDocument::undoStack() const
{
    QVariant resource = m_document->resource(KoTextDocument::UndoStack, UndoStackURL);
    return static_cast<KUndoStack*>(resource.value<void*>());
}

void KoTextDocument::setLists(const QList<KoList *> &lists)
{
    QVariant v;
    v.setValue(lists);
    m_document->addResource(KoTextDocument::Lists, ListsURL, v);
}

QList<KoList *> KoTextDocument::lists() const
{
    QVariant resource = m_document->resource(KoTextDocument::Lists, ListsURL);
    return resource.value<QList<KoList *> >();
}

void KoTextDocument::addList(KoList *list)
{
    Q_ASSERT(list);
    list->setParent(m_document);
    QList<KoList *> l = lists();
    if (l.contains(list))
        return;
    l.append(list);
    setLists(l);
}

void KoTextDocument::removeList(KoList *list)
{
    QList<KoList *> l = lists();
    if (l.contains(list)) {
        l.removeAll(list);
        setLists(l);
    }
}

KoList *KoTextDocument::list(const QTextBlock &block) const
{
    QTextList *textList = block.textList();
    if (!textList)
        return 0;
    return list(textList);
}

KoList *KoTextDocument::list(QTextList *textList) const
{
    // FIXME: this is horrible.
    foreach(KoList *l, lists()) {
        if (l->textLists().contains(textList))
            return l;
    }
    return 0;
}

KoList *KoTextDocument::list(KoListStyle::ListIdType listId) const
{
    foreach(KoList *l, lists()) {
        if (l->textListIds().contains(listId))
            return l;
    }
    return 0;
}

void KoTextDocument::clearText()
{
    QTextCursor cursor(m_document);
    cursor.select(QTextCursor::Document);
    cursor.removeSelectedText();
}

KoInlineTextObjectManager *KoTextDocument::inlineTextObjectManager() const
{
    QVariant resource = m_document->resource(KoTextDocument::InlineTextManager,
            InlineObjectTextManagerURL);
    return resource.value<KoInlineTextObjectManager *>();
}

void KoTextDocument::setResizeMethod(KoTextDocument::ResizeMethod method)
{
    KoTextDocumentLayout *layout = dynamic_cast<KoTextDocumentLayout*>(m_document->documentLayout());
    Q_ASSERT(layout);
    layout->setResizeMethod(method);
}

KoTextDocument::ResizeMethod KoTextDocument::resizeMethod() const
{
    KoTextDocumentLayout *layout = dynamic_cast<KoTextDocumentLayout*>(m_document->documentLayout());
    Q_ASSERT(layout);
    return layout->resizeMethod();
}
