/*
 *  Copyright (c) 2010 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2014 Denis Kuplyakov <dener.kup@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "KoTextWriter_p.h"

#include <KoSectionUtils.h>
#include <KoSectionEnd.h>
#include <KoList.h>
#include <KoElementReference.h>
#include <KoTextRangeManager.h>
#include <KoStyleManager.h>
#include <KoParagraphStyle.h>
#include <KoListLevelProperties.h>
#include <KoTableCellStyle.h>
#include <KoTableStyle.h>
#include <KoTextBlockData.h>
#include <KoTextDocument.h>
#include <KoTextInlineRdf.h>
#include <KoSection.h>
#include <KoTextMeta.h>
#include <KoShapeSavingContext.h>
#include <KoXmlWriter.h>
#include <KoGenStyles.h>
#include <KoXmlNS.h>
#include <KoTableColumnAndRowStyleManager.h>
#include <KoTableColumnStyle.h>
#include <opendocument/KoTextSharedSavingData.h>
#include <KoTableOfContentsGeneratorInfo.h>
#include <KoBibliographyInfo.h>
#include <KoTableRowStyle.h>
#include <KoInlineTextObjectManager.h>
#include <KoVariable.h>

#include "TextDebug.h"

#include <QTextTable>

// A convenience function to get a listId from a list-format
static KoListStyle::ListIdType ListId(const QTextListFormat &format)
{
    KoListStyle::ListIdType listId;

    if (sizeof(KoListStyle::ListIdType) == sizeof(uint))
        listId = format.property(KoListStyle::ListId).toUInt();
    else
        listId = format.property(KoListStyle::ListId).toULongLong();

    return listId;
}

typedef QPair<QString, QString> Attribute;


KoTextWriter::Private::Private(KoShapeSavingContext &context)
    : rdfData(0)
    , sharedData(0)
    , styleManager(0)
    , document(0)
    , writer(0)
    , context(context)
{
    currentPairedInlineObjectsStack = new QStack<KoInlineObject*>();
    writer = &context.xmlWriter();
}

void KoTextWriter::Private::writeBlocks(QTextDocument *document, int from, int to, QHash<QTextList *, QString> &listStyles, QTextTable *currentTable, QTextList *currentList)
{
    pairedInlineObjectsStackStack.push(currentPairedInlineObjectsStack);
    currentPairedInlineObjectsStack = new QStack<KoInlineObject*>();
    QTextBlock block = document->findBlock(from);

    // Here we are going to detect all sections that
    // are positioned entirely inside selection.
    // They will stay untouched, and others will be omitted.

    // So we are using stack to detect them, by going through
    // the selection and finding open/close pairs.
    QSet<QString> entireWithinSectionNames;
    QStack<QString> sectionNamesStack;
    QTextCursor cur(document);
    cur.setPosition(from);
    while (to == -1 || cur.position() <= to) {
        if (cur.block().position() >= from) { // Begin of the block is inside selection.
            foreach (const KoSection *sec, KoSectionUtils::sectionStartings(cur.blockFormat())) {
                sectionNamesStack.push_back(sec->name());
            }
        }

        if (to == -1 || cur.block().position() + cur.block().length() - 1 <= to) { // End of the block is inside selection.
            foreach (const KoSectionEnd *sec, KoSectionUtils::sectionEndings(cur.blockFormat())) {
                if (!sectionNamesStack.empty() && sectionNamesStack.top() == sec->name()) {
                    sectionNamesStack.pop();
                    entireWithinSectionNames.insert(sec->name());
                }
            }
        }

        if (!KoSectionUtils::getNextBlock(cur)) {
            break;
        }
    }

    while (block.isValid() && ((to == -1) || (block.position() <= to))) {

        QTextCursor cursor(block);

        int frameType = cursor.currentFrame()->format().intProperty(KoText::SubFrameType);
        if (frameType == KoText::AuxillaryFrameType) {
            break; // we've reached the "end" (end/footnotes saved by themselves)
                   // note how NoteFrameType passes through here so the notes can
                   // call writeBlocks to save their contents.
        }

        QTextBlockFormat format = block.blockFormat();

        foreach (const KoSection *section, KoSectionUtils::sectionStartings(format)) {
            // We are writing in only sections, that are completely inside selection.
            if (entireWithinSectionNames.contains(section->name())) {
                section->saveOdf(context);
            }
        }

        if (format.hasProperty(KoParagraphStyle::HiddenByTable)) {
            block = block.next();
            continue;
        }
        if (format.hasProperty(KoParagraphStyle::TableOfContentsData)) {
            saveTableOfContents(document, listStyles, block);
            block = block.next();
            continue;
        }
        if (format.hasProperty(KoParagraphStyle::BibliographyData)) {
            saveBibliography(document, listStyles, block);
            block = block.next();
            continue;
        }

        if (cursor.currentTable() && cursor.currentTable() != currentTable) {
            // Call the code to save the table....
            saveTable(cursor.currentTable(), listStyles, from, to);
            // We skip to the end of the table.
            block = cursor.currentTable()->lastCursorPosition().block();
            block = block.next();
            continue;
        }

        if (cursor.currentList() && cursor.currentList() != currentList) {
            int previousBlockNumber = block.blockNumber();
            block = saveList(block, listStyles, 1, currentTable);
            int blockNumberToProcess = block.blockNumber();
            if (blockNumberToProcess != previousBlockNumber)
                continue;
        }

        saveParagraph(block, from, to);

        foreach (const KoSectionEnd *sectionEnd, KoSectionUtils::sectionEndings(format)) {
            // We are writing in only sections, that are completely inside selection.
            if (entireWithinSectionNames.contains(sectionEnd->name())) {
                sectionEnd->saveOdf(context);
            }
        }

        block = block.next();
    } // while

    Q_ASSERT(!pairedInlineObjectsStackStack.isEmpty());
    delete currentPairedInlineObjectsStack;
    currentPairedInlineObjectsStack = pairedInlineObjectsStackStack.pop();
}


QHash<QTextList *, QString> KoTextWriter::Private::saveListStyles(QTextBlock block, int to)
{
    QHash<KoList *, QString> generatedLists;
    QHash<QTextList *, QString> listStyles;

    for (;block.isValid() && ((to == -1) || (block.position() < to)); block = block.next()) {
        QTextList *textList = block.textList();
        if (!textList)
            continue;
        KoListStyle::ListIdType listId = ListId(textList->format());
        if (KoList *list = KoTextDocument(document).list(listId)) {
            if (generatedLists.contains(list)) {
                if (!listStyles.contains(textList))
                    listStyles.insert(textList, generatedLists.value(list));
                continue;
            }
            KoListStyle *listStyle = list->style();
            if (listStyle && listStyle->isOulineStyle()) {
                continue;
            }
            bool automatic = listStyle->styleId() == 0;
            KoGenStyle style(automatic ? KoGenStyle::ListAutoStyle : KoGenStyle::ListStyle);
            if (automatic && context.isSet(KoShapeSavingContext::AutoStyleInStyleXml))
                style.setAutoStyleInStylesDotXml(true);
            listStyle->saveOdf(style, context);
            QString generatedName = context.mainStyles().insert(style, listStyle->name(), listStyle->isNumberingStyle() ? KoGenStyles::AllowDuplicates : KoGenStyles::DontAddNumberToName);
            listStyles[textList] = generatedName;
            generatedLists.insert(list, generatedName);
        } else {
            if (listStyles.contains(textList))
                continue;
            KoListLevelProperties llp = KoListLevelProperties::fromTextList(textList);
            KoGenStyle style(KoGenStyle::ListAutoStyle);
            if (context.isSet(KoShapeSavingContext::AutoStyleInStyleXml))
                style.setAutoStyleInStylesDotXml(true);
            KoListStyle listStyle;
            listStyle.setLevelProperties(llp);
            if (listStyle.isOulineStyle()) {
                continue;
            }
            listStyle.saveOdf(style, context);
            QString generatedName = context.mainStyles().insert(style, listStyle.name());
            listStyles[textList] = generatedName;
        }
    }
    return listStyles;
}

//---------------------------- PRIVATE -----------------------------------------------------------

void KoTextWriter::Private::openTagRegion(ElementType elementType, TagInformation& tagInformation)
{
    //debugText << "tag:" << tagInformation.name() << openedTagStack.size();
    if (tagInformation.name()) {
    writer->startElement(tagInformation.name(), elementType != ParagraphOrHeader);
    foreach (const Attribute &attribute, tagInformation.attributes()) {
        writer->addAttribute(attribute.first.toLocal8Bit(), attribute.second);
    }
    }
    openedTagStack.push(tagInformation.name());
    //debugText << "stack" << openedTagStack.size();
}

void KoTextWriter::Private::closeTagRegion()
{
    // the tag needs to be closed even if there is no change tracking
    //debugText << "stack" << openedTagStack.size();
    const char *tagName = openedTagStack.pop();
    //debugText << "tag:" << tagName << openedTagStack.size();
    if (tagName) {
        writer->endElement(); // close the tag
    }
}

QString KoTextWriter::Private::saveParagraphStyle(const QTextBlock &block)
{
    return KoTextWriter::saveParagraphStyle(block, styleManager, context);
}

QString KoTextWriter::Private::saveParagraphStyle(const QTextBlockFormat &blockFormat, const QTextCharFormat &charFormat)
{
    return KoTextWriter::saveParagraphStyle(blockFormat, charFormat, styleManager, context);
}

QString KoTextWriter::Private::saveCharacterStyle(const QTextCharFormat &charFormat, const QTextCharFormat &blockCharFormat)
{
    KoCharacterStyle *defaultCharStyle = styleManager->defaultCharacterStyle();

    KoCharacterStyle *originalCharStyle = styleManager->characterStyle(charFormat.intProperty(KoCharacterStyle::StyleId));
    if (!originalCharStyle)
        originalCharStyle = defaultCharStyle;

    QString generatedName;
    QString displayName = originalCharStyle->name();
    QString internalName = QString(QUrl::toPercentEncoding(displayName, "", " ")).replace('%', '_');

    KoCharacterStyle *autoStyle = originalCharStyle->autoStyle(charFormat, blockCharFormat);

    if (autoStyle->isEmpty()) { // This is the real, unmodified character style.
        if (originalCharStyle != defaultCharStyle) {
            KoGenStyle style(KoGenStyle::TextStyle, "text");
            originalCharStyle->saveOdf(style);
            generatedName = context.mainStyles().insert(style, internalName, KoGenStyles::DontAddNumberToName);
        }
    } else { // There are manual changes... We'll have to store them then
        KoGenStyle style(KoGenStyle::TextAutoStyle, "text", originalCharStyle != defaultCharStyle ? internalName : "" /*parent*/);
        if (context.isSet(KoShapeSavingContext::AutoStyleInStyleXml))
            style.setAutoStyleInStylesDotXml(true);

        autoStyle->saveOdf(style);
        generatedName = context.mainStyles().insert(style, "T");
    }

    delete autoStyle;
    return generatedName;
}

