/* This file is part of the KDE project
 * Copyright (C) 2009-2012 Pierre Stirnweiss <pstirnweiss@googlemail.com>
 * Copyright (C) 2006-2010 Thomas Zander <zander@kde.org>
 * Copyright (c) 2011 Boudewijn Rempt <boud@kogmbh.com>
 * Copyright (C) 2011-2015 C. Boemann <cbo@boemann.dk>
 * Copyright (C) 2014-2015 Denis Kuplyakov <dener.kup@gmail.com>
 * Copyright (C) 2015 Soma Schliszka <soma.schliszka@gmail.com>
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

#include "KoTextEditor.h"
#include "KoTextEditor_p.h"

#include "KoList.h"
#include "KoBookmark.h"
#include "KoAnnotation.h"
#include "KoTextRangeManager.h"
#include "KoInlineTextObjectManager.h"
#include "KoInlineNote.h"
#include "KoInlineCite.h"
#include "BibliographyGenerator.h"
#include <KoTextShapeDataBase.h>
#include <KoSelection.h>
#include <KoShapeController.h>
#include <KoShapeManager.h>
#include <KoCanvasBase.h>
#include "KoShapeAnchor.h"
#include "KoTextDocument.h"
#include "KoTextLocator.h"
#include "KoTableOfContentsGeneratorInfo.h"
#include "KoBibliographyInfo.h"
#include "changetracker/KoChangeTracker.h"
#include "changetracker/KoChangeTrackerElement.h"
#include "styles/KoCharacterStyle.h"
#include "styles/KoParagraphStyle.h"
#include "styles/KoStyleManager.h"
#include "styles/KoTableCellStyle.h"
#include "styles/KoTableStyle.h"
#include "KoTableColumnAndRowStyleManager.h"
#include "commands/DeleteTableRowCommand.h"
#include "commands/DeleteTableColumnCommand.h"
#include "commands/InsertTableRowCommand.h"
#include "commands/InsertTableColumnCommand.h"
#include "commands/ResizeTableCommand.h"
#include "commands/TextPasteCommand.h"
#include "commands/ListItemNumberingCommand.h"
#include "commands/ChangeListCommand.h"
#include "commands/InsertInlineObjectCommand.h"
#include "commands/DeleteCommand.h"
#include "commands/DeleteAnchorsCommand.h"
#include "commands/DeleteAnnotationsCommand.h"
#include "commands/InsertNoteCommand.h"
#include "commands/AddTextRangeCommand.h"
#include "commands/AddAnnotationCommand.h"
#include "commands/RenameSectionCommand.h"
#include "commands/NewSectionCommand.h"
#include "commands/SplitSectionsCommand.h"

#include <klocalizedstring.h>

#include <QTextList>
#include <QTextBlock>
#include <QTextBlockFormat>
#include <QTextCharFormat>
#include <QTextDocument>
#include <QTextDocumentFragment>
#include <QTextFormat>
#include <QTextTable>
#include <QTextTableCell>
#include <kundo2command.h>

#include "TextDebug.h"
#include "KoTextDebug.h"

Q_DECLARE_METATYPE(QTextFrame*)

/*Private*/

KoTextEditor::Private::Private(KoTextEditor *qq, QTextDocument *document)
    : q(qq)
    , document (document)
    , addNewCommand(true)
    , dummyMacroAdded(false)
    , customCommandCount(0)
    , editProtectionCached(false)
{
    caret = QTextCursor(document);
    editorState = NoOp;
}

void KoTextEditor::Private::emitTextFormatChanged()
{
    emit q->textFormatChanged();
}

void KoTextEditor::Private::newLine(KUndo2Command *parent)
{
    // Handle if this is the special block before a table
    bool hiddenTableHandling = caret.blockFormat().hasProperty(KoParagraphStyle::HiddenByTable);
    if (hiddenTableHandling) {
        // Easy solution is to go back to the end of previous block and do the insertion from there.
        // However if there is no block before we have a problem. This may be the case if there is
        // a table before or we are at the beginning of a cell or a document.
        // So here is a better approach
        // 1) create block
        // 2) select the previous block so it get's deleted and replaced
        // 3) remove HiddenByTable from both new and previous block
        // 4) actually make new line replacing the block we just inserted
        // 5) set HiddenByTable on the block just before the table again
        caret.insertText("oops you should never see this");
        caret.insertBlock();
        caret.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor);
        caret.movePosition(QTextCursor::StartOfBlock, QTextCursor::KeepAnchor);
        QTextBlockFormat bf = caret.blockFormat();
        bf.clearProperty(KoParagraphStyle::HiddenByTable);
        caret.setBlockFormat(bf);
   }


    if (caret.hasSelection()) {
        q->deleteChar(false, parent);
    }
    KoTextDocument textDocument(document);
    KoStyleManager *styleManager = textDocument.styleManager();
    KoParagraphStyle *nextStyle = 0;
    KoParagraphStyle *currentStyle = 0;
    if (styleManager) {
        int id = caret.blockFormat().intProperty(KoParagraphStyle::StyleId);
        currentStyle = styleManager->paragraphStyle(id);
        if (currentStyle == 0) // not a style based parag.  Lets make the next one correct.
            nextStyle = styleManager->defaultParagraphStyle();
        else
            nextStyle = styleManager->paragraphStyle(currentStyle->nextStyle());
        Q_ASSERT(nextStyle);
        if (currentStyle == nextStyle)
            nextStyle = 0;
    }

    QTextCharFormat format = caret.charFormat();
    if (format.hasProperty(KoCharacterStyle::ChangeTrackerId)) {
        format.clearProperty(KoCharacterStyle::ChangeTrackerId);
    }

    // Build the block format and subtract the properties that are not inherited
    QTextBlockFormat bf = caret.blockFormat();

    bf.clearProperty(KoParagraphStyle::BreakBefore);
    bf.clearProperty(KoParagraphStyle::ListStartValue);
    bf.clearProperty(KoParagraphStyle::UnnumberedListItem);
    bf.clearProperty(KoParagraphStyle::IsListHeader);
    bf.clearProperty(KoParagraphStyle::MasterPageName);
    bf.clearProperty(KoParagraphStyle::OutlineLevel);
    bf.clearProperty(KoParagraphStyle::HiddenByTable);

    // We should stay in the same section so we can't start new one.
    bf.clearProperty(KoParagraphStyle::SectionStartings);
    // But we move all the current endings to the next paragraph.
    QTextBlockFormat origin = caret.blockFormat();
    origin.clearProperty(KoParagraphStyle::SectionEndings);
    caret.setBlockFormat(origin);

    // Build the block char format which is just a copy
    QTextCharFormat bcf = caret.blockCharFormat();

    // Actually insert the new paragraph char
    int startPosition = caret.position();

    caret.insertBlock(bf, bcf);

    int endPosition = caret.position();

    // Mark the CR as a tracked change
    QTextCursor changeCursor(document);
    changeCursor.beginEditBlock();
    changeCursor.setPosition(startPosition);
    changeCursor.setPosition(endPosition, QTextCursor::KeepAnchor);
    changeCursor.endEditBlock();

    q->registerTrackedChange(changeCursor, KoGenChange::InsertChange, kundo2_i18n("New Paragraph"), format, format, false);

    // possibly change the style if requested
    if (nextStyle) {
        QTextBlock block = caret.block();
        if (currentStyle)
            currentStyle->unapplyStyle(block);
        nextStyle->applyStyle(block);
        format = block.charFormat();
    }

    caret.setCharFormat(format);

    if (hiddenTableHandling) {
        // see code and comment above
        QTextBlockFormat bf = caret.blockFormat();
        bf.setProperty(KoParagraphStyle::HiddenByTable, true);
        caret.setBlockFormat(bf);
        caret.movePosition(QTextCursor::PreviousCharacter);
    }
}

