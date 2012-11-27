/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2008 Girish Ramakrishnan <girish@forwardbias.in>
 * Copyright (C) 2009 Pierre Stirnweiss <pstirnweiss@googlemail.com>
 * Copyright (C) 2010 Benjamin Port <port.benjamin@gmail.com>
 * Copyright (C) 2011 Pierre Ducroquet <pinaraf@pinaraf.info>
 * Copyright (C) 2011 Boudewijn Rempt <boud@kogmbh.com>
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

#include "KoTextWriter.h"

#include <KoTextWriter_p.h>

#include <commands/InsertDeleteChangesCommand.h>
#include <commands/RemoveDeleteChangesCommand.h>


KoTextWriter::KoTextWriter(KoShapeSavingContext &context, KoDocumentRdfBase *rdfData)
    : d(new Private(context))
{
    d->rdfData = rdfData;
    KoSharedSavingData *sharedData = context.sharedData(KOTEXT_SHARED_SAVING_ID);
    if (sharedData) {
        d->sharedData = dynamic_cast<KoTextSharedSavingData *>(sharedData);
    }

    if (!d->sharedData) {
        d->sharedData = new KoTextSharedSavingData();
        KoGenChanges *changes = new KoGenChanges();
        d->sharedData->setGenChanges(*changes);
        if (!sharedData) {
            context.addSharedData(KOTEXT_SHARED_SAVING_ID, d->sharedData);
        } else {
            kWarning(32500) << "A different type of sharedData was found under the" << KOTEXT_SHARED_SAVING_ID;
            Q_ASSERT(false);
        }
    }
}

KoTextWriter::~KoTextWriter()
{
    delete d;
}

void KoTextWriter::saveOdf(KoShapeSavingContext &context, KoDocumentRdfBase *rdfData, QTextDocument *document, int from, int to)
{
    InsertDeleteChangesCommand *insertCommand = new InsertDeleteChangesCommand(document);
    RemoveDeleteChangesCommand *removeCommand = new RemoveDeleteChangesCommand(document);

    KoChangeTracker *changeTracker = KoTextDocument(document).changeTracker();
    KoChangeTracker::ChangeSaveFormat changeSaveFormat = KoChangeTracker::UNKNOWN;
    if (changeTracker) {
        changeSaveFormat = changeTracker->saveFormat();
        if (!changeTracker->displayChanges() && (changeSaveFormat == KoChangeTracker::DELTAXML)) {
            KoTextDocument(document).textEditor()->instantlyExecuteCommand(insertCommand);
        }

        if (changeTracker->displayChanges() && (changeSaveFormat == KoChangeTracker::ODF_1_2)) {
            KoTextDocument(document).textEditor()->instantlyExecuteCommand(removeCommand);
        }
    }

    KoTextWriter writer(context, rdfData);
    writer.write(document, from, to);

    if (changeTracker) {
        changeSaveFormat = changeTracker->saveFormat();
        if (!changeTracker->displayChanges() && (changeSaveFormat == KoChangeTracker::DELTAXML)) {
            insertCommand->undo();
            delete insertCommand;
        }

        if (changeTracker->displayChanges() && (changeSaveFormat == KoChangeTracker::ODF_1_2)) {
            removeCommand->undo();
            delete removeCommand;
        }
    }
}

QString KoTextWriter::saveParagraphStyle(const QTextBlock &block, KoStyleManager *styleManager, KoShapeSavingContext &context)
{
    QTextBlockFormat blockFormat = block.blockFormat();
    QTextCharFormat charFormat = QTextCursor(block).blockCharFormat();
    return saveParagraphStyle(blockFormat, charFormat, styleManager, context);
}