QString KoTextWriter::Private::saveTableStyle(const QTextTable& table)
{
    KoTableStyle *originalTableStyle = styleManager->tableStyle(table.format().intProperty(KoTableStyle::StyleId));
    QString generatedName;
    QString internalName;
    if (originalTableStyle)
    {
        internalName = QString(QUrl::toPercentEncoding(originalTableStyle->name(), "", " ")).replace('%', '_');
    }
    KoTableStyle tableStyle(table.format());
    if ((originalTableStyle) && (*originalTableStyle == tableStyle)) { // This is the real unmodified table style
        KoGenStyle style(KoGenStyle::TableStyle, "table");
        originalTableStyle->saveOdf(style);
        generatedName = context.mainStyles().insert(style, internalName, KoGenStyles::DontAddNumberToName);
    } else { // There are manual changes... We'll have to store them then
        KoGenStyle style(KoGenStyle::TableAutoStyle, "table", internalName);
        if (context.isSet(KoShapeSavingContext::AutoStyleInStyleXml))
            style.setAutoStyleInStylesDotXml(true);
        if (originalTableStyle)
            tableStyle.removeDuplicates(*originalTableStyle);
        if (!tableStyle.isEmpty()) {
            tableStyle.saveOdf(style);
            generatedName = context.mainStyles().insert(style, "Table");
        }
    }
    return generatedName;
}