/*KoTextEditor*/

//TODO factor out the changeTracking charFormat setting from all individual slots to a public slot, which will be available for external commands (TextShape)

//The BlockFormatVisitor and CharFormatVisitor are used when a property needs to be modified relative to its current value (which could be different over the selection). For example: increase indentation by 10pt.
//The BlockFormatVisitor is also used for the change tracking of a blockFormat. The changeTracker stores the information about the changeId in the charFormat. The BlockFormatVisitor ensures that thd changeId is set on the whole block (even if only a part of the block is actually selected).
//Should such mechanisms be later provided directly by Qt, we could dispose of these classes.


KoTextEditor::KoTextEditor(QTextDocument *document)
    : QObject(document),
      d (new Private(this, document))
{
    connect (d->document, SIGNAL (undoCommandAdded()), this, SLOT (documentCommandAdded()));
}

KoTextEditor::~KoTextEditor()
{
    delete d;
}

KoTextEditor *KoTextEditor::getTextEditorFromCanvas(KoCanvasBase *canvas)
{
    KoSelection *selection = canvas->shapeManager()->selection();
    if (selection) {
        Q_FOREACH (KoShape *shape, selection->selectedShapes()) {
            if (KoTextShapeDataBase *textData = qobject_cast<KoTextShapeDataBase*>(shape->userData())) {
                KoTextDocument doc(textData->document());
                return doc.textEditor();
            }
        }
    }
    return 0;
}

QTextCursor* KoTextEditor::cursor()
{
    return &(d->caret);
}

const QTextCursor KoTextEditor::constCursor() const
{
    return QTextCursor(d->caret);
}

void KoTextEditor::registerTrackedChange(QTextCursor &selection, KoGenChange::Type changeType, const KUndo2MagicString &title, QTextFormat& format, QTextFormat& prevFormat, bool applyToWholeBlock)
{
    KoChangeTracker *changeTracker = KoTextDocument(d->document).changeTracker();
    if (!changeTracker || !changeTracker->recordChanges()) {
        // clear the ChangeTrackerId from the passed in selection, without recursively registring
        // change tracking again  ;)
        int start = qMin(selection.position(), selection.anchor());
        int end = qMax(selection.position(), selection.anchor());

        QTextBlock block = selection.block();
        if (block.position() > start)
            block = block.document()->findBlock(start);

        while (block.isValid() && block.position() < end) {
            QTextBlock::iterator iter = block.begin();
            while (!iter.atEnd()) {
                QTextFragment fragment = iter.fragment();
                if (fragment.position() > end) {
                    break;
                }

                if (fragment.position() + fragment.length() <= start) {
                    ++iter;
                    continue;
                }

                QTextCursor cursor(block);
                cursor.setPosition(fragment.position());
                QTextCharFormat fm = fragment.charFormat();

                if (fm.hasProperty(KoCharacterStyle::ChangeTrackerId)) {
                    fm.clearProperty(KoCharacterStyle::ChangeTrackerId);
                    int to = qMin(end, fragment.position() + fragment.length());
                    cursor.setPosition(to, QTextCursor::KeepAnchor);
                    cursor.setCharFormat(fm);
                    iter = block.begin();
                } else {
                    ++iter;
                }
            }
            block = block.next();
        }
    } else {
        if (changeType != KoGenChange::DeleteChange) {
            //first check if there already is an identical change registered just before or just after the selection. If so, merge appropriatly.
            //TODO implement for format change. handle the prevFormat/newFormat check.
            QTextCursor checker = QTextCursor(selection);
            int idBefore = 0;
            int idAfter = 0;
            int changeId = 0;
            int selectionBegin = qMin(checker.anchor(), checker.position());
            int selectionEnd = qMax(checker.anchor(), checker.position());

            checker.setPosition(selectionBegin);
            if (!checker.atBlockStart()) {
                int changeId = checker.charFormat().property(KoCharacterStyle::ChangeTrackerId).toInt();
                if (changeId && changeTracker->elementById(changeId)->getChangeType() == changeType)
                    idBefore = changeId;
            } else {
                if (!checker.currentTable()) {
                    int changeId = checker.blockFormat().intProperty(KoCharacterStyle::ChangeTrackerId);
                    if (changeId && changeTracker->elementById(changeId)->getChangeType() == changeType)
                        idBefore = changeId;
                } else {
                    idBefore = checker.currentTable()->format().intProperty(KoCharacterStyle::ChangeTrackerId);
                    if (!idBefore) {
                        idBefore = checker.currentTable()->cellAt(checker).format().intProperty(KoCharacterStyle::ChangeTrackerId);
                    }
                }
            }

            checker.setPosition(selectionEnd);
            if (!checker.atEnd()) {
                checker.movePosition(QTextCursor::NextCharacter);
                idAfter = changeTracker->mergeableId(changeType, title, checker.charFormat().property( KoCharacterStyle::ChangeTrackerId ).toInt());
            }
            changeId = (idBefore)?idBefore:idAfter;

            switch (changeType) {//TODO: this whole thing actually needs to be done like a visitor. If the selection contains several change regions, the parenting needs to be individualised.
            case KoGenChange::InsertChange:
                if (!changeId)
                    changeId = changeTracker->getInsertChangeId(title, 0);
                break;
            case KoGenChange::FormatChange:
                if (!changeId)
                    changeId = changeTracker->getFormatChangeId(title, format, prevFormat, 0);
                break;
            case KoGenChange::DeleteChange:
                //this should never be the case
                break;
            default:
                ;// do nothing
            }

            if (applyToWholeBlock) {
                selection.movePosition(QTextCursor::StartOfBlock);
                selection.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
            }

            QTextCharFormat f;
            f.setProperty(KoCharacterStyle::ChangeTrackerId, changeId);
            selection.mergeCharFormat(f);

            QTextBlock startBlock = selection.document()->findBlock(selection.anchor());
            QTextBlock endBlock = selection.document()->findBlock(selection.position());

            while (startBlock.isValid() && startBlock != endBlock) {
                startBlock = startBlock.next();
                QTextCursor cursor(startBlock);
                QTextBlockFormat blockFormat;
                blockFormat.setProperty(KoCharacterStyle::ChangeTrackerId, changeId);
                cursor.mergeBlockFormat(blockFormat);

                QTextCharFormat blockCharFormat = cursor.blockCharFormat();
                if (blockCharFormat.hasProperty(KoCharacterStyle::ChangeTrackerId)) {
                    blockCharFormat.clearProperty(KoCharacterStyle::ChangeTrackerId);
                    cursor.setBlockCharFormat(blockCharFormat);
                }
            }
        }
    }
}

