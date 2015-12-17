/* This file is part of the KDE project
 * Copyright (C) 2008 Girish Ramakrishnan <girish@forwardbias.in>
 * Copyright (C) 2009 Thomas Zander <zander@kde.org>
 * Copyright (C) 2008 Pierre Stirnweiss \pierre.stirnweiss_calligra@gadz.org>
 * Copyright (C) 2011-2012 C. Boemann <cbo@boemann.dk>
 * Copyright (C) 2014-2015 Denis Kuplyakov <dener.kup@gmail.com>
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
 * You should have received a copy of the GNU Library General Public Licen give you se
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "KoTextDocument.h"

#include <QTextCursor>
#include <QVariant>
#include <QVariantList>

#include "TextDebug.h"
#include <KoTextDebug.h>

#include <kundo2stack.h>

#include "KoText.h"
#include "KoTextEditor.h"
#include "styles/KoStyleManager.h"
#include "OdfTextTrackStyles.h"
#include "KoTextRangeManager.h"
#include "KoInlineTextObjectManager.h"
#include "KoList.h"
#include "KoOdfLineNumberingConfiguration.h"
#include "changetracker/KoChangeTracker.h"
#include <KoShapeController.h>
#include <KoSectionModel.h>

Q_DECLARE_METATYPE(QAbstractTextDocumentLayout::Selection)
Q_DECLARE_METATYPE(QTextFrame*)
Q_DECLARE_METATYPE(QTextCharFormat)
Q_DECLARE_METATYPE(QTextBlockFormat)

const QUrl KoTextDocument::StyleManagerURL = QUrl("kotext://stylemanager");
const QUrl KoTextDocument::ListsURL = QUrl("kotext://lists");
const QUrl KoTextDocument::InlineObjectTextManagerURL = QUrl("kotext://inlineObjectTextManager");
const QUrl KoTextDocument::TextRangeManagerURL = QUrl("kotext://textRangeManager");
const QUrl KoTextDocument::UndoStackURL = QUrl("kotext://undoStack");
const QUrl KoTextDocument::ChangeTrackerURL = QUrl("kotext://changetracker");
const QUrl KoTextDocument::TextEditorURL = QUrl("kotext://textEditor");
const QUrl KoTextDocument::LineNumberingConfigurationURL = QUrl("kotext://linenumberingconfiguration");
const QUrl KoTextDocument::RelativeTabsURL = QUrl("kotext://relativetabs");
const QUrl KoTextDocument::HeadingListURL = QUrl("kotext://headingList");
const QUrl KoTextDocument::SelectionsURL = QUrl("kotext://selections");
const QUrl KoTextDocument::LayoutTextPageUrl = QUrl("kotext://layoutTextPage");
const QUrl KoTextDocument::ParaTableSpacingAtStartUrl = QUrl("kotext://spacingAtStart");
const QUrl KoTextDocument::IndexGeneratorManagerUrl = QUrl("kotext://indexGeneratorManager");
const QUrl KoTextDocument::FrameCharFormatUrl = QUrl("kotext://frameCharFormat");
const QUrl KoTextDocument::FrameBlockFormatUrl = QUrl("kotext://frameBlockFormat");
const QUrl KoTextDocument::ShapeControllerUrl = QUrl("kotext://shapeController");
const QUrl KoTextDocument::SectionModelUrl = QUrl("ktext://sectionModel");

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

KoTextDocument::KoTextDocument(QWeakPointer<QTextDocument> document)
    : m_document(document.data())
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
    Q_ASSERT(textEditor->document() == m_document);

    QVariant v;
    v.setValue(textEditor);
    m_document->addResource(KoTextDocument::TextEditor, TextEditorURL, v);
}

KoTextEditor* KoTextDocument::textEditor() const
{
    QVariant resource = m_document->resource(KoTextDocument::TextEditor, TextEditorURL);
    return resource.value<KoTextEditor *>();
}

void KoTextDocument::setStyleManager(KoStyleManager *sm)
{
    QVariant v;
    v.setValue(sm);
    m_document->addResource(KoTextDocument::StyleManager, StyleManagerURL, v);
    if (sm) {
        OdfTextTrackStyles *cf = OdfTextTrackStyles::instance(sm);
        cf->registerDocument(m_document);
    }
}

void KoTextDocument::setInlineTextObjectManager(KoInlineTextObjectManager *manager)
{
    QVariant v;
    v.setValue(manager);
    m_document->addResource(KoTextDocument::InlineTextManager, InlineObjectTextManagerURL, v);
}

void KoTextDocument::setTextRangeManager(KoTextRangeManager *manager)
{
    QVariant v;
    v.setValue(manager);
    m_document->addResource(KoTextDocument::TextRangeManager, TextRangeManagerURL, v);
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
    if (resource.isValid()) {
        return resource.value<KoChangeTracker *>();
    }
    else {
        return 0;
    }
}

void KoTextDocument::setShapeController(KoShapeController *controller)
{
    QVariant v;
    v.setValue(controller);
    m_document->addResource(KoTextDocument::ShapeController, ShapeControllerUrl, v);
}

KoShapeController *KoTextDocument::shapeController() const
{
    QVariant resource = m_document->resource(KoTextDocument::ShapeController, ShapeControllerUrl);
    if (resource.isValid()) {
        return resource.value<KoShapeController *>();
    }
    else {
        return 0;
    }
}

void KoTextDocument::setLineNumberingConfiguration(KoOdfLineNumberingConfiguration *lineNumberingConfiguration)
{
    lineNumberingConfiguration->setParent(m_document);
    QVariant v;
    v.setValue(lineNumberingConfiguration);
    m_document->addResource(KoTextDocument::LineNumberingConfiguration, LineNumberingConfigurationURL, v);
}

KoOdfLineNumberingConfiguration *KoTextDocument::lineNumberingConfiguration() const
{
    return m_document->resource(KoTextDocument::LineNumberingConfiguration, LineNumberingConfigurationURL)
            .value<KoOdfLineNumberingConfiguration*>();
}

void KoTextDocument::setHeadingList(KoList *headingList)
{
    QVariant v;
    v.setValue(headingList);
    m_document->addResource(KoTextDocument::HeadingList, HeadingListURL, v);
}

KoList *KoTextDocument::headingList() const
{
    QVariant resource = m_document->resource(KoTextDocument::HeadingList, HeadingListURL);
    return resource.value<KoList *>();
}

void KoTextDocument::setUndoStack(KUndo2Stack *undoStack)
{
    QVariant v;
    v.setValue<void*>(undoStack);
    m_document->addResource(KoTextDocument::UndoStack, UndoStackURL, v);
}

KUndo2Stack *KoTextDocument::undoStack() const
{
    QVariant resource = m_document->resource(KoTextDocument::UndoStack, UndoStackURL);
    return static_cast<KUndo2Stack*>(resource.value<void*>());
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
    if (!textList) {
        return 0;
    }
    // FIXME: this is horrible.
    Q_FOREACH (KoList *l, lists()) {
        if (l->textLists().contains(textList))
            return l;
    }
    return 0;
}

KoList *KoTextDocument::list(KoListStyle::ListIdType listId) const
{
    Q_FOREACH (KoList *l, lists()) {
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

QVector< QAbstractTextDocumentLayout::Selection > KoTextDocument::selections() const
{
    QVariant resource = m_document->resource(KoTextDocument::Selections, SelectionsURL);
    QVariantList variants = resource.toList();

    QVector<QAbstractTextDocumentLayout::Selection> selections;
    Q_FOREACH (const QVariant &variant, variants) {
        selections.append(variant.value<QAbstractTextDocumentLayout::Selection>());
    }

    return selections;
}

void KoTextDocument::setSelections(const QVector< QAbstractTextDocumentLayout::Selection >& selections)
{
    QVariantList variants;
    Q_FOREACH (const QAbstractTextDocumentLayout::Selection &selection, selections) {
        variants.append(QVariant::fromValue<QAbstractTextDocumentLayout::Selection>(selection));
    }

    m_document->addResource(KoTextDocument::Selections, SelectionsURL, variants);
}

KoInlineTextObjectManager *KoTextDocument::inlineTextObjectManager() const
{
    QVariant resource = m_document->resource(KoTextDocument::InlineTextManager,
            InlineObjectTextManagerURL);
    return resource.value<KoInlineTextObjectManager *>();
}

KoTextRangeManager *KoTextDocument::textRangeManager() const
{
    QVariant resource = m_document->resource(KoTextDocument::TextRangeManager,
            TextRangeManagerURL);
    return resource.value<KoTextRangeManager *>();
}

QTextFrame *KoTextDocument::auxillaryFrame()
{
    QTextCursor cursor(m_document->rootFrame()->lastCursorPosition());
    cursor.movePosition(QTextCursor::PreviousCharacter);
    QTextFrame *frame = cursor.currentFrame();

    if (frame->format().intProperty(KoText::SubFrameType) != KoText::AuxillaryFrameType) {
        cursor = m_document->rootFrame()->lastCursorPosition();

        QTextFrameFormat format;
        format.setProperty(KoText::SubFrameType, KoText::AuxillaryFrameType);

        frame = cursor.insertFrame(format);
    }
    return frame;
}

void KoTextDocument::setRelativeTabs(bool relative)
{
    QVariant v(relative);
    m_document->addResource(KoTextDocument::RelativeTabs, RelativeTabsURL, v);
}

bool KoTextDocument::relativeTabs() const
{
    QVariant resource = m_document->resource(KoTextDocument::RelativeTabs, RelativeTabsURL);
    if (resource.isValid())
        return resource.toBool();
    else
        return true;
}

void KoTextDocument::setParaTableSpacingAtStart(bool spacingAtStart)
{
    QVariant v(spacingAtStart);
    m_document->addResource(KoTextDocument::ParaTableSpacingAtStart, ParaTableSpacingAtStartUrl, v);
}

bool KoTextDocument::paraTableSpacingAtStart() const
{
    QVariant resource = m_document->resource(KoTextDocument::ParaTableSpacingAtStart, ParaTableSpacingAtStartUrl);
    if (resource.isValid())
        return resource.toBool();
    else
        return false;
}

QTextCharFormat KoTextDocument::frameCharFormat() const
{
    QVariant resource = m_document->resource(KoTextDocument::FrameCharFormat, FrameCharFormatUrl);
    if (resource.isValid())
        return resource.value<QTextCharFormat>();
    else
        return QTextCharFormat();
}

void KoTextDocument::setFrameCharFormat(const QTextCharFormat &format)
{
    m_document->addResource(KoTextDocument::FrameCharFormat, FrameCharFormatUrl, QVariant::fromValue(format));
}

QTextBlockFormat KoTextDocument::frameBlockFormat() const
{
    QVariant resource = m_document->resource(KoTextDocument::FrameBlockFormat, FrameBlockFormatUrl);
    if (resource.isValid())
        return resource.value<QTextBlockFormat>();
    else
        return QTextBlockFormat();
}

void KoTextDocument::setFrameBlockFormat(const QTextBlockFormat &format)
{
    m_document->addResource(KoTextDocument::FrameBlockFormat, FrameBlockFormatUrl, QVariant::fromValue(format));
}

KoSectionModel* KoTextDocument::sectionModel()
{
    QVariant resource = m_document->resource(KoTextDocument::SectionModel, SectionModelUrl);
    if (!resource.isValid()) {
        setSectionModel(new KoSectionModel(document())); // Using create on demand strategy
    }
    return m_document->resource(KoTextDocument::SectionModel, SectionModelUrl).value<KoSectionModel *>();
}

void KoTextDocument::setSectionModel(KoSectionModel *model)
{
    QVariant v;
    v.setValue(model);
    m_document->addResource(KoTextDocument::SectionModel, SectionModelUrl, v);
}