QString KoTextWriter::Private::saveTableColumnStyle(const KoTableColumnStyle& tableColumnStyle, int columnNumber, const QString& tableStyleName)
{
    // 26*26 columns should be enough for everyone
    QString columnName = QChar('A' + int(columnNumber % 26));
    if (columnNumber > 25)
        columnName.prepend(QChar('A' + int(columnNumber/26)));
    QString generatedName = tableStyleName + '.' + columnName;

    KoGenStyle style(KoGenStyle::TableColumnAutoStyle, "table-column");

    if (context.isSet(KoShapeSavingContext::AutoStyleInStyleXml))
        style.setAutoStyleInStylesDotXml(true);

    tableColumnStyle.saveOdf(style);
    generatedName = context.mainStyles().insert(style, generatedName, KoGenStyles::DontAddNumberToName);
    return generatedName;
}

QString KoTextWriter::Private::saveTableRowStyle(const KoTableRowStyle& tableRowStyle, int rowNumber, const QString& tableStyleName)
{
    QString generatedName = tableStyleName + '.' + QString::number(rowNumber + 1);

    KoGenStyle style(KoGenStyle::TableRowAutoStyle, "table-row");

    if (context.isSet(KoShapeSavingContext::AutoStyleInStyleXml))
        style.setAutoStyleInStylesDotXml(true);

    tableRowStyle.saveOdf(style);
    generatedName = context.mainStyles().insert(style, generatedName, KoGenStyles::DontAddNumberToName);
    return generatedName;
}

QString KoTextWriter::Private::saveTableCellStyle(const QTextTableCellFormat& cellFormat, int columnNumber, const QString& tableStyleName)
{
    // 26*26 columns should be enough for everyone
    QString columnName = QChar('A' + int(columnNumber % 26));
    if (columnNumber > 25)
        columnName.prepend(QChar('A' + int(columnNumber/26)));
    QString generatedName = tableStyleName + '.' + columnName;

    KoGenStyle style(KoGenStyle::TableCellAutoStyle, "table-cell");

    if (context.isSet(KoShapeSavingContext::AutoStyleInStyleXml))
        style.setAutoStyleInStylesDotXml(true);

    KoTableCellStyle cellStyle(cellFormat);
    cellStyle.saveOdf(style, context);
    generatedName = context.mainStyles().insert(style, generatedName);
    return generatedName;
}

void KoTextWriter::Private::saveInlineRdf(KoTextInlineRdf* rdf, TagInformation* tagInfos)
{
    QBuffer rdfXmlData;
    KoXmlWriter rdfXmlWriter(&rdfXmlData);
    rdfXmlWriter.startDocument("rdf");
    rdfXmlWriter.startElement("rdf");
    rdf->saveOdf(context, &rdfXmlWriter);
    rdfXmlWriter.endElement();
    rdfXmlWriter.endDocument();

    KoXmlDocument xmlReader;
    xmlReader.setContent(rdfXmlData.data(), true);
    KoXmlElement mainElement = xmlReader.documentElement();
    foreach (const Attribute &attributeNameNS, mainElement.attributeFullNames()) {
        QString attributeName = QString("%1:%2").arg(KoXmlNS::nsURI2NS(attributeNameNS.first))
                                                .arg(attributeNameNS.second);
        if (attributeName.startsWith(':'))
            attributeName.prepend("xml");
        tagInfos->addAttribute(attributeName, mainElement.attribute(attributeNameNS.second));
    }
}