// To figure out if a the blocks of the selection are write protected we need to
// traverse the entire document as sections build up the protectiveness recursively.
void KoTextEditor::recursivelyVisitSelection(QTextFrame::iterator it, KoTextVisitor &visitor) const
{
    do {
        if (visitor.abortVisiting())
            return;

        QTextBlock block = it.currentBlock();
        QTextTable *table = qobject_cast<QTextTable*>(it.currentFrame());
        QTextFrame *subFrame = it.currentFrame();
        if (table) {
            // There are 4 ways this table can be selected:
            //  - "before to mid"
            //  - "mid to after"
            //  - "complex mid to mid"
            //  - "simple mid to mid"
            // The 3 first are entire cells, the fourth is within a cell

            if (d->caret.selectionStart() <= table->lastPosition()
                    && d->caret.selectionEnd() >= table->firstPosition()) {
                // We have a selection somewhere
                QTextTableCell cell1 = table->cellAt(d->caret.selectionStart());
                QTextTableCell cell2 = table->cellAt(d->caret.selectionEnd());
                if (cell1 != cell2 || !cell1.isValid() || !cell2.isValid()) {
                    // And the selection is complex or entire table
                    int selectionRow;
                    int selectionColumn;
                    int selectionRowSpan;
                    int selectionColumnSpan;
                    if (!cell1.isValid() || !cell2.isValid()) {
                        // entire table
                        visitor.visitTable(table, KoTextVisitor::Entirely);
                        selectionRow = selectionColumn = 0;
                        selectionRowSpan = table->rows();
                        selectionColumnSpan = table->columns();
                    } else {
                        visitor.visitTable(table, KoTextVisitor::Partly);
                        d->caret.selectedTableCells(&selectionRow, &selectionRowSpan, &selectionColumn, &selectionColumnSpan);
                    }

                    for (int r = selectionRow; r < selectionRow + selectionRowSpan; r++) {
                        for (int c = selectionColumn; c < selectionColumn +
                             selectionColumnSpan; c++) {
                            QTextTableCell cell = table->cellAt(r,c);
                            if (!cell.format().boolProperty(KoTableCellStyle::CellIsProtected)) {
                                visitor.visitTableCell(&cell, KoTextVisitor::Partly);
                                recursivelyVisitSelection(cell.begin(), visitor);
                            } else {
                                visitor.nonVisit();
                            }

                            if (visitor.abortVisiting())
                                return;
                        }
                    }
                } else {
                    visitor.visitTable(table, KoTextVisitor::Partly);
                    // And the selection is simple
                    if (!cell1.format().boolProperty(KoTableCellStyle::CellIsProtected)) {
                        visitor.visitTableCell(&cell1, KoTextVisitor::Entirely);
                        recursivelyVisitSelection(cell1.begin(), visitor);
                    } else {
                        visitor.nonVisit();
                    }
                    return;
                }
            }
            if (d->caret.selectionEnd() <= table->lastPosition()) {
                return;
            }
        } else if (subFrame) {
            recursivelyVisitSelection(subFrame->begin(), visitor);
        } else {
            // TODO build up the section stack

            if (d->caret.selectionStart() < block.position() + block.length()
                    && d->caret.selectionEnd() >= block.position()) {
                // We have a selection somewhere
                if (true) { // TODO don't change if block is protected by section
                    visitor.visitBlock(block, d->caret);
                } else {
                    visitor.nonVisit();
                }
            }

            // TODO tear down the section stack

            if (d->caret.selectionEnd() < block.position() + block.length()) {
                return;
            }
        }
        if (!it.atEnd()) {
            ++it;
        }
    } while (!it.atEnd());
}

KoBookmark *KoTextEditor::addBookmark(const QString &name)
{//TODO changeTracking
    KUndo2Command *topCommand = beginEditBlock(kundo2_i18n("Add Bookmark"));

    KoBookmark *bookmark = new KoBookmark(d->caret);
    bookmark->setName(name);
    bookmark->setManager(KoTextDocument(d->document).textRangeManager());

    addCommand(new AddTextRangeCommand(bookmark, topCommand));

    endEditBlock();

    return bookmark;
}
KoTextRangeManager *KoTextEditor::textRangeManager() const
{
    return KoTextDocument(d->document).textRangeManager();
}

KoAnnotation *KoTextEditor::addAnnotation(KoShape *annotationShape)
{
    KUndo2Command *topCommand = beginEditBlock(kundo2_i18n("Add Annotation"));

    KoAnnotation *annotation = new KoAnnotation(d->caret);
    KoTextRangeManager *textRangeManager = KoTextDocument(d->document).textRangeManager();
    annotation->setManager(textRangeManager);
    //FIXME: I need the name, a unique name, we can set selected text as annotation name or use createUniqueAnnotationName function
    // to do it for us.
    QString name = annotation->createUniqueAnnotationName(textRangeManager->annotationManager(), "", false);
    annotation->setName(name);
    annotation->setAnnotationShape(annotationShape);

    addCommand(new AddAnnotationCommand(annotation, topCommand));

    endEditBlock();

    return annotation;
}

KoInlineObject *KoTextEditor::insertIndexMarker()
{//TODO changeTracking
    if (isEditProtected()) {
        return 0;
    }

    d->updateState(KoTextEditor::Private::Custom, kundo2_i18n("Insert Index"));

    if (d->caret.blockFormat().hasProperty(KoParagraphStyle::HiddenByTable)) {
        d->newLine(0);
    }

    QTextBlock block = d->caret.block();
    if (d->caret.position() >= block.position() + block.length() - 1)
        return 0; // can't insert one at end of text
    if (block.text()[ d->caret.position() - block.position()].isSpace())
        return 0; // can't insert one on a whitespace as that does not indicate a word.

    KoTextLocator *tl = new KoTextLocator();
    KoTextDocument(d->document).inlineTextObjectManager()->insertInlineObject(d->caret, tl);
    d->updateState(KoTextEditor::Private::NoOp);
    return tl;
}

void KoTextEditor::insertInlineObject(KoInlineObject *inliner, KUndo2Command *cmd)
{
    if (isEditProtected()) {
        return;
    }

    KUndo2Command *topCommand = cmd;
    if (!cmd) {
        topCommand = beginEditBlock(kundo2_i18n("Insert Variable"));
    }

    if (d->caret.hasSelection()) {
        deleteChar(false, topCommand);
    }
    d->caret.beginEditBlock();


    if (d->caret.blockFormat().hasProperty(KoParagraphStyle::HiddenByTable)) {
        d->newLine(0);
    }

    QTextCharFormat format = d->caret.charFormat();
    if (format.hasProperty(KoCharacterStyle::ChangeTrackerId)) {
        format.clearProperty(KoCharacterStyle::ChangeTrackerId);
    }

    InsertInlineObjectCommand *insertInlineObjectCommand = new InsertInlineObjectCommand(inliner, d->document, topCommand);
    Q_UNUSED(insertInlineObjectCommand);
    d->caret.endEditBlock();
 
    if (!cmd) {
        addCommand(topCommand);
        endEditBlock();
    }

    emit cursorPositionChanged();
}

