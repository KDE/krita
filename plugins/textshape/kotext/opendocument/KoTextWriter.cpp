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
#include <KoStyleManager.h>
#include <KoParagraphStyle.h>
#include <KoTextDocument.h>
#include <KoShapeSavingContext.h>
#include <KoGenStyles.h>
#include <opendocument/KoTextSharedSavingData.h>

#include <QTextList>
#include <QTextTableCell>

#include "TextDebug.h"

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
        if (!sharedData) {
            context.addSharedData(KOTEXT_SHARED_SAVING_ID, d->sharedData);
        } else {
            warnText << "A different type of sharedData was found under the" << KOTEXT_SHARED_SAVING_ID;
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
    KoTextWriter writer(context, rdfData);
    writer.write(document, from, to);
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

    QTextBlock fromblock = document->findBlock(from);
    QTextBlock toblock = document->findBlock(to);

    QTextCursor fromcursor(fromblock);

    QTextList *currentList = fromcursor.currentList();

    // NOTE even better would be if we create a new list out of multiple selected
    // listitems that contain only the selected items. But following
    // at least enables copying a whole list while still being able to copy/paste
    // only parts of the text within a list (see also bug 275990).
    // NOTE this has been fixed for tables now, and it looks like the list code is seriously wrong
    // not just like the table code was, but more fundamentally as lists in qt is an orthogonal concept
    if (currentList) {
        if (from == 0 && to < 0) {
            // save everything means also save current table and list
            currentList = 0;
        } else {
            QTextCursor toCursor(toblock);
            toCursor.setPosition(to, QTextCursor::KeepAnchor);

            if (!fromcursor.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor)) {
                fromcursor = QTextCursor();
            }
            if (!toCursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor)) {
                toCursor = QTextCursor();
            }

            // save the whole list if all list-items are selected
            if (currentList) {
                int fromindex = currentList->itemNumber(fromblock);
                int toindex = currentList->itemNumber(toblock);
                if ((fromcursor.isNull() || fromcursor.currentList() != currentList) &&
                    (toCursor.isNull() || toCursor.currentList() != currentList) &&
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
    d->writeBlocks(const_cast<QTextDocument *>(document), from, to, listStyles, 0, currentList);
}