/*
Note on saving textranges:
Start and end tags of textranges can appear on cursor positions in a text block.
in front of the first text element, between the elements, or behind the last.
A textblock is composed of no, one or many text fragments.
If there is no fragment at all, the only possible cursor position is 0 (relative to the
begin of the block).
Example:  ([] marks a block, {} a fragment)
Three blocks, first with text fragments {AB} {C}, second empty, last with {DEF}.
Possible positions are: [|{A|B}|{C}|]  [|]  [|{D|E|F}|]

Start tags are ideally written in front of the content they are tagging,
not behind the previous content. That way tags which are at the very begin
of the complete document do not need special handling.
End tags are ideally written directly behind the content, and not in front of
the next content. That way end tags which are at the end of the complete document
do not need special handling.
Next there is the case of start tags which are at the final position of a text block:
the content they belong to includes the block end/border, so they need to be
written at the place of the last position.
Then there is the case of end tags at the first position of a text block:
the content they belong to includes the block start/border, so they need to be
written at the place of the first position.
Example:   (< marks a start tag, > marks an end tag)
[|>{<A><B>}|{<C>}|<]  [|><]  [|>{<D>|<E>|<F>}|<]
*/
void KoTextWriter::Private::saveParagraph(const QTextBlock &block, int from, int to)
{
    QTextCursor cursor(block);
    QTextBlockFormat blockFormat = block.blockFormat();
    const int outlineLevel = blockFormat.intProperty(KoParagraphStyle::OutlineLevel);

    TagInformation blockTagInformation;
    if (outlineLevel > 0) {
        blockTagInformation.setTagName("text:h");
        blockTagInformation.addAttribute("text:outline-level", outlineLevel);
        if (blockFormat.boolProperty(KoParagraphStyle::IsListHeader) || blockFormat.boolProperty(KoParagraphStyle::UnnumberedListItem)) {
            blockTagInformation.addAttribute("text:is-list-header", "true");
        }
    } else {
        blockTagInformation.setTagName("text:p");
    }

    openTagRegion(KoTextWriter::Private::ParagraphOrHeader, blockTagInformation);

    QString styleName = saveParagraphStyle(block);
    if (!styleName.isEmpty())
        writer->addAttribute("text:style-name", styleName);

    KoElementReference xmlid;
    xmlid.invalidate();

    QTextBlock currentBlock = block;
    KoTextBlockData blockData(currentBlock);
    if (blockData.saveXmlID()) {
        xmlid = context.xmlid(&blockData);
        xmlid.saveOdf(writer, KoElementReference::TextId);
    }
    // Write the fragments and their formats
    QTextCharFormat blockCharFormat = cursor.blockCharFormat();
    QTextCharFormat previousCharFormat;
    QTextBlock::iterator it;
    if (KoTextInlineRdf* inlineRdf = KoTextInlineRdf::tryToGetInlineRdf(blockCharFormat)) {
        // Write xml:id here for Rdf
        debugText << "have inline rdf xmlid:" << inlineRdf->xmlId() << "active xml id" << xmlid.toString();
        inlineRdf->saveOdf(context, writer, xmlid);
    }

    const KoTextRangeManager *textRangeManager = KoTextDocument(block.document()).textRangeManager();

    if (textRangeManager) {
        // write tags for ranges which end at the first position of the block
        const QHash<int, KoTextRange *> endingTextRangesAtStart =
            textRangeManager->textRangesChangingWithin(block.document(), block.position(), block.position(), globalFrom, globalTo);
        foreach (const KoTextRange *range, endingTextRangesAtStart) {
            range->saveOdf(context, block.position(), KoTextRange::EndTag);
        }
    }

    QString previousFragmentLink;
    // stores the end position of the last fragment, is position of the block without any fragment at all
    int lastEndPosition = block.position();
    for (it = block.begin(); !(it.atEnd()); ++it) {
        QTextFragment currentFragment = it.fragment();
        const int fragmentStart = currentFragment.position();
        const int fragmentEnd = fragmentStart + currentFragment.length();
        if (to != -1 && fragmentStart >= to)
            break;
        if (currentFragment.isValid()) {
            QTextCharFormat charFormat = currentFragment.charFormat();

            if ((!previousFragmentLink.isEmpty()) && (charFormat.anchorHref() != previousFragmentLink || !charFormat.isAnchor())) {
                // Close the current text:a
                closeTagRegion();
                previousFragmentLink.clear();
            }

            if (charFormat.isAnchor() && charFormat.anchorHref() != previousFragmentLink) {
                // Open a text:a
                previousFragmentLink = charFormat.anchorHref();
                TagInformation linkTagInformation;

                if (charFormat.intProperty(KoCharacterStyle::AnchorType) == KoCharacterStyle::Bookmark) {
                    linkTagInformation.setTagName("text:bookmark-ref");
                    QString href = previousFragmentLink.right(previousFragmentLink.size()-1);
                    linkTagInformation.addAttribute("text:ref-name", href);
                    //linkTagInformation.addAttribute("text:ref-format", add the style of the ref here);
                } else {
                    linkTagInformation.setTagName("text:a");
                    linkTagInformation.addAttribute("xlink:type", "simple");
                    linkTagInformation.addAttribute("xlink:href", charFormat.anchorHref());
                }
                if (KoTextInlineRdf* inlineRdf = KoTextInlineRdf::tryToGetInlineRdf(charFormat)) {
                    // Write xml:id here for Rdf
                    debugText << "have inline rdf xmlid:" << inlineRdf->xmlId();
                    saveInlineRdf(inlineRdf, &linkTagInformation);
                }
                openTagRegion(KoTextWriter::Private::Span, linkTagInformation);
            }

            KoInlineTextObjectManager *textObjectManager = KoTextDocument(document).inlineTextObjectManager();
            KoInlineObject *inlineObject = textObjectManager ? textObjectManager->inlineTextObject(charFormat) : 0;
            // If we are in an inline object
            if (currentFragment.length() == 1 && inlineObject
                    && currentFragment.text()[0].unicode() == QChar::ObjectReplacementCharacter) {
                bool saveInlineObject = true;

                if (KoTextMeta* z = dynamic_cast<KoTextMeta*>(inlineObject)) {
                    if (z->position() < from) {
                        //
                        // This <text:meta> starts before the selection, default
                        // to not saving it with special cases to allow saving
                        //
                        saveInlineObject = false;
                        if (z->type() == KoTextMeta::StartBookmark) {
                            if (z->endBookmark()->position() > from) {
                                //
                                // They have selected something starting after the
                                // <text:meta> opening but before the </text:meta>
                                //
                                saveInlineObject = true;
                            }
                        }
                    }
                }

        // get all text ranges which start before this inline object
        // or end directly after it (+1 to last position for that)
                const QHash<int, KoTextRange *> textRanges = textRangeManager ?
                        textRangeManager->textRangesChangingWithin(block.document(), currentFragment.position(), currentFragment.position()+1,
                        globalFrom, (globalTo==-1)?-1:globalTo+1) : QHash<int, KoTextRange *>();
        // get all text ranges which start before this
        const QList<KoTextRange *> textRangesBefore = textRanges.values(currentFragment.position());
        // write tags for ranges which start before this content or at positioned at it
        foreach (const KoTextRange *range, textRangesBefore) {
            range->saveOdf(context, currentFragment.position(), KoTextRange::StartTag);
        }

                bool saveSpan = dynamic_cast<KoVariable*>(inlineObject) != 0;

                if (saveSpan) {
                    QString styleName = saveCharacterStyle(charFormat, blockCharFormat);
                    if (!styleName.isEmpty()) {
                        writer->startElement("text:span", false);
                        writer->addAttribute("text:style-name", styleName);
                    }
                    else {
                        saveSpan = false;
                    }
                }

                if (saveInlineObject) {
                    inlineObject->saveOdf(context);
                }

                if (saveSpan) {
                    writer->endElement();
                }

                // write tags for ranges which end after this inline object
                const QList<KoTextRange *> textRangesAfter = textRanges.values(currentFragment.position()+1);
                foreach (const KoTextRange *range, textRangesAfter) {
                    range->saveOdf(context, currentFragment.position()+1, KoTextRange::EndTag);
                }

                //
                // Track the end marker for matched pairs so we produce valid
                // ODF
                //
                if (KoTextMeta* z = dynamic_cast<KoTextMeta*>(inlineObject)) {
                    debugText << "found kometa, type:" << z->type();
                    if (z->type() == KoTextMeta::StartBookmark)
                        currentPairedInlineObjectsStack->push(z->endBookmark());
                    if (z->type() == KoTextMeta::EndBookmark
                            && !currentPairedInlineObjectsStack->isEmpty())
                        currentPairedInlineObjectsStack->pop();
                }/* else if (KoBookmark* z = dynamic_cast<KoBookmark*>(inlineObject)) {
                    if (z->type() == KoBookmark::StartBookmark)
                        currentPairedInlineObjectsStack->push(z->endBookmark());
                    if (z->type() == KoBookmark::EndBookmark
                            && !currentPairedInlineObjectsStack->isEmpty())
                        currentPairedInlineObjectsStack->pop();
                }*/
            } else {
                // Normal block, easier to handle
                QString styleName = saveCharacterStyle(charFormat, blockCharFormat);

                TagInformation fragmentTagInformation;
                if (!styleName.isEmpty() /*&& !identical*/) {
                    fragmentTagInformation.setTagName("text:span");
                    fragmentTagInformation.addAttribute("text:style-name", styleName);
                }

                openTagRegion(KoTextWriter::Private::Span, fragmentTagInformation);

                QString text = currentFragment.text();
                int spanFrom = fragmentStart >= from ? fragmentStart : from;
                int spanTo = to == -1 ? fragmentEnd : (fragmentEnd > to ? to : fragmentEnd);
                // get all text ranges which change within this span
                // or end directly after it (+1 to last position to include those)
                const QHash<int, KoTextRange *> textRanges = textRangeManager ?
                    textRangeManager->textRangesChangingWithin(block.document(), spanFrom, spanTo, globalFrom, (globalTo==-1)?-1:globalTo+1) :
                    QHash<int, KoTextRange *>();
                // avoid mid, if possible
                if (spanFrom != fragmentStart || spanTo != fragmentEnd || !textRanges.isEmpty()) {
                    if (textRanges.isEmpty()) {
                        writer->addTextSpan(text.mid(spanFrom - fragmentStart, spanTo - spanFrom));
                    } else {
                        // split the fragment into subspans at the points of range starts/ends
                        QList<int> subSpanTos = textRanges.uniqueKeys();
                        qSort(subSpanTos);
                        // ensure last subSpanTo to be at the end
                        if (subSpanTos.last() != spanTo) {
                            subSpanTos.append(spanTo);
                        }
                        // spanFrom should not need to be included
                        if (subSpanTos.first() == spanFrom) {
                            subSpanTos.removeOne(spanFrom);
                        }
                        int subSpanFrom = spanFrom;
                        // for all subspans
                        foreach (int subSpanTo, subSpanTos) {
                            // write tags for text ranges which start before this subspan or are positioned at it
                            const QList<KoTextRange *> textRangesStartingBefore = textRanges.values(subSpanFrom);
                            foreach (const KoTextRange *range, textRangesStartingBefore) {
                                range->saveOdf(context, subSpanFrom, KoTextRange::StartTag);
                            }

                            // write subspan content
                            writer->addTextSpan(text.mid(subSpanFrom - fragmentStart, subSpanTo - subSpanFrom));

                            // write tags for text ranges which end behind this subspan
                            const QList<KoTextRange *> textRangesEndingBehind = textRanges.values(subSpanTo);
                            foreach (const KoTextRange *range, textRangesEndingBehind) {
                                range->saveOdf(context, subSpanTo, KoTextRange::EndTag);
                            }

                            subSpanFrom = subSpanTo;
                        }
                    }
                } else {
                    writer->addTextSpan(text);
                }

                closeTagRegion();
            } // if (inlineObject)

            previousCharFormat = charFormat;
            lastEndPosition = fragmentEnd;
        }
    }

    if (!previousFragmentLink.isEmpty()) {
        writer->endElement();
    }

    if (it.atEnd() && textRangeManager && ((to == -1) || (lastEndPosition <= to))) {
        // write tags for ranges which start at the last position of the block,
        // i.e. at the position after the last (text) fragment
        const QHash<int, KoTextRange *> startingTextRangesAtEnd =
            textRangeManager->textRangesChangingWithin(block.document(), lastEndPosition, lastEndPosition, globalFrom, globalTo);
        foreach (const KoTextRange *range, startingTextRangesAtEnd) {
            range->saveOdf(context, lastEndPosition, KoTextRange::StartTag);
        }
    }

    QString text = block.text();
    if (text.length() == 0 || text.at(text.length()-1) == QChar(0x2028)) {
        if (block.blockFormat().hasProperty(KoParagraphStyle::EndCharStyle)) {
            QVariant v = block.blockFormat().property(KoParagraphStyle::EndCharStyle);
            QSharedPointer<KoCharacterStyle> endCharStyle = v.value< QSharedPointer<KoCharacterStyle> >();
            if (!endCharStyle.isNull()) {
                QTextCharFormat charFormat;
                endCharStyle->applyStyle(charFormat);

                QString styleName = saveCharacterStyle(charFormat, blockCharFormat);

                if (!styleName.isEmpty()) {
                    writer->startElement("text:span", false);
                    writer->addAttribute("text:style-name", styleName);
                    writer->endElement();
                }
            }
        }
     }

    if (to !=-1 && to < block.position() + block.length()) {
        foreach (KoInlineObject* inlineObject, *currentPairedInlineObjectsStack) {
            inlineObject->saveOdf(context);
        }
    }

    closeTagRegion();
}