void KoTextEditor::updateInlineObjectPosition(int start, int end)
{
    KoInlineTextObjectManager *inlineObjectManager = KoTextDocument(d->document).inlineTextObjectManager();
    // and, of course, every inline object after the current position has the wrong position
    QTextCursor cursor = d->document->find(QString(QChar::ObjectReplacementCharacter), start);
    while (!cursor.isNull() && (end > -1 && cursor.position() < end )) {
        QTextCharFormat fmt = cursor.charFormat();
        KoInlineObject *obj = inlineObjectManager->inlineTextObject(fmt);
        obj->updatePosition(d->document, cursor.position(), fmt);
        cursor = d->document->find(QString(QChar::ObjectReplacementCharacter), cursor.position());
    }

}

void KoTextEditor::removeAnchors(const QList<KoShapeAnchor*> &anchors, KUndo2Command *parent)
{
    Q_ASSERT(parent);
    addCommand(new DeleteAnchorsCommand(anchors, d->document, parent));
}

void KoTextEditor::removeAnnotations(const QList<KoAnnotation *> &annotations, KUndo2Command *parent)
{
    Q_ASSERT(parent);
    addCommand(new DeleteAnnotationsCommand(annotations, d->document, parent));
}

void KoTextEditor::insertFrameBreak()
{
    if (isEditProtected()) {
        return;
    }

    QTextCursor curr(d->caret.block());
    if (dynamic_cast<QTextTable *> (curr.currentFrame())) {
        return;
    }

    d->updateState(KoTextEditor::Private::KeyPress, kundo2_i18n("Insert Break"));
    QTextBlock block = d->caret.block();
    if (d->caret.position() == block.position() && block.length() > 0) { // start of parag
        QTextBlockFormat bf = d->caret.blockFormat();
        bf.setProperty(KoParagraphStyle::BreakBefore, KoText::PageBreak);
        d->caret.insertBlock(bf);
        if (block.textList())
            block.textList()->remove(block);
    } else {
        QTextBlockFormat bf = d->caret.blockFormat();
        if (!d->caret.blockFormat().hasProperty(KoParagraphStyle::HiddenByTable)) {
            newLine();
        }
        bf = d->caret.blockFormat();
        bf.setProperty(KoParagraphStyle::BreakBefore, KoText::PageBreak);
        d->caret.setBlockFormat(bf);
    }
    d->updateState(KoTextEditor::Private::NoOp);
    emit cursorPositionChanged();
}

void KoTextEditor::paste(KoCanvasBase *canvas, const QMimeData *mimeData, bool pasteAsText)
{
    if (isEditProtected()) {
        return;
    }

    KoShapeController *shapeController = KoTextDocument(d->document).shapeController();

    addCommand(new TextPasteCommand(mimeData,
                                    d->document,
                                    shapeController,
                                    canvas, 0,
                                    pasteAsText));
}

void KoTextEditor::deleteChar(bool previous, KUndo2Command *parent)
{
    if (isEditProtected()) {
        return;
    }

    KoShapeController *shapeController = KoTextDocument(d->document).shapeController();

    // Find out if we should track changes or not
//    KoChangeTracker *changeTracker = KoTextDocument(d->document).changeTracker();
//    bool trackChanges = false;
//    if (changeTracker && changeTracker->recordChanges()) {
//        trackChanges = true;
//    }

    if (previous) {
        if (!d->caret.hasSelection() && d->caret.block().blockFormat().hasProperty(KoParagraphStyle::HiddenByTable)) {
            movePosition(QTextCursor::PreviousCharacter);
            if (d->caret.block().length() <= 1) {
                movePosition(QTextCursor::NextCharacter);
            } else
                return; // it becomes just a cursor movement;
        }
    } else {
        if (!d->caret.hasSelection() && d->caret.block().length() > 1) {
            QTextCursor tmpCursor = d->caret;
            tmpCursor.movePosition(QTextCursor::NextCharacter);
            if (tmpCursor.block().blockFormat().hasProperty(KoParagraphStyle::HiddenByTable)) {
                movePosition(QTextCursor::NextCharacter);
                return; // it becomes just a cursor movement;
            }
        }
    }

    if (previous) {
        addCommand(new DeleteCommand(DeleteCommand::PreviousChar,
                                        d->document,
                                        shapeController, parent));
    } else {
        addCommand(new DeleteCommand(DeleteCommand::NextChar,
                                        d->document,
                                        shapeController, parent));
    }
}

void KoTextEditor::toggleListNumbering(bool numberingEnabled)
{
    if (isEditProtected()) {
        return;
    }

    addCommand(new ListItemNumberingCommand(block(), numberingEnabled));
    emit textFormatChanged();
}

void KoTextEditor::setListProperties(const KoListLevelProperties &llp,
                                     ChangeListFlags flags, KUndo2Command *parent)
{
    if (isEditProtected()) {
        return;
    }

    if (flags & AutoListStyle && d->caret.block().textList() == 0) {
        flags = MergeWithAdjacentList;
    }

    if (KoList *list = KoTextDocument(d->document).list(d->caret.block().textList())) {
        KoListStyle *listStyle = list->style();
        if (KoStyleManager *styleManager = KoTextDocument(d->document).styleManager()) {
            QList<KoParagraphStyle *> paragraphStyles = styleManager->paragraphStyles();
            foreach (KoParagraphStyle *paragraphStyle, paragraphStyles) {
                if (paragraphStyle->listStyle() == listStyle ||
                        (paragraphStyle->list() && paragraphStyle->list()->style() == listStyle)) {
                    flags = NoFlags;
                    break;
                }
            }
        }
    }

    addCommand(new ChangeListCommand(d->caret, llp, flags, parent));
    emit textFormatChanged();
}


int KoTextEditor::anchor() const
{
    return d->caret.anchor();
}

bool KoTextEditor::atBlockEnd() const
{
    return d->caret.atBlockEnd();
}

bool KoTextEditor::atBlockStart() const
{
    return d->caret.atBlockStart();
}

bool KoTextEditor::atEnd() const
{
    QTextCursor cursor(d->caret.document()->rootFrame()->lastCursorPosition());
    cursor.movePosition(QTextCursor::PreviousCharacter);
    QTextFrame *auxFrame = cursor.currentFrame();

    if (auxFrame->format().intProperty(KoText::SubFrameType) == KoText::AuxillaryFrameType) {
        //auxFrame really is the auxillary frame
        if (d->caret.position() == auxFrame->firstPosition() - 1) {
            return true;
        }
        return false;
    }
    return d->caret.atEnd();
}

bool KoTextEditor::atStart() const
{
    return d->caret.atStart();
}

QTextBlock KoTextEditor::block() const
{
    return d->caret.block();
}

int KoTextEditor::blockNumber() const
{
    return d->caret.blockNumber();
}

void KoTextEditor::clearSelection()
{
    d->caret.clearSelection();
}

int KoTextEditor::columnNumber() const
{
    return d->caret.columnNumber();
}