QString KoTextWriter::saveParagraphStyle(const QTextBlockFormat &blockFormat, const QTextCharFormat &charFormat, KoStyleManager * styleManager, KoShapeSavingContext &context)
{
    KoParagraphStyle *defaultParagraphStyle = styleManager->defaultParagraphStyle();
    KoParagraphStyle *originalParagraphStyle = styleManager->paragraphStyle(blockFormat.intProperty(KoParagraphStyle::StyleId));
    if (!originalParagraphStyle)
        originalParagraphStyle = defaultParagraphStyle;

    QString generatedName;
    QString displayName = originalParagraphStyle->name();
    QString internalName = QString(QUrl::toPercentEncoding(displayName, "", " ")).replace('%', '_');

    // we'll convert the blockFormat to a KoParagraphStyle to check for local changes.
    KoParagraphStyle paragStyle(blockFormat, charFormat);
    if (paragStyle == (*originalParagraphStyle)) { // This is the real, unmodified character style.
        // TODO zachmann: this could use the name of the saved style without saving it again
        // therefore we would need to store that information in the saving context
        if (originalParagraphStyle != defaultParagraphStyle) {
            KoGenStyle style(KoGenStyle::ParagraphStyle, "paragraph");
            originalParagraphStyle->saveOdf(style, context);
            generatedName = context.mainStyles().insert(style, internalName, KoGenStyles::DontAddNumberToName);
        }
    } else { // There are manual changes... We'll have to store them then
        KoGenStyle style(KoGenStyle::ParagraphAutoStyle, "paragraph", internalName);
        if (context.isSet(KoShapeSavingContext::AutoStyleInStyleXml))
            style.setAutoStyleInStylesDotXml(true);
        if (originalParagraphStyle) {
            paragStyle.removeDuplicates(*originalParagraphStyle);
            paragStyle.setParentStyle(originalParagraphStyle);
        }
        paragStyle.saveOdf(style, context);
        generatedName = context.mainStyles().insert(style, "P");
    }
    return generatedName;
}

void KoTextWriter::write(const QTextDocument *document, int from, int to)
{
    d->document = const_cast<QTextDocument*>(document);
    d->styleManager = KoTextDocument(document).styleManager();
    d->changeTracker = KoTextDocument(document).changeTracker();

    d->saveAllChanges();


    QTextBlock fromblock = document->findBlock(from);
    QTextBlock toblock = document->findBlock(to);

    QTextCursor fromcursor(fromblock);

    QTextTable *currentTable = fromcursor.currentTable();
    QTextList *currentList = fromcursor.currentList();

    // NOTE even better would be if we create a new table/list out of multiple selected
    // tablecells/listitems that contain only the selected cells/items. But following
    // at least enables copying a whole list/table while still being able to copy/paste
    // only parts of the text within a list/table (see also bug 275990).
    if (currentTable || currentList) {
        if (from == 0 && to < 0) {
            // save everything means also save current table and list
            currentTable = 0;
            currentList = 0;
        } else {
            QTextCursor tocursor(toblock);
            //fromcursor.setPosition(from, QTextCursor::KeepAnchor);
            tocursor.setPosition(to, QTextCursor::KeepAnchor);

            if (!fromcursor.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor)) {
                fromcursor = QTextCursor();
            }
            if (!tocursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor)) {
                tocursor = QTextCursor();
            }

            // save the whole table if all cells are selected
            if (currentTable) {
                QTextTableCell fromcell = currentTable->cellAt(from);
                QTextTableCell tocell = currentTable->cellAt(to);
                if ((fromcursor.isNull() || fromcursor.currentTable() != currentTable) &&
                    (tocursor.isNull() || tocursor.currentTable() != currentTable) &&
                    fromcell.column() == 0 && fromcell.row() == 0 &&
                    tocell.column() == currentTable->columns()-1 && tocell.row() == currentTable->rows()-1
                ) {
                    currentTable = 0;
                }
            }

            // save the whole list if all list-items are selected
            if (currentList) {
                int fromindex = currentList->itemNumber(fromblock);
                int toindex = currentList->itemNumber(toblock);
                if ((fromcursor.isNull() || fromcursor.currentList() != currentList) &&
                    (tocursor.isNull() || tocursor.currentList() != currentList) &&
                    fromindex <= 0 && (toindex < 0 || toindex == currentList->count()-1)
                ) {
                    currentList = 0;
                }
            }
        }
    }

    QHash<QTextList *, QString> listStyles = d->saveListStyles(fromblock, to);
    d->globalFrom = from;
    d->globalTo = to;
    d->writeBlocks(const_cast<QTextDocument *>(document), from, to, listStyles, currentTable, currentList);
}