void KoTextWriter::Private::saveTable(QTextTable *table, QHash<QTextList *, QString> &listStyles, int from, int to)
{
    KoTableColumnAndRowStyleManager tcarManager = KoTableColumnAndRowStyleManager::getManager(table);
    int numberHeadingRows = table->format().property(KoTableStyle::NumberHeadingRows).toInt();
    TagInformation tableTagInformation;
    QString tableStyleName = saveTableStyle(*table);
    tableTagInformation.setTagName("table:table");
    tableTagInformation.addAttribute("table:style-name", tableStyleName);
    if (table->format().boolProperty(KoTableStyle::TableIsProtected))
    {
        tableTagInformation.addAttribute("table:protected", "true");
    }

    if (table->format().hasProperty(KoTableStyle::TableTemplate))
    {
        tableTagInformation.addAttribute("table:template-name",
                                         sharedData->styleName(table->format().intProperty(KoTableStyle::TableTemplate)));
    }

    if (table->format().boolProperty(KoTableStyle::UseBandingColumnStyles))
    {
        tableTagInformation.addAttribute("table:use-banding-columns-styles", "true");
    }

    if (table->format().boolProperty(KoTableStyle::UseBandingRowStyles))
    {
        tableTagInformation.addAttribute("table:use-banding-rows-styles", "true");
    }

    if (table->format().boolProperty(KoTableStyle::UseFirstColumnStyles))
    {
        tableTagInformation.addAttribute("table:use-first-column-styles", "true");
    }

    if (table->format().boolProperty(KoTableStyle::UseFirstRowStyles))
    {
        tableTagInformation.addAttribute("table:use-first-row-styles", "true");
    }

    if (table->format().boolProperty(KoTableStyle::UseLastColumnStyles))
    {
        tableTagInformation.addAttribute("table:use-last-column-styles", "true");
    }

    if (table->format().boolProperty(KoTableStyle::UseLastRowStyles))
    {
        tableTagInformation.addAttribute("table:use-last-row-styles", "true");
    }



    int firstColumn = 0;
    int lastColumn = table->columns() -1;
    int firstRow = 0;
    int lastRow = table->rows() -1;
    if (to != -1 && from >= table->firstPosition() && to <= table->lastPosition()) {
        firstColumn = table->cellAt(from).column();
        firstRow = table->cellAt(from).row();
        lastColumn = table->cellAt(to).column();
        lastRow = table->cellAt(to).row();

        if (firstColumn == lastColumn && firstRow == lastRow && from >= table->firstPosition()) {
            // we only selected something inside a single cell so don't save a table
            writeBlocks(table->document(), from, to, listStyles, table);
            return;
        }
    }


    openTagRegion(KoTextWriter::Private::Table, tableTagInformation);

    for (int c = firstColumn ; c <= lastColumn; c++) {
        KoTableColumnStyle columnStyle = tcarManager.columnStyle(c);
        int repetition = 0;

        for (; repetition <= (lastColumn - c) ; repetition++)
        {
            if (columnStyle != tcarManager.columnStyle(c + repetition + 1))
                break;
        }

        TagInformation tableColumnInformation;
        tableColumnInformation.setTagName("table:table-column");
        QString columnStyleName = saveTableColumnStyle(columnStyle, c, tableStyleName);
        tableColumnInformation.addAttribute("table:style-name", columnStyleName);

        if (repetition > 0)
            tableColumnInformation.addAttribute("table:number-columns-repeated", repetition + 1);

        openTagRegion(KoTextWriter::Private::TableColumn, tableColumnInformation);
        closeTagRegion();
        c += repetition;
    }

    if (numberHeadingRows)
        writer->startElement("table:table-header-rows");

    // TODO make work for copying part of table that has header rows - copy header rows additionally or not ?
    for (int r = firstRow; r <= lastRow; r++) {
        TagInformation tableRowInformation;
        tableRowInformation.setTagName("table:table-row");
        KoTableRowStyle rowStyle = tcarManager.rowStyle(r);
        if (!rowStyle.isEmpty())
        {
            QString rowStyleName = saveTableRowStyle(rowStyle, r, tableStyleName);
            tableRowInformation.addAttribute("table:style-name", rowStyleName);
        }
        openTagRegion(KoTextWriter::Private::TableRow, tableRowInformation);

        for (int c = firstColumn; c <= lastColumn; c++) {
            QTextTableCell cell = table->cellAt(r, c);

            TagInformation tableCellInformation;
            if ((cell.row() == r) && (cell.column() == c)) {
                tableCellInformation.setTagName("table:table-cell");
                if (cell.rowSpan() > 1)
                    tableCellInformation.addAttribute("table:number-rows-spanned", cell.rowSpan());
                if (cell.columnSpan() > 1)
                    tableCellInformation.addAttribute("table:number-columns-spanned", cell.columnSpan());
                if (cell.format().boolProperty(KoTableCellStyle::CellIsProtected))
                {
                    tableCellInformation.addAttribute("table:protected", "true");
                }

                // Save the Rdf for the table cell
                QTextTableCellFormat cellFormat = cell.format().toTableCellFormat();
                QVariant v = cellFormat.property(KoTableCellStyle::InlineRdf);
                if (KoTextInlineRdf* inlineRdf = v.value<KoTextInlineRdf*>()) {
                    inlineRdf->saveOdf(context, writer);
                }

                QString cellStyleName = saveTableCellStyle(cellFormat, c, tableStyleName);
                tableCellInformation.addAttribute("table:style-name", cellStyleName);
                openTagRegion(KoTextWriter::Private::TableCell, tableCellInformation);
                writeBlocks(table->document(), cell.firstPosition(), cell.lastPosition(), listStyles, table);
            } else {
                tableCellInformation.setTagName("table:covered-table-cell");
                if (cell.format().boolProperty(KoTableCellStyle::CellIsProtected))
                {
                    tableCellInformation.addAttribute("table:protected", "true");
                }
                openTagRegion(KoTextWriter::Private::TableCell, tableCellInformation);
            }
            closeTagRegion();
        }
        closeTagRegion();

        if (r + 1 == numberHeadingRows) {
            writer->endElement();   // table:table-header-rows
            writer->startElement("table:table-rows");
        }
    }

    if (numberHeadingRows)
        writer->endElement();   // table:table-rows
    closeTagRegion();

}