void KoTextEditor::deleteChar()
{
    if (isEditProtected()) {
        return;
    }

    if (!d->caret.hasSelection()) {
        if (d->caret.atEnd())
            return;

        // We alson need to refuse delete if we are at final pos in table cell
        if (QTextTable *table = d->caret.currentTable()) {
            QTextTableCell cell = table->cellAt(d->caret.position());
            if (d->caret.position() == cell.lastCursorPosition().position()) {
                return;
            }
        }

        // We also need to refuse delete if it will delete a note frame
        QTextCursor after(d->caret);
        after.movePosition(QTextCursor::NextCharacter);

        QTextFrame *beforeFrame = d->caret.currentFrame();
        while (qobject_cast<QTextTable *>(beforeFrame)) {
            beforeFrame = beforeFrame->parentFrame();
        }

        QTextFrame *afterFrame = after.currentFrame();
        while (qobject_cast<QTextTable *>(afterFrame)) {
            afterFrame = afterFrame->parentFrame();
        }
        if (beforeFrame != afterFrame) {
            return;
        }
    }


    deleteChar(false);

    emit cursorPositionChanged();
}

void KoTextEditor::deletePreviousChar()
{
    if (isEditProtected()) {
        return;
    }

    if (!d->caret.hasSelection()) {
        if (d->caret.atStart())
            return;

        // We also need to refuse delete if we are at first pos in table cell
        if (QTextTable *table = d->caret.currentTable()) {
            QTextTableCell cell = table->cellAt(d->caret.position());
            if (d->caret.position() == cell.firstCursorPosition().position()) {
                return;
            }
        }

        // We also need to refuse delete if it will delete a note frame
        QTextCursor after(d->caret);
        after.movePosition(QTextCursor::PreviousCharacter);

        QTextFrame *beforeFrame = d->caret.currentFrame();
        while (qobject_cast<QTextTable *>(beforeFrame)) {
            beforeFrame = beforeFrame->parentFrame();
        }

        QTextFrame *afterFrame = after.currentFrame();
        while (qobject_cast<QTextTable *>(afterFrame)) {
            afterFrame = afterFrame->parentFrame();
        }

        if (beforeFrame != afterFrame) {
            return;
        }
    }

    deleteChar(true);

    emit cursorPositionChanged();
}

QTextDocument *KoTextEditor::document() const
{
    return d->caret.document();
}

bool KoTextEditor::hasComplexSelection() const
{
    return d->caret.hasComplexSelection();
}

bool KoTextEditor::hasSelection() const
{
    return d->caret.hasSelection();
}


class ProtectionCheckVisitor : public KoTextVisitor
{
public:
    ProtectionCheckVisitor(const KoTextEditor *editor)
        : KoTextVisitor(const_cast<KoTextEditor *>(editor))
    {
    }

    // override super's implementation to not waste cpu cycles
    virtual void visitBlock(QTextBlock&, const QTextCursor &)
    {
    }

    virtual void nonVisit()
    {
        setAbortVisiting(true);
    }
};

bool KoTextEditor::isEditProtected(bool useCached) const
{
    ProtectionCheckVisitor visitor(this);

    if (useCached) {
        if (! d->editProtectionCached) {
            recursivelyVisitSelection(d->document->rootFrame()->begin(), visitor);
            d->editProtected = visitor.abortVisiting();
            d->editProtectionCached = true;
        }
        return d->editProtected;
    }
    d->editProtectionCached = false;
    recursivelyVisitSelection(d->document->rootFrame()->begin(), visitor);
    return visitor.abortVisiting();
}

void KoTextEditor::insertTable(int rows, int columns)
{
    if (isEditProtected() || rows <= 0 || columns <= 0) {
        return;
    }

    bool hasSelection = d->caret.hasSelection();
    if (!hasSelection) {
        d->updateState(KoTextEditor::Private::Custom, kundo2_i18n("Insert Table"));
    } else {
        KUndo2Command *topCommand = beginEditBlock(kundo2_i18n("Insert Table"));
        deleteChar(false, topCommand);
        d->caret.beginEditBlock();
    }

    QTextTableFormat tableFormat;

    tableFormat.setWidth(QTextLength(QTextLength::PercentageLength, 100));
    tableFormat.setProperty(KoTableStyle::CollapsingBorders, true);
    tableFormat.setMargin(5);

    KoChangeTracker *changeTracker = KoTextDocument(d->document).changeTracker();
    if (changeTracker && changeTracker->recordChanges()) {
        QTextCharFormat charFormat = d->caret.charFormat();
        QTextBlockFormat blockFormat = d->caret.blockFormat();
        KUndo2MagicString title = kundo2_i18n("Insert Table");

        int changeId;
        if (!d->caret.atBlockStart()) {
            changeId = changeTracker->mergeableId(KoGenChange::InsertChange, title, charFormat.intProperty(KoCharacterStyle::ChangeTrackerId));
        } else {
            changeId = changeTracker->mergeableId(KoGenChange::InsertChange, title, blockFormat.intProperty(KoCharacterStyle::ChangeTrackerId));
        }

        if (!changeId) {
            changeId = changeTracker->getInsertChangeId(title, 0);
        }

        tableFormat.setProperty(KoCharacterStyle::ChangeTrackerId, changeId);
    }

    QTextBlock currentBlock = d->caret.block();
    if (d->caret.position() != currentBlock.position()) {
        d->caret.insertBlock();
        currentBlock = d->caret.block();
    }

    QTextTable *table = d->caret.insertTable(rows, columns, tableFormat);

    // Get (and thus create) columnandrowstyle manager so it becomes part of undo
    // and not something that happens uncontrollably during layout
    KoTableColumnAndRowStyleManager::getManager(table);

    // 'Hide' the block before the table
    QTextBlockFormat blockFormat = currentBlock.blockFormat();
    QTextCursor cursor(currentBlock);
    blockFormat.setProperty(KoParagraphStyle::HiddenByTable, true);
    cursor.setBlockFormat(blockFormat);

    // Define the initial cell format
    QTextTableCellFormat format;
    KoTableCellStyle cellStyle;
    cellStyle.setEdge(KoBorder::TopBorder, KoBorder::BorderSolid, 2, QColor(Qt::black));
    cellStyle.setEdge(KoBorder::LeftBorder, KoBorder::BorderSolid, 2, QColor(Qt::black));
    cellStyle.setEdge(KoBorder::BottomBorder, KoBorder::BorderSolid, 2, QColor(Qt::black));
    cellStyle.setEdge(KoBorder::RightBorder, KoBorder::BorderSolid, 2, QColor(Qt::black));
    cellStyle.setPadding(5);
    cellStyle.applyStyle(format);

    // Apply formatting to all cells
    for (int row = 0; row < table->rows(); ++row) {
        for (int col = 0; col < table->columns(); ++col) {
            QTextTableCell cell = table->cellAt(row, col);
            cell.setFormat(format);
        }
    }

    if (hasSelection) {
        d->caret.endEditBlock();
        endEditBlock();
    } else {
        d->updateState(KoTextEditor::Private::NoOp);
    }

    emit cursorPositionChanged();
}

void KoTextEditor::insertTableRowAbove()
{
    if (isEditProtected()) {
        return;
    }

    QTextTable *table = d->caret.currentTable();
    if (table) {
        addCommand(new InsertTableRowCommand(this, table, false));
    }
}

void KoTextEditor::insertTableRowBelow()
{
    if (isEditProtected()) {
        return;
    }

    QTextTable *table = d->caret.currentTable();
    if (table) {
        addCommand(new InsertTableRowCommand(this, table, true));
    }
}

void KoTextEditor::insertTableColumnLeft()
{
    if (isEditProtected()) {
        return;
    }

    QTextTable *table = d->caret.currentTable();
    if (table) {
        addCommand(new InsertTableColumnCommand(this, table, false));
    }
}