void KoTextWriter::Private::saveTableOfContents(QTextDocument *document, QHash<QTextList *, QString> &listStyles, QTextBlock toc)
{
    Q_UNUSED(document);

    writer->startElement("text:table-of-content");

    KoTableOfContentsGeneratorInfo *info = toc.blockFormat().property(KoParagraphStyle::TableOfContentsData).value<KoTableOfContentsGeneratorInfo*>();
    QTextDocument *tocDocument = toc.blockFormat().property(KoParagraphStyle::GeneratedDocument).value<QTextDocument*>();
    if (!info->m_styleName.isNull()) {
            writer->addAttribute("text:style-name",info->m_styleName);
    }
    writer->addAttribute("text:name",info->m_name);

    info->saveOdf(writer);

    writer->startElement("text:index-body");
    // write the title (one p block)
    QTextCursor localBlock = tocDocument->rootFrame()->firstCursorPosition();
    localBlock.movePosition(QTextCursor::NextBlock);
    int endTitle = localBlock.position();
    writer->startElement("text:index-title");
    writer->addAttribute("text:name", QString("%1_Head").arg(info->m_name));
    writeBlocks(tocDocument, 0, endTitle, listStyles);
    writer->endElement(); // text:index-title

    writeBlocks(tocDocument, endTitle, -1, listStyles);

    writer->endElement(); // table:index-body
    writer->endElement(); // table:table-of-content
}