void KoTextEditor::insertTableColumnRight()
{
    if (isEditProtected()) {
        return;
    }

    QTextTable *table = d->caret.currentTable();
    if (table) {
        addCommand(new InsertTableColumnCommand(this, table, true));
    }
}

void KoTextEditor::deleteTableColumn()
{
    if (isEditProtected()) {
        return;
    }

    QTextTable *table = d->caret.currentTable();
    if (table) {
        addCommand(new DeleteTableColumnCommand(this, table));
    }
}

void KoTextEditor::deleteTableRow()
{
    if (isEditProtected()) {
        return;
    }

    QTextTable *table = d->caret.currentTable();
    if (table) {
        addCommand(new DeleteTableRowCommand(this, table));
    }
}

void KoTextEditor::mergeTableCells()
{
    if (isEditProtected()) {
        return;
    }

    d->updateState(KoTextEditor::Private::Custom, kundo2_i18n("Merge Cells"));

    QTextTable *table = d->caret.currentTable();

    if (table) {
        table->mergeCells(d->caret);
    }

    d->updateState(KoTextEditor::Private::NoOp);
}

void KoTextEditor::splitTableCells()
{
    if (isEditProtected()) {
        return;
    }

    d->updateState(KoTextEditor::Private::Custom, kundo2_i18n("Split Cells"));

    QTextTable *table = d->caret.currentTable();

    if (table) {
        QTextTableCell cell = table->cellAt(d->caret);
        table->splitCell(cell.row(), cell.column(),  1, 1);
    }

    d->updateState(KoTextEditor::Private::NoOp);
}

void KoTextEditor::adjustTableColumnWidth(QTextTable *table, int column, qreal width, KUndo2Command *parentCommand)
{
    ResizeTableCommand *cmd = new ResizeTableCommand(table, true, column, width, parentCommand);

    addCommand(cmd);
}


void KoTextEditor::adjustTableRowHeight(QTextTable *table, int column, qreal height, KUndo2Command *parentCommand)
{
    ResizeTableCommand *cmd = new ResizeTableCommand(table, false, column, height, parentCommand);

    addCommand(cmd);
}

void KoTextEditor::adjustTableWidth(QTextTable *table, qreal dLeft, qreal dRight)
{
    d->updateState(KoTextEditor::Private::Custom, kundo2_i18n("Adjust Table Width"));
    d->caret.beginEditBlock();
    QTextTableFormat fmt = table->format();
    if (dLeft) {
        fmt.setLeftMargin(fmt.leftMargin() + dLeft);
    }
    if (dRight) {
        fmt.setRightMargin(fmt.rightMargin() + dRight);
    }
    table->setFormat(fmt);
    d->caret.endEditBlock();
    d->updateState(KoTextEditor::Private::NoOp);
}

void KoTextEditor::setTableBorderData(QTextTable *table, int row, int column,
         KoBorder::BorderSide cellSide, const KoBorder::BorderData &data)
{
    d->updateState(KoTextEditor::Private::Custom, kundo2_i18n("Change Border Formatting"));
    d->caret.beginEditBlock();
    QTextTableCell cell = table->cellAt(row, column);
    QTextCharFormat fmt = cell.format();
    KoBorder border = fmt.property(KoTableCellStyle::Borders).value<KoBorder>();

    border.setBorderData(cellSide, data);
    fmt.setProperty(KoTableCellStyle::Borders, QVariant::fromValue<KoBorder>(border));
    cell.setFormat(fmt);
    d->caret.endEditBlock();
    d->updateState(KoTextEditor::Private::NoOp);
}

KoInlineNote *KoTextEditor::insertFootNote()
{
    if (isEditProtected()) {
        return 0;
    }

    InsertNoteCommand *cmd = new InsertNoteCommand(KoInlineNote::Footnote, d->document);
    addCommand(cmd);

    emit cursorPositionChanged();
    return cmd->m_inlineNote;
}

KoInlineNote *KoTextEditor::insertEndNote()
{
    if (isEditProtected()) {
        return 0;
    }

    InsertNoteCommand *cmd = new InsertNoteCommand(KoInlineNote::Endnote, d->document);
    addCommand(cmd);

    emit cursorPositionChanged();
    return cmd->m_inlineNote;
}

void KoTextEditor::insertTableOfContents(KoTableOfContentsGeneratorInfo *info)
{
    if (isEditProtected()) {
        return;
    }

    bool hasSelection = d->caret.hasSelection();
    if (!hasSelection) {
        d->updateState(KoTextEditor::Private::Custom, kundo2_i18n("Insert Table Of Contents"));
    } else {
        KUndo2Command *topCommand = beginEditBlock(kundo2_i18n("Insert Table Of Contents"));
        deleteChar(false, topCommand);
        d->caret.beginEditBlock();
    }

    QTextBlockFormat tocFormat;
    KoTableOfContentsGeneratorInfo *newToCInfo = info->clone();
    QTextDocument *tocDocument = new QTextDocument();
    tocFormat.setProperty(KoParagraphStyle::TableOfContentsData, QVariant::fromValue<KoTableOfContentsGeneratorInfo *>(newToCInfo) );
    tocFormat.setProperty(KoParagraphStyle::GeneratedDocument, QVariant::fromValue<QTextDocument*>(tocDocument));

    //make sure we set up the textrangemanager on the subdocument as well
    KoTextDocument(tocDocument).setTextRangeManager(new KoTextRangeManager);

    KoChangeTracker *changeTracker = KoTextDocument(d->document).changeTracker();
    if (changeTracker && changeTracker->recordChanges()) {
        QTextCharFormat charFormat = d->caret.charFormat();
        QTextBlockFormat blockFormat = d->caret.blockFormat();
        KUndo2MagicString title = kundo2_i18n("Insert Table Of Contents");

        int changeId;
        if (!d->caret.atBlockStart()) {
            changeId = changeTracker->mergeableId(KoGenChange::InsertChange, title, charFormat.intProperty(KoCharacterStyle::ChangeTrackerId));
        } else {
            changeId = changeTracker->mergeableId(KoGenChange::InsertChange, title, blockFormat.intProperty(KoCharacterStyle::ChangeTrackerId));
        }

        if (!changeId) {
            changeId = changeTracker->getInsertChangeId(title, 0);
        }

        tocFormat.setProperty(KoCharacterStyle::ChangeTrackerId, changeId);
    }

    d->caret.insertBlock();
    d->caret.movePosition(QTextCursor::Left);
    d->caret.insertBlock(tocFormat);
    d->caret.movePosition(QTextCursor::Right);

    if (hasSelection) {
        d->caret.endEditBlock();
        endEditBlock();
    } else {
        d->updateState(KoTextEditor::Private::NoOp);
    }

    emit cursorPositionChanged();
}

void KoTextEditor::setTableOfContentsConfig(KoTableOfContentsGeneratorInfo *info, const QTextBlock &block)
{
    if (isEditProtected()) {
        return;
    }

    KoTableOfContentsGeneratorInfo *newToCInfo=info->clone();

    d->updateState(KoTextEditor::Private::Custom, kundo2_i18n("Modify Table Of Contents"));

    QTextCursor cursor(block);
    QTextBlockFormat tocBlockFormat=block.blockFormat();

    tocBlockFormat.setProperty(KoParagraphStyle::TableOfContentsData, QVariant::fromValue<KoTableOfContentsGeneratorInfo*>(newToCInfo) );
    cursor.setBlockFormat(tocBlockFormat);

    d->updateState(KoTextEditor::Private::NoOp);
    emit cursorPositionChanged();
    const_cast<QTextDocument *>(document())->markContentsDirty(document()->firstBlock().position(), 0);
}