void KoTextWriter::Private::saveBibliography(QTextDocument *document, QHash<QTextList *, QString> &listStyles, QTextBlock bib)
{
    Q_UNUSED(document);

    writer->startElement("text:bibliography");

    KoBibliographyInfo *info = bib.blockFormat().property(KoParagraphStyle::BibliographyData).value<KoBibliographyInfo*>();
    QTextDocument *bibDocument = bib.blockFormat().property(KoParagraphStyle::GeneratedDocument).value<QTextDocument*>();
    if (!info->m_styleName.isNull()) {
            writer->addAttribute("text:style-name",info->m_styleName);
    }
    writer->addAttribute("text:name",info->m_name);

    info->saveOdf(writer);

    writer->startElement("text:index-body");
    // write the title (one p block)
    QTextCursor localBlock = bibDocument->rootFrame()->firstCursorPosition();
    localBlock.movePosition(QTextCursor::NextBlock);
    int endTitle = localBlock.position();
    writer->startElement("text:index-title");
    writeBlocks(bibDocument, 0, endTitle, listStyles);
    writer->endElement(); // text:index-title

    writeBlocks(bibDocument, endTitle, -1, listStyles);

    writer->endElement(); // table:index-body
    writer->endElement(); // table:bibliography
}

QTextBlock& KoTextWriter::Private::saveList(QTextBlock &block, QHash<QTextList *, QString> &listStyles, int level, QTextTable *currentTable)
{
    QTextList *textList, *topLevelTextList;
    topLevelTextList = textList = block.textList();

    int headingLevel = 0, numberedParagraphLevel = 0;
    QTextBlockFormat blockFormat = block.blockFormat();
    headingLevel = blockFormat.intProperty(KoParagraphStyle::OutlineLevel);
    numberedParagraphLevel = blockFormat.intProperty(KoParagraphStyle::ListLevel);

    KoTextDocument textDocument(block.document());
    KoList *list = textDocument.list(block);
    int topListLevel = KoList::level(block);

    bool listStarted = false;
    if (!headingLevel && !numberedParagraphLevel) {
        listStarted = true;

        TagInformation listTagInformation;
        listTagInformation.setTagName("text:list");
        listTagInformation.addAttribute("text:style-name", listStyles[textList]);

        if (list && listXmlIds.contains(list->listContinuedFrom())) {
            listTagInformation.addAttribute("text:continue-list", listXmlIds.value(list->listContinuedFrom()));
        }

        QString listXmlId = QString("list-%1").arg(createXmlId());
        listTagInformation.addAttribute("xml:id", listXmlId);
        if (! listXmlIds.contains(list)) {
            listXmlIds.insert(list, listXmlId);
        }

        openTagRegion(KoTextWriter::Private::List, listTagInformation);
    }

    if (!headingLevel) {
      do {
            if (numberedParagraphLevel) {
                TagInformation paraTagInformation;
                paraTagInformation.setTagName("text:numbered-paragraph");
                paraTagInformation.addAttribute("text:level", numberedParagraphLevel);
                paraTagInformation.addAttribute("text:style-name", listStyles.value(textList));

                QString listId = numberedParagraphListIds.value(list, QString("list-%1").arg(createXmlId()));
                numberedParagraphListIds.insert(list, listId);
                paraTagInformation.addAttribute("text:list-id", listId);

                openTagRegion(KoTextWriter::Private::NumberedParagraph, paraTagInformation);
                writeBlocks(textDocument.document(), block.position(), block.position() + block.length() - 1, listStyles, currentTable, textList);
                closeTagRegion();
            } else {

                const bool listHeader = blockFormat.boolProperty(KoParagraphStyle::IsListHeader)|| blockFormat.boolProperty(KoParagraphStyle::UnnumberedListItem);
                TagInformation listItemTagInformation;
                listItemTagInformation.setTagName(listHeader ? "text:list-header" : "text:list-item");
                if (block.blockFormat().hasProperty(KoParagraphStyle::ListStartValue)) {
                    int startValue = block.blockFormat().intProperty(KoParagraphStyle::ListStartValue);
                    listItemTagInformation.addAttribute("text:start-value", startValue);
                }
                if (textList == topLevelTextList) {
                    openTagRegion(KoTextWriter::Private::ListItem, listItemTagInformation);
                } else {
                    // This is a sub-list. So check for a list-change
                    openTagRegion(KoTextWriter::Private::List, listItemTagInformation);
                }

                if (KoListStyle::isNumberingStyle(textList->format().style())) {
                    KoTextBlockData blockData(block);
                    writer->startElement("text:number", false);
                    writer->addTextSpan(blockData.counterText());
                    writer->endElement();
                }

                if (topListLevel == level && textList == topLevelTextList) {
                    writeBlocks(textDocument.document(), block.position(), block.position() + block.length() - 1, listStyles, currentTable, textList);
                    // we are generating a text:list-item. Look forward and generate unnumbered list items.
                    while (true) {
                        QTextBlock nextBlock = block.next();
                        if (!nextBlock.textList() || !nextBlock.blockFormat().boolProperty(KoParagraphStyle::UnnumberedListItem))
                            break;
                        block = nextBlock;
                        saveParagraph(block, block.position(), block.position() + block.length() - 1);
                    }
                } else {
                    //This is a sub-list
                    while (KoList::level(block) >= (level + 1) && !(headingLevel || numberedParagraphLevel)) {
                        block = saveList(block, listStyles, level + 1, currentTable);

                        blockFormat = block.blockFormat();
                        headingLevel = blockFormat.intProperty(KoParagraphStyle::OutlineLevel);
                        numberedParagraphLevel = blockFormat.intProperty(KoParagraphStyle::ListLevel);
                    }
                    //saveList will return a block one-past the last block of the list.
                    //Since we are doing a block.next() below, we need to go one back.
                    block = block.previous();
                }

                closeTagRegion();

            }
            block = block.next();
            blockFormat = block.blockFormat();
            headingLevel = blockFormat.intProperty(KoParagraphStyle::OutlineLevel);
            numberedParagraphLevel = blockFormat.intProperty(KoParagraphStyle::ListLevel);
            textList = block.textList();
        } while ((textDocument.list(block) == list) && (KoList::level(block) >= topListLevel));
    }

    if (listStarted) {
        closeTagRegion();
    }

    return block;
}