void KoTextEditor::insertBibliography(KoBibliographyInfo *info)
{
    bool hasSelection = d->caret.hasSelection();
    if (!hasSelection) {
        d->updateState(KoTextEditor::Private::Custom, kundo2_i18n("Insert Bibliography"));
    } else {
        KUndo2Command *topCommand = beginEditBlock(kundo2_i18n("Insert Bibliography"));
        deleteChar(false, topCommand);
        d->caret.beginEditBlock();
    }

    QTextBlockFormat bibFormat;
    KoBibliographyInfo *newBibInfo = info->clone();
    QTextDocument *bibDocument = new QTextDocument();

    bibFormat.setProperty( KoParagraphStyle::BibliographyData, QVariant::fromValue<KoBibliographyInfo*>(newBibInfo));
    bibFormat.setProperty( KoParagraphStyle::GeneratedDocument, QVariant::fromValue<QTextDocument*>(bibDocument));

    //make sure we set up the textrangemanager on the subdocument as well
    KoTextDocument(bibDocument).setTextRangeManager(new KoTextRangeManager);

    KoChangeTracker *changeTracker = KoTextDocument(d->document).changeTracker();
    if (changeTracker && changeTracker->recordChanges()) {
        QTextCharFormat charFormat = d->caret.charFormat();
        QTextBlockFormat blockFormat = d->caret.blockFormat();
        KUndo2MagicString title = kundo2_i18n("Insert Bibliography");

        int changeId;
        if (!d->caret.atBlockStart()) {
            changeId = changeTracker->mergeableId(KoGenChange::InsertChange, title, charFormat.intProperty(KoCharacterStyle::ChangeTrackerId));
        } else {
            changeId = changeTracker->mergeableId(KoGenChange::InsertChange, title, blockFormat.intProperty(KoCharacterStyle::ChangeTrackerId));
        }

        if (!changeId) {
            changeId = changeTracker->getInsertChangeId(title, 0);
        }

        bibFormat.setProperty(KoCharacterStyle::ChangeTrackerId, changeId);
    }

    d->caret.insertBlock();
    d->caret.movePosition(QTextCursor::Left);
    d->caret.insertBlock(bibFormat);
    d->caret.movePosition(QTextCursor::Right);

    new BibliographyGenerator(bibDocument, block(), newBibInfo);

    if (hasSelection) {
        d->caret.endEditBlock();
        endEditBlock();
    } else {
        d->updateState(KoTextEditor::Private::NoOp);
    }

    emit cursorPositionChanged();
}

KoInlineCite *KoTextEditor::insertCitation()
{
    bool hasSelection = d->caret.hasSelection();
    if (!hasSelection) {
        d->updateState(KoTextEditor::Private::KeyPress, kundo2_i18n("Add Citation"));
    } else {
        KUndo2Command *topCommand = beginEditBlock(kundo2_i18n("Add Citation"));
        deleteChar(false, topCommand);
        d->caret.beginEditBlock();
    }

    KoInlineCite *cite = new KoInlineCite(KoInlineCite::Citation);
    KoInlineTextObjectManager *manager = KoTextDocument(d->document).inlineTextObjectManager();
    manager->insertInlineObject(d->caret,cite);

    if (hasSelection) {
        d->caret.endEditBlock();
        endEditBlock();
    } else {
        d->updateState(KoTextEditor::Private::NoOp);
    }

    return cite;
}

void KoTextEditor::insertText(const QString &text, const QString &hRef)
{
    if (isEditProtected()) {
        return;
    }

    bool hasSelection = d->caret.hasSelection();
    if (!hasSelection) {
        d->updateState(KoTextEditor::Private::KeyPress, kundo2_i18n("Typing"));
    } else {
        KUndo2Command *topCommand = beginEditBlock(kundo2_i18n("Typing"));
        deleteChar(false, topCommand);
        d->caret.beginEditBlock();
    }

    //first we make sure that we clear the inlineObject charProperty, if we have no selection
    if (!hasSelection && d->caret.charFormat().hasProperty(KoCharacterStyle::InlineInstanceId))
        d->clearCharFormatProperty(KoCharacterStyle::InlineInstanceId);

    int startPosition = d->caret.position();

    if (d->caret.blockFormat().hasProperty(KoParagraphStyle::HiddenByTable)) {
        d->newLine(0);
        startPosition = d->caret.position();
    }

    QTextCharFormat format = d->caret.charFormat();
    if (format.hasProperty(KoCharacterStyle::ChangeTrackerId)) {
        format.clearProperty(KoCharacterStyle::ChangeTrackerId);
    }
    static QRegExp urlScanner("\\S+://\\S+");
    if (!hRef.isEmpty()) {
        format.setAnchor(true);
        format.setProperty(KoCharacterStyle::AnchorType, KoCharacterStyle::Anchor);
        if ((urlScanner.indexIn(hRef)) == 0) {//web url
            format.setAnchorHref(hRef);
        } else {
            format.setAnchorHref("#"+hRef);
        }
    }
    d->caret.insertText(text, format);

    int endPosition = d->caret.position();

    //Mark the inserted text
    d->caret.setPosition(startPosition);
    d->caret.setPosition(endPosition, QTextCursor::KeepAnchor);

    registerTrackedChange(d->caret, KoGenChange::InsertChange, kundo2_i18n("Typing"), format, format, false);

    d->caret.clearSelection();

    if (hasSelection) {
        d->caret.endEditBlock();
        endEditBlock();
    }
    if (!hRef.isEmpty()) {
        format.setAnchor(false);
        format.clearProperty(KoCharacterStyle::Anchor);
        format.clearProperty(KoCharacterStyle::AnchorType);
        d->caret.setCharFormat(format);
    }
    emit cursorPositionChanged();
}

void KoTextEditor::insertHtml(const QString &html)
{
    if (isEditProtected()) {
        return;
    }

    // XXX: do the changetracking and everything!
    QTextBlock currentBlock = d->caret.block();
    d->caret.insertHtml(html);

    QList<QTextList *> pastedLists;
    KoList *currentPastedList = 0;
    while (currentBlock != d->caret.block()) {
        currentBlock = currentBlock.next();
        QTextList *currentTextList = currentBlock.textList();
        if(currentTextList && !pastedLists.contains(currentBlock.textList())) {
            KoListStyle *listStyle = KoTextDocument(d->document).styleManager()->defaultListStyle()->clone();
            listStyle->setName("");
            listStyle->setStyleId(0);
            currentPastedList = new KoList(d->document, listStyle);
            QTextListFormat currentTextListFormat = currentTextList->format();

            KoListLevelProperties levelProperty = listStyle->levelProperties(currentTextListFormat.indent());
            levelProperty.setStyle(static_cast<KoListStyle::Style>(currentTextListFormat.style()));
            levelProperty.setLevel(currentTextListFormat.indent());
            levelProperty.setListItemPrefix("");
            levelProperty.setListItemSuffix("");
            levelProperty.setListId((KoListStyle::ListIdType)currentTextList);
            listStyle->setLevelProperties(levelProperty);

            currentTextListFormat.setProperty(KoListStyle::Level, currentTextListFormat.indent());
            currentBlock.textList()->setFormat(currentTextListFormat);

            currentPastedList->updateStoredList(currentBlock);
            currentPastedList->setStyle(listStyle);

            pastedLists.append(currentBlock.textList());
        }
    }
}