void KoTextWriter::Private::addNameSpaceDefinitions(QString &generatedXmlString)
{
    //Generate the name-space definitions so that it can be parsed. Like what is office:text, office:delta etc
    QString nameSpaceDefinitions;
    QTextStream nameSpacesStream(&nameSpaceDefinitions);
    nameSpacesStream.setCodec("UTF-8");

    nameSpacesStream << "<generated-xml ";
    nameSpacesStream << "xmlns:office=\"" << KoXmlNS::office << "\" ";
    nameSpacesStream << "xmlns:meta=\"" << KoXmlNS::meta << "\" ";
    nameSpacesStream << "xmlns:config=\"" << KoXmlNS::config << "\" ";
    nameSpacesStream << "xmlns:text=\"" << KoXmlNS::text << "\" ";
    nameSpacesStream << "xmlns:table=\"" << KoXmlNS::table << "\" ";
    nameSpacesStream << "xmlns:draw=\"" << KoXmlNS::draw << "\" ";
    nameSpacesStream << "xmlns:presentation=\"" << KoXmlNS::presentation << "\" ";
    nameSpacesStream << "xmlns:dr3d=\"" << KoXmlNS::dr3d << "\" ";
    nameSpacesStream << "xmlns:chart=\"" << KoXmlNS::chart << "\" ";
    nameSpacesStream << "xmlns:form=\"" << KoXmlNS::form << "\" ";
    nameSpacesStream << "xmlns:script=\"" << KoXmlNS::script << "\" ";
    nameSpacesStream << "xmlns:style=\"" << KoXmlNS::style << "\" ";
    nameSpacesStream << "xmlns:number=\"" << KoXmlNS::number << "\" ";
    nameSpacesStream << "xmlns:math=\"" << KoXmlNS::math << "\" ";
    nameSpacesStream << "xmlns:svg=\"" << KoXmlNS::svg << "\" ";
    nameSpacesStream << "xmlns:fo=\"" << KoXmlNS::fo << "\" ";
    nameSpacesStream << "xmlns:anim=\"" << KoXmlNS::anim << "\" ";
    nameSpacesStream << "xmlns:smil=\"" << KoXmlNS::smil << "\" ";
    nameSpacesStream << "xmlns:calligra=\"" << KoXmlNS::calligra << "\" ";
    nameSpacesStream << "xmlns:officeooo=\"" << KoXmlNS::officeooo << "\" ";
    nameSpacesStream << "xmlns:split=\"" << KoXmlNS::split << "\" ";
    nameSpacesStream << "xmlns:ac=\"" << KoXmlNS::ac << "\" ";
    nameSpacesStream << ">";

    generatedXmlString.prepend(nameSpaceDefinitions);
    generatedXmlString.append("</generated-xml>");
}


void KoTextWriter::Private::writeAttributes(QTextStream &outputXmlStream, KoXmlElement &element)
{
    QList<QPair<QString, QString> > attributes = element.attributeFullNames();

    foreach (const Attribute &attributeNamePair, attributes) {
        if (attributeNamePair.first == KoXmlNS::text) {
            outputXmlStream << " text:" << attributeNamePair.second << "=";
            outputXmlStream << "\"" << element.attributeNS(KoXmlNS::text, attributeNamePair.second) << "\"";
        } else {
            //To Be Added when needed
        }
    }
}

void KoTextWriter::Private::writeNode(QTextStream &outputXmlStream, KoXmlNode &node, bool writeOnlyChildren)
{
    if (node.isText()) {
        outputXmlStream  << node.toText().data();
    } else if (node.isElement()) {
        KoXmlElement element = node.toElement();
        if ((element.localName() == "removed-content") && !element.childNodesCount()) {
            return;
        }

        if (!writeOnlyChildren) {
            outputXmlStream << "<" << element.prefix() << ":" << element.localName();
            writeAttributes(outputXmlStream,element);
            outputXmlStream << ">";
        }

        for (KoXmlNode node = element.firstChild(); !node.isNull(); node = node.nextSibling()) {
            writeNode(outputXmlStream, node);
        }

        if (!writeOnlyChildren) {
            outputXmlStream << "</" << element.prefix() << ":" << element.localName() << ">";
        }
    }
}

QString KoTextWriter::Private::createXmlId()
{
    QString uuid = QUuid::createUuid().toString();
    uuid.remove('{');
    uuid.remove('}');
    return uuid;
}