bool KoTextEditor::movePosition(QTextCursor::MoveOperation operation, QTextCursor::MoveMode mode, int n)
{
    d->editProtectionCached = false;

    // We need protection against moving in and out of note areas
    QTextCursor after(d->caret);
    bool b = after.movePosition (operation, mode, n);

    QTextFrame *beforeFrame = d->caret.currentFrame();
    while (qobject_cast<QTextTable *>(beforeFrame)) {
        beforeFrame = beforeFrame->parentFrame();
    }

    QTextFrame *afterFrame = after.currentFrame();
    while (qobject_cast<QTextTable *>(afterFrame)) {
        afterFrame = afterFrame->parentFrame();
    }

    if (beforeFrame == afterFrame) {
        if (after.selectionEnd() == after.document()->characterCount() -1) {
            QTextCursor cursor(d->caret.document()->rootFrame()->lastCursorPosition());
            cursor.movePosition(QTextCursor::PreviousCharacter);
            QTextFrame *auxFrame = cursor.currentFrame();

            if (auxFrame->format().intProperty(KoText::SubFrameType) == KoText::AuxillaryFrameType) {
                if (operation == QTextCursor::End) {
                    d->caret.setPosition(auxFrame->firstPosition() - 1, mode);
                    emit cursorPositionChanged();
                    return true;
                }
                return false;
            }
        }
        d->caret = after;
        emit cursorPositionChanged();
        return b;
    }
    return false;
}

void KoTextEditor::newSection()
{
    if (isEditProtected()) {
        return;
    }

    NewSectionCommand *cmd = new NewSectionCommand(d->document);
    addCommand(cmd);
    emit cursorPositionChanged();
}

void KoTextEditor::splitSectionsStartings(int sectionIdToInsertBefore)
{
    if (isEditProtected()) {
        return;
    }
    addCommand(new SplitSectionsCommand(
        d->document,
        SplitSectionsCommand::Startings,
        sectionIdToInsertBefore));
    emit cursorPositionChanged();
}

void KoTextEditor::splitSectionsEndings(int sectionIdToInsertAfter)
{
    if (isEditProtected()) {
        return;
    }
    addCommand(new SplitSectionsCommand(
        d->document,
        SplitSectionsCommand::Endings,
        sectionIdToInsertAfter));
    emit cursorPositionChanged();
}

void KoTextEditor::renameSection(KoSection* section, const QString &newName)
{
    if (isEditProtected()) {
        return;
    }
    addCommand(new RenameSectionCommand(section, newName, document()));
}

void KoTextEditor::newLine()
{
    if (isEditProtected()) {
        return;
    }

    bool hasSelection = d->caret.hasSelection();
    if (!hasSelection) {
        d->updateState(KoTextEditor::Private::Custom, kundo2_i18n("New Paragraph"));
    } else {
        KUndo2Command *topCommand = beginEditBlock(kundo2_i18n("New Paragraph"));
        deleteChar(false, topCommand);
    }
    d->caret.beginEditBlock();

    d->newLine(0);

    d->caret.endEditBlock();

    if (hasSelection) {
        endEditBlock();
    } else {
        d->updateState(KoTextEditor::Private::NoOp);
    }

    emit cursorPositionChanged();
}

class WithinSelectionVisitor : public KoTextVisitor
{
public:
    WithinSelectionVisitor(KoTextEditor *editor, int position)
        : KoTextVisitor(editor)
        , m_position(position)
        , m_returnValue(false)
    {
    }

    virtual void visitBlock(QTextBlock &block, const QTextCursor &caret)
    {
        if (m_position >= qMax(block.position(), caret.selectionStart())
                    && m_position <= qMin(block.position() + block.length(), caret.selectionEnd())) {
            m_returnValue = true;
            setAbortVisiting(true);
        }
    }
    int m_position; //the position we are searching for
    bool m_returnValue; //if position is within the selection
};

bool KoTextEditor::isWithinSelection(int position) const
{
    // we know the visitor doesn't do anything with the texteditor so let's const cast
    // to have a more beautiful outer api
    WithinSelectionVisitor visitor(const_cast<KoTextEditor *>(this), position);

    recursivelyVisitSelection(d->document->rootFrame()->begin(), visitor);
    return visitor.m_returnValue;
}

int KoTextEditor::position() const
{
    return d->caret.position();
}

void KoTextEditor::select(QTextCursor::SelectionType selection)
{
    //TODO add selection of previous/next char, and option about hasSelection
    d->caret.select(selection);
}

QString KoTextEditor::selectedText() const
{
    return d->caret.selectedText();
}

QTextDocumentFragment KoTextEditor::selection() const
{
    return d->caret.selection();
}

int KoTextEditor::selectionEnd() const
{
    return d->caret.selectionEnd();
}

int KoTextEditor::selectionStart() const
{
    return d->caret.selectionStart();
}

void KoTextEditor::setPosition(int pos, QTextCursor::MoveMode mode)
{
    d->editProtectionCached = false;

    if (pos == d->caret.document()->characterCount() -1) {
        QTextCursor cursor(d->caret.document()->rootFrame()->lastCursorPosition());
        cursor.movePosition(QTextCursor::PreviousCharacter);
        QTextFrame *auxFrame = cursor.currentFrame();

        if (auxFrame->format().intProperty(KoText::SubFrameType) == KoText::AuxillaryFrameType) {
            return;
        }
    }

    if (mode == QTextCursor::MoveAnchor) {
        d->caret.setPosition (pos, mode);
        emit cursorPositionChanged();
    }

    // We need protection against moving in and out of note areas
    QTextCursor after(d->caret);
    after.setPosition (pos, mode);

    QTextFrame *beforeFrame = d->caret.currentFrame();
    while (qobject_cast<QTextTable *>(beforeFrame)) {
        beforeFrame = beforeFrame->parentFrame();
    }

    QTextFrame *afterFrame = after.currentFrame();
    while (qobject_cast<QTextTable *>(afterFrame)) {
        afterFrame = afterFrame->parentFrame();
    }

    if (beforeFrame == afterFrame) {
        d->caret = after;
        emit cursorPositionChanged();
    }
}

void KoTextEditor::setVisualNavigation(bool b)
{
    d->caret.setVisualNavigation (b);
}

bool KoTextEditor::visualNavigation() const
{
    return d->caret.visualNavigation();
}

const QTextFrame *KoTextEditor::currentFrame () const
{
    return d->caret.currentFrame();
}

const QTextList *KoTextEditor::currentList () const
{
    return d->caret.currentList();
}

const QTextTable *KoTextEditor::currentTable () const
{
    return d->caret.currentTable();
}

//have to include this because of Q_PRIVATE_SLOT
#include "moc_KoTextEditor.cpp"
