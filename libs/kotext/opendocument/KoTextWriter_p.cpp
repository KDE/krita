/*
 *  Copyright (c) 2010 Boudewijn Rempt <boud@valdyas.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
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


#include <KoElementReference.h>

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




KoTextWriter::Private::Private(KoShapeSavingContext &context)
    : rdfData(0)
    , sharedData(0)
    , styleManager(0)
    , changeTracker(0)
    , document(0)
    , writer(0)
    , context(context)
    , splitEndBlockNumber(-1)
    , splitRegionOpened(false)
    , splitIdCounter(1)
    , deleteMergeRegionOpened(false)
    , deleteMergeEndBlockNumber(-1)
{
    currentPairedInlineObjectsStack = new QStack<KoInlineObject*>();
    writer = &context.xmlWriter();
    changeStack.push(0);
}


void KoTextWriter::Private::writeBlocks(QTextDocument *document, int from, int to, QHash<QTextList *, QString> &listStyles, QTextTable *currentTable, QTextList *currentList)
{
    pairedInlineObjectsStackStack.push(currentPairedInlineObjectsStack);
    currentPairedInlineObjectsStack = new QStack<KoInlineObject*>();
    QTextBlock block = document->findBlock(from);
    int sectionLevel = 0;

    while (block.isValid() && ((to == -1) || (block.position() <= to))) {

        QTextCursor cursor(block);

        int frameType = cursor.currentFrame()->format().intProperty(KoText::SubFrameType);
        if (frameType == KoText::AuxillaryFrameType) {
            break; // we've reached the "end" (end/footnotes saved by themselves)
                   // note how NoteFrameType passes through here so the notes can
                   // call writeBlocks to save their contents.
        }

        QTextBlockFormat format = block.blockFormat();
        if (format.hasProperty(KoParagraphStyle::SectionStartings)) {
            QVariant v = format.property(KoParagraphStyle::SectionStartings);
            QList<QVariant> sectionStarts = v.value<QList<QVariant> >();

            foreach (QVariant sv, sectionStarts) {
                KoSection* section = (KoSection*)(sv.value<void*>());
                if (section) {
                    ++sectionLevel;
                    section->saveOdf(context);
                }
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
        int blockOutlineLevel = format.property(KoParagraphStyle::OutlineLevel).toInt();

        if (cursor.currentTable() && cursor.currentTable() != currentTable) {
            // Call the code to save the table....
            saveTable(cursor.currentTable(), listStyles);
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

        if (changeTracker && changeTracker->saveFormat() == KoChangeTracker::DELTAXML) {
            if (!deleteMergeRegionOpened && !cursor.currentTable() && (!cursor.currentList() || blockOutlineLevel)) {
                deleteMergeEndBlockNumber = checkForDeleteMerge(block);
                if (deleteMergeEndBlockNumber != -1) {
                    deleteMergeRegionOpened = true;
                    openSplitMergeRegion();
                }
            }
        }

        saveParagraph(block, from, to);

        if (changeTracker && changeTracker->saveFormat() == KoChangeTracker::DELTAXML) {
            if (deleteMergeRegionOpened && (block.blockNumber() == deleteMergeEndBlockNumber) && (!cursor.currentList() || blockOutlineLevel)) {
                closeSplitMergeRegion();
                deleteMergeRegionOpened = false;
                deleteMergeEndBlockNumber = -1;
                postProcessDeleteMergeXml();
            }
        }

        if (format.hasProperty(KoParagraphStyle::SectionEndings)) {
            QVariant v = format.property(KoParagraphStyle::SectionEndings);
            QList<QVariant> sectionEndings = v.value<QList<QVariant> >();
            KoSectionEnd sectionEnd;
            foreach (QVariant sv, sectionEndings) {
                if (sectionLevel >= 1) {
                    --sectionLevel;
                    sectionEnd.saveOdf(context);
                }
            }
        }


        block = block.next();
    } // while

    while (sectionLevel >= 1) {
        --sectionLevel;
        KoSectionEnd sectionEnd;
        sectionEnd.saveOdf(context);
    }

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

void KoTextWriter::Private::saveAllChanges()
{
    if (!changeTracker) return;
    changeTransTable = changeTracker->saveInlineChanges(changeTransTable, sharedData->genChanges());
}

//---------------------------- PRIVATE -----------------------------------------------------------

void KoTextWriter::Private::saveODF12Change(QTextCharFormat format)
{
    if (!changeTracker /*&& changeTracker->isEnabled()*/)
        return;//The change tracker exist and we are allowed to save tracked changes

    int changeId = format.property(KoCharacterStyle::ChangeTrackerId).toInt();

    //First we need to check if the eventual already opened change regions are still valid
    while (int change = changeStack.top()) {
        if (!changeId || !changeTracker->isParent(change, changeId)) {
            writer->startElement("text:change-end", false);
            writer->addAttribute("text:change-id", changeTransTable.value(change));
            writer->endElement();
            changeStack.pop();
        }
    }

    if (changeId) { //There is a tracked change
        if (changeTracker->elementById(changeId)->getChangeType() != KoGenChange::DeleteChange) {
            //Now start a new change region if not already done
            if (!changeStack.contains(changeId)) {
                QString changeName = changeTransTable.value(changeId);
                writer->startElement("text:change-start", false);
                writer->addAttribute("text:change-id",changeName);
                writer->endElement();
                changeStack.push(changeId);
            }
        }
    }
    KoInlineTextObjectManager *textObjectManager = KoTextDocument(document).inlineTextObjectManager();
    KoDeleteChangeMarker *changeMarker;
    if (textObjectManager && (changeMarker = dynamic_cast<KoDeleteChangeMarker*>(textObjectManager->inlineTextObject(format)))) {
        if (!savedDeleteChanges.contains(changeMarker->changeId())) {
            QString deleteChangeXml = generateDeleteChangeXml(changeMarker);
            changeMarker->setDeleteChangeXml(deleteChangeXml);
            changeMarker->saveOdf(context);
            savedDeleteChanges.append(changeMarker->changeId());
        }
    }
}

QString KoTextWriter::Private::generateDeleteChangeXml(KoDeleteChangeMarker *marker)
{
    if (!changeTracker) return QString();

    //Create a QTextDocument from the Delete Fragment
    QTextDocument doc;
    QTextCursor cursor(&doc);
    cursor.insertFragment(changeTracker->elementById(marker->changeId())->getDeleteData());

    //Save the current writer
    KoXmlWriter &oldWriter = context.xmlWriter();

    //Create a new KoXmlWriter pointing to a QBuffer
    QByteArray xmlArray;
    QBuffer xmlBuffer(&xmlArray);
    KoXmlWriter newXmlWriter(&xmlBuffer);

    //Set our xmlWriter as the writer to be used
    writer = &newXmlWriter;
    context.setXmlWriter(newXmlWriter);

    //Call writeBlocks to generate the xml
    QHash<QTextList *,QString> listStyles = saveListStyles(doc.firstBlock(), doc.characterCount());
    writeBlocks(&doc, 0, doc.characterCount(),listStyles);

    //Restore the actual xml writer
    writer = &oldWriter;
    context.setXmlWriter(oldWriter);

    QString generatedXmlString(xmlArray);
    return generatedXmlString;
}

int KoTextWriter::Private::openTagRegion(int position, ElementType elementType, TagInformation& tagInformation)
{
    int changeId = 0, returnChangeId = 0;

    if (!changeTracker) {
        //kDebug(30015) << "tag:" << tagInformation.name() << openedTagStack.size();
        if (tagInformation.name()) {
            writer->startElement(tagInformation.name(), elementType != ParagraphOrHeader);
            QPair<QString, QString> attribute;
            foreach (attribute, tagInformation.attributes()) {
                writer->addAttribute(attribute.first.toLocal8Bit(), attribute.second);
            }
        }
        openedTagStack.push(tagInformation.name());
        //kDebug(30015) << "stack" << openedTagStack.size();
        return changeId;
    }

    QTextCursor cursor(document);
    QTextBlock block = document->findBlock(position);

    openedTagStack.push(tagInformation.name());

    KoChangeTracker::ChangeSaveFormat changeSaveFormat = changeTracker->saveFormat();

    if (changeSaveFormat == KoChangeTracker::DELTAXML) {
        switch (elementType) {
            case KoTextWriter::Private::Span:
                cursor.setPosition(position + 1);
                changeId = cursor.charFormat().property(KoCharacterStyle::ChangeTrackerId).toInt();
                break;
            case KoTextWriter::Private::ParagraphOrHeader:
                changeId = checkForBlockChange(block);
                break;
            case KoTextWriter::Private::NumberedParagraph:
                changeId = checkForBlockChange(block);
                break;
            case KoTextWriter::Private::ListItem:
                changeId = checkForListItemChange(block);
                break;
            case KoTextWriter::Private::List:
                changeId = checkForListChange(block);
                break;
            case KoTextWriter::Private::TableRow:
                changeId = checkForTableRowChange(position);
                break;
            case KoTextWriter::Private::TableColumn:
                changeId = checkForTableColumnChange(position);
                break;
            case KoTextWriter::Private::TableCell:
                cursor.setPosition(position);
                changeId = cursor.currentTable()->cellAt(position).format().property(KoCharacterStyle::ChangeTrackerId).toInt();
                break;
            case KoTextWriter::Private::Table:
                cursor.setPosition(position);
                QTextTableFormat tableFormat = cursor.currentTable()->format();
                changeId = tableFormat.property(KoCharacterStyle::ChangeTrackerId).toInt();
                break;
        }
    }

    if (!changeId || (changeStack.top() == changeId)) {
        changeId = 0;
    } else if ((changeTracker->isDuplicateChangeId(changeId)) && (changeTracker->originalChangeId(changeId) == changeStack.top())) {
        QVectorIterator<int> changeStackIterator(changeStack);
        changeStackIterator.toBack();

        while ((changeStackIterator.peekPrevious()) && (changeStackIterator.peekPrevious() == changeTracker->originalChangeId(changeId))) {
            changeStackIterator.previous();
            changeId = changeTracker->parent(changeId);
        }
    } else if ((changeTracker->isDuplicateChangeId(changeId)) && (changeTracker->isParent(changeStack.top(), changeId))) {
        changeId = 0;
    }

    returnChangeId = changeId;

    //Navigate through the change history and push into a stack so that they can be processed in the reverse order (i.e starting from earliest)
    QStack<int> changeHistory;
    while (changeId && (changeId != changeStack.top())) {
        changeHistory.push(changeId);
        changeId = changeTracker->parent(changeId);
    }

    if (returnChangeId) {
        changeStack.push(returnChangeId);
    }

    while (changeHistory.size()) {
        int changeId = changeHistory.pop();
        if (changeTracker->isDuplicateChangeId(changeId)) {
            changeId = changeTracker->originalChangeId(changeId);
        }

        if (changeId && changeTracker->elementById(changeId)->getChangeType() == KoGenChange::DeleteChange) {
            writer->startElement("delta:removed-content", false);
            writer->addAttribute("delta:removal-change-idref", changeTransTable.value(changeId));
        } else if (changeId && changeTracker->elementById(changeId)->getChangeType() == KoGenChange::InsertChange) {
            tagInformation.addAttribute("delta:insertion-change-idref", changeTransTable.value(changeId));
            tagInformation.addAttribute("delta:insertion-type", "insert-with-content");
        } else if (changeId && changeTracker->elementById(changeId)->getChangeType() == KoGenChange::FormatChange && elementType == KoTextWriter::Private::Span) {
            KoFormatChangeInformation *formatChangeInformation = changeTracker->formatChangeInformation(changeId);

            if (formatChangeInformation && formatChangeInformation->formatType() == KoFormatChangeInformation::eTextStyleChange) {
                writer->startElement("delta:remove-leaving-content-start", false);
                writer->addAttribute("delta:removal-change-idref", changeTransTable.value(changeId));
                writer->addAttribute("delta:end-element-idref", QString("end%1").arg(changeId));

                cursor.setPosition(position);
                KoTextStyleChangeInformation *textStyleChangeInformation = static_cast<KoTextStyleChangeInformation *>(formatChangeInformation);
                QString styleName = saveCharacterStyle(textStyleChangeInformation->previousCharFormat(), cursor.blockCharFormat());
                if (!styleName.isEmpty()) {
                    writer->startElement("text:span", false);
                    writer->addAttribute("text:style-name", styleName);
                    writer->endElement();
                }
                writer->endElement();
            }

            tagInformation.addAttribute("delta:insertion-change-idref", changeTransTable.value(changeId));
            tagInformation.addAttribute("delta:insertion-type", "insert-around-content");
        } else if (changeId && changeTracker->elementById(changeId)->getChangeType() == KoGenChange::FormatChange
                            && elementType == KoTextWriter::Private::ParagraphOrHeader) {
            KoFormatChangeInformation *formatChangeInformation = changeTracker->formatChangeInformation(changeId);
            if (formatChangeInformation && formatChangeInformation->formatType() == KoFormatChangeInformation::eParagraphStyleChange) {
                KoParagraphStyleChangeInformation *paraStyleChangeInformation = static_cast<KoParagraphStyleChangeInformation *>(formatChangeInformation);
                QString styleName = saveParagraphStyle(paraStyleChangeInformation->previousBlockFormat(), QTextCharFormat());
                QString attributeChangeRecord = changeTransTable.value(changeId) + QString(",") + QString("modify")
                                                                                 + QString(",") + QString("text:style-name")
                                                                                 + QString(",") + styleName;
                tagInformation.addAttribute("ac:change001", attributeChangeRecord);
            }
        } else if (changeId && changeTracker->elementById(changeId)->getChangeType() == KoGenChange::FormatChange
                            && elementType == KoTextWriter::Private::ListItem) {
            KoFormatChangeInformation *formatChangeInformation = changeTracker->formatChangeInformation(changeId);
            if (formatChangeInformation && formatChangeInformation->formatType() == KoFormatChangeInformation::eListItemNumberingChange) {
                KoListItemNumChangeInformation *listItemChangeInfo = static_cast<KoListItemNumChangeInformation *>(formatChangeInformation);

                if (listItemChangeInfo->listItemNumChangeType() == KoListItemNumChangeInformation::eNumberingRestarted) {
                    QString attributeChangeRecord = changeTransTable.value(changeId) + QString(",") + QString("insert")
                                                                                     + QString(",") + QString("text:start-value");
                    tagInformation.addAttribute("ac:change001", attributeChangeRecord);
                } else if (listItemChangeInfo->listItemNumChangeType() == KoListItemNumChangeInformation::eRestartRemoved) {
                    QString attributeChangeRecord = changeTransTable.value(changeId) + QString(",") + QString("remove")
                                                                                     + QString(",") + QString("text:start-value")
                                                                                     + QString(",") + QString::number(listItemChangeInfo->previousStartNumber());
                    tagInformation.addAttribute("ac:change001", attributeChangeRecord);
                }
            }
        }
    }

    if (tagInformation.name()) {
        writer->startElement(tagInformation.name(), false);
        const QVector<QPair<QString, QString> > &attributeList = tagInformation.attributes();
        QPair<QString, QString> attribute;
        foreach(attribute, attributeList) {
            writer->addAttribute(attribute.first.toAscii(), attribute.second.toAscii());
        }
    }

    return returnChangeId;
}

void KoTextWriter::Private::closeTagRegion(int changeId)
{
    // the tag needs to be closed even if there is no change tracking
    //kDebug(30015) << "stack" << openedTagStack.size();
    const char *tagName = openedTagStack.pop();
    //kDebug(30015) << "tag:" << tagName << openedTagStack.size();
    if (tagName) {
        writer->endElement(); // close the tag
    }

    if (!changeTracker) return;

    if (changeId)
        changeStack.pop();

    if (changeId && changeTracker->elementById(changeId)->getChangeType() == KoGenChange::DeleteChange) {
        writer->endElement(); //delta:removed-content
    } else if (changeId && changeTracker->elementById(changeId)->getChangeType() == KoGenChange::FormatChange) {
        KoFormatChangeInformation *formatChangeInformation = changeTracker->formatChangeInformation(changeId);
        if (formatChangeInformation && formatChangeInformation->formatType() == KoFormatChangeInformation::eTextStyleChange) {
            writer->startElement("delta:remove-leaving-content-end", false);
            writer->addAttribute("delta:end-element-id", QString("end%1").arg(changeId));
            writer->endElement();
        }
    }
    return;
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
            KoGenStyle style(KoGenStyle::ParagraphStyle, "text");
            originalCharStyle->saveOdf(style);
            generatedName = context.mainStyles().insert(style, internalName, KoGenStyles::DontAddNumberToName);
        }
    } else { // There are manual changes... We'll have to store them then
        KoGenStyle style(KoGenStyle::ParagraphAutoStyle, "text", originalCharStyle != defaultCharStyle ? internalName : "" /*parent*/);
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
    QString internalName = "";
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
    KoXmlWriter *rdfXmlWriter = new KoXmlWriter(&rdfXmlData);
    rdfXmlWriter->startDocument("rdf");
    rdfXmlWriter->startElement("rdf");
    rdf->saveOdf(context, rdfXmlWriter);
    rdfXmlWriter->endElement();
    rdfXmlWriter->endDocument();
    KoXmlDocument *xmlReader = new KoXmlDocument;
    xmlReader->setContent(rdfXmlData.data(), true);
    KoXmlElement mainElement = xmlReader->documentElement();
    QPair<QString, QString> attributeNameNS;
    foreach (attributeNameNS, mainElement.attributeFullNames()) {
        QString attributeName = QString("%1:%2").arg(KoXmlNS::nsURI2NS(attributeNameNS.first))
                                                .arg(attributeNameNS.second);
        if (attributeName.startsWith(':'))
            attributeName.prepend("xml");
        tagInfos->addAttribute(attributeName, mainElement.attribute(attributeNameNS.second));
    }
    delete(rdfXmlWriter);
}

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

    int changeId = openTagRegion(block.position(), KoTextWriter::Private::ParagraphOrHeader, blockTagInformation);

    if (changeTracker && changeTracker->saveFormat() == KoChangeTracker::DELTAXML) {
        if (!deleteMergeRegionOpened && !splitRegionOpened && !cursor.currentTable() && (!cursor.currentList() || outlineLevel)) {
            splitEndBlockNumber = checkForSplit(block);
            if (splitEndBlockNumber != -1) {
                splitRegionOpened = true;
                QString splitId = QString("split") + QString::number(splitIdCounter);
                writer->addAttribute("split:split001-idref", splitId);
            }
        }

        if (splitRegionOpened && (block.blockNumber() == splitEndBlockNumber)) {
            splitRegionOpened = false;
            splitEndBlockNumber = -1;
            QString splitId = QString("split") + QString::number(splitIdCounter);
            writer->addAttribute("delta:split-id", splitId);
            int changeId = block.blockFormat().intProperty(KoCharacterStyle::ChangeTrackerId);
            writer->addAttribute("delta:insertion-change-idref", changeTransTable.value(changeId));
            writer->addAttribute("delta:insertion-type", "split");
            splitIdCounter++;
        }
    }

    QString styleName = saveParagraphStyle(block);
    if (!styleName.isEmpty())
        writer->addAttribute("text:style-name", styleName);

    KoElementReference xmlid;
    xmlid.invalidate();

    if (const KoTextBlockData *blockData = dynamic_cast<const KoTextBlockData *>(block.userData())) {
        if (blockData->saveXmlID()) {
            xmlid = context.xmlid(blockData);
            xmlid.saveOdf(writer, KoElementReference::TextId);
        }
    }

    if (changeTracker && changeTracker->saveFormat() == KoChangeTracker::DELTAXML) {
        QTextBlock previousBlock = block.previous();
        if (previousBlock.isValid()) {
            QTextBlockFormat blockFormat = block.blockFormat();
            int changeId = blockFormat.intProperty(KoCharacterStyle::ChangeTrackerId);
            if (changeId && changeTracker->elementById(changeId)->getChangeType() == KoGenChange::DeleteChange) {
                QTextFragment firstFragment = (block.begin()).fragment();
                QTextCharFormat firstFragmentFormat = firstFragment.charFormat();
                int firstFragmentChangeId = firstFragmentFormat.intProperty(KoCharacterStyle::ChangeTrackerId);
                if (changeTracker->isDuplicateChangeId(firstFragmentChangeId)) {
                    firstFragmentChangeId = changeTracker->originalChangeId(firstFragmentChangeId);
                }
                if (firstFragmentChangeId != changeId) {
                    QString outputXml("<delta:removed-content delta:removal-change-idref=\"" + changeTransTable.value(changeId) + "\"/>");
                    writer->addCompleteElement(outputXml.toUtf8());
                }
            }
        }
    }

    // Write the fragments and their formats
    QTextCharFormat blockCharFormat = cursor.blockCharFormat();
    QTextCharFormat previousCharFormat;
    QTextBlock::iterator it;
    if (KoTextInlineRdf* inlineRdf = KoTextInlineRdf::tryToGetInlineRdf(blockCharFormat)) {
        // Write xml:id here for Rdf
        kDebug(30015) << "have inline rdf xmlid:" << inlineRdf->xmlId() << "active xml id" << xmlid.toString();
        inlineRdf->saveOdf(context, writer, xmlid);
    }

    QString previousFragmentLink;
    int linkTagChangeId = -1;
    for (it = block.begin(); !(it.atEnd()); ++it) {
        QTextFragment currentFragment = it.fragment();
        const int fragmentStart = currentFragment.position();
        const int fragmentEnd = fragmentStart + currentFragment.length();
        if (to != -1 && fragmentStart >= to)
            break;
        if (currentFragment.isValid()) {
            QTextCharFormat charFormat = currentFragment.charFormat();
            QTextCharFormat compFormat = charFormat;
            previousCharFormat.clearProperty(KoCharacterStyle::ChangeTrackerId);
            compFormat.clearProperty(KoCharacterStyle::ChangeTrackerId);

            if (changeTracker && changeTracker->saveFormat() == KoChangeTracker::ODF_1_2) {
                saveODF12Change(charFormat);
            }

            if ((!previousFragmentLink.isEmpty()) && (charFormat.anchorHref() != previousFragmentLink || !charFormat.isAnchor())) {
                // Close the current text:a
                closeTagRegion(linkTagChangeId);
                previousFragmentLink = "";
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
                    kDebug(30015) << "have inline rdf xmlid:" << inlineRdf->xmlId();
                    saveInlineRdf(inlineRdf, &linkTagInformation);
                }
                linkTagChangeId = openTagRegion(currentFragment.position(), KoTextWriter::Private::Span, linkTagInformation);
            }

            KoInlineTextObjectManager *textObjectManager = KoTextDocument(document).inlineTextObjectManager();
            KoInlineObject *inlineObject = textObjectManager ? textObjectManager->inlineTextObject(charFormat) : 0;
            // If we are in an inline object
            if (currentFragment.length() == 1 && inlineObject
                    && currentFragment.text()[0].unicode() == QChar::ObjectReplacementCharacter) {
                if (!dynamic_cast<KoDeleteChangeMarker*>(inlineObject)) {
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
                        int changeId = charFormat.intProperty(KoCharacterStyle::ChangeTrackerId);
                        KoTextAnchor *textAnchor = dynamic_cast<KoTextAnchor *>(inlineObject);
                        if (changeTracker && changeTracker->saveFormat() == KoChangeTracker::DELTAXML) {
                            if (textAnchor && changeId && changeTracker->elementById(changeId)->getChangeType() == KoGenChange::InsertChange) {
                                textAnchor->shape()->setAdditionalAttribute("delta:insertion-change-idref", changeTransTable.value(changeId));
                                textAnchor->shape()->setAdditionalAttribute("delta:insertion-type", "insert-with-content");
                            } else if (textAnchor && changeId && changeTracker->elementById(changeId)->getChangeType() == KoGenChange::DeleteChange) {
                                writer->startElement("delta:removed-content", false);
                                writer->addAttribute("delta:removal-change-idref", changeTransTable.value(changeId));
                            }
                        }

                        inlineObject->saveOdf(context);

                        if (changeTracker && changeTracker->saveFormat() == KoChangeTracker::DELTAXML) {
                            if (textAnchor && changeId && changeTracker->elementById(changeId)->getChangeType() == KoGenChange::InsertChange) {
                                textAnchor->shape()->removeAdditionalAttribute("delta:insertion-change-idref");
                                textAnchor->shape()->removeAdditionalAttribute("delta:insertion-type");
                            } else if (textAnchor && changeId && changeTracker->elementById(changeId)->getChangeType() == KoGenChange::DeleteChange) {
                                writer->endElement();
                            }
                        }
                    }

                    if (saveSpan) {
                        writer->endElement();
                    }
                    //
                    // Track the end marker for matched pairs so we produce valid
                    // ODF
                    //
                    if (KoTextMeta* z = dynamic_cast<KoTextMeta*>(inlineObject)) {
                        kDebug(30015) << "found kometa, type:" << z->type();
                        if (z->type() == KoTextMeta::StartBookmark)
                            currentPairedInlineObjectsStack->push(z->endBookmark());
                        if (z->type() == KoTextMeta::EndBookmark
                                && !currentPairedInlineObjectsStack->isEmpty())
                            currentPairedInlineObjectsStack->pop();
                    } else if (KoBookmark* z = dynamic_cast<KoBookmark*>(inlineObject)) {
                        if (z->type() == KoBookmark::StartBookmark)
                            currentPairedInlineObjectsStack->push(z->endBookmark());
                        if (z->type() == KoBookmark::EndBookmark
                                && !currentPairedInlineObjectsStack->isEmpty())
                            currentPairedInlineObjectsStack->pop();
                    }
                }
            } else {
                // Normal block, easier to handle
                QString styleName = saveCharacterStyle(charFormat, blockCharFormat);

                TagInformation fragmentTagInformation;
                if (!styleName.isEmpty() /*&& !identical*/) {
                    fragmentTagInformation.setTagName("text:span");
                    fragmentTagInformation.addAttribute("text:style-name", styleName);
                }

                int changeId = openTagRegion(currentFragment.position(), KoTextWriter::Private::Span, fragmentTagInformation);

                QString text = currentFragment.text();
                int spanFrom = fragmentStart >= from ? 0 : from;
                int spanTo = to == -1 ? fragmentEnd : (fragmentEnd > to ? to : fragmentEnd);
                if (spanFrom != fragmentStart || spanTo != fragmentEnd) { // avoid mid, if possible
                    writer->addTextSpan(text.mid(spanFrom - fragmentStart, spanTo - spanFrom));
                } else {
                    writer->addTextSpan(text);
                }

                closeTagRegion(changeId);
            } // if (inlineObject)

            previousCharFormat = charFormat;
        }
    }
    if (!previousFragmentLink.isEmpty()) {
        writer->endElement();
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

    if (changeTracker) {
        if (changeTracker->saveFormat() == KoChangeTracker::DELTAXML) {
            QTextBlock nextBlock = block.next();
            if (nextBlock.isValid() && deleteMergeRegionOpened) {
                QTextBlockFormat nextBlockFormat = nextBlock.blockFormat();
                int changeId = nextBlockFormat.intProperty(KoCharacterStyle::ChangeTrackerId);
                if (changeId && changeTracker->elementById(changeId)->getChangeType() == KoGenChange::DeleteChange) {
                    QTextFragment lastFragment = (--block.end()).fragment();
                    QTextCharFormat lastFragmentFormat = lastFragment.charFormat();
                    int lastFragmentChangeId = lastFragmentFormat.intProperty(KoCharacterStyle::ChangeTrackerId);
                    if (changeTracker->isDuplicateChangeId(lastFragmentChangeId)) {
                        lastFragmentChangeId = changeTracker->originalChangeId(lastFragmentChangeId);
                    }
                    if (lastFragmentChangeId != changeId) {
                        QString outputXml("<delta:removed-content delta:removal-change-idref=\"" + changeTransTable.value(changeId) + "\"/>");
                        writer->addCompleteElement(outputXml.toUtf8());
                    }
                }
            }
        }
        if (changeTracker->saveFormat() == KoChangeTracker::ODF_1_2) {
            while (int change = changeStack.top()) {
                writer->startElement("text:change-end", false);
                writer->addAttribute("text:change-id", changeTransTable.value(change));
                writer->endElement();
                changeStack.pop();
            }
        }
    }

    closeTagRegion(changeId);
}

//Check if the whole Block is a part of a single change
//If so return the changeId else return 0
int KoTextWriter::Private::checkForBlockChange(const QTextBlock &block)
{
    int changeId = 0;
    if (!changeTracker) {
        return changeId;
    }
    QTextBlock::iterator it = block.begin();

    if (it.atEnd()) {
        //This is a empty block. So just return the change-id of the block
        changeId = block.blockFormat().property(KoCharacterStyle::ChangeTrackerId).toInt();
    }

    for (it = block.begin(); !(it.atEnd()); ++it) {
        QTextFragment currentFragment = it.fragment();
        if (currentFragment.isValid()) {
            QTextCharFormat charFormat = currentFragment.charFormat();
            int currentChangeId = charFormat.property(KoCharacterStyle::ChangeTrackerId).toInt();

            KoInlineTextObjectManager *textObjectManager = KoTextDocument(block.document()).inlineTextObjectManager();
            if (textObjectManager) {
                KoInlineObject *inlineObject = textObjectManager->inlineTextObject(charFormat);
                if (currentFragment.length() == 1 && inlineObject && currentFragment.text()[0].unicode() == QChar::ObjectReplacementCharacter) {
                    continue;
                }
            }

            if (!currentChangeId) {
                // Encountered a fragment that is not a change
                // So break out of loop and return 0
                changeId = 0;
                break;
            } else {
                // This Fragment is a change fragment. Continue further.
                if (changeId == 0) {
                    //First fragment and it is a change-fragment
                    //Store it and continue
                    changeId = currentChangeId;
                    continue;
                } else {
                    if (currentChangeId == changeId) {
                        //Change Fragment and it is the same as the first change.
                        //continue looking
                        continue;
                    } else if (changeTracker->isParent(currentChangeId, changeId)) {
                        //The currentChangeId is a parent of changeId
                        changeId = currentChangeId;
                        continue;
                    } else if (changeTracker->isParent(changeId, currentChangeId)) {
                        //The current change id is a child of change-id
                        continue;
                    } else {
                        //A Change Fragment but not same as the first change fragment
                        //Break-out of loop and return 0
                        changeId = 0;
                        break;
                    }
                }
            }
        }
    }
    return changeId;
}

//Check if the whole list-item is a part of a single change
//If so return the changeId else return 0
int KoTextWriter::Private::checkForListItemChange(const QTextBlock &block)
{
    QTextBlock listItemBlock = block;
    int listItemChangeId = checkForBlockChange(listItemBlock);
    while (listItemChangeId) {
        QTextBlock nextBlock = listItemBlock.next();
        if (!nextBlock.textList() || !nextBlock.blockFormat().boolProperty(KoParagraphStyle::UnnumberedListItem))
            break;
        listItemBlock = nextBlock;
        listItemChangeId = checkForBlockChange(listItemBlock);
    }
    return listItemChangeId;
}

//Check if the whole list is a part of a single change
//If so return the changeId else return 0
int KoTextWriter::Private::checkForListChange(const QTextBlock &listBlock)
{
    QTextBlock block(listBlock);

    KoTextDocument textDocument(block.document());
    KoList *list = textDocument.list(block);
    int topListLevel = KoList::level(block);

    int changeId = 0;
    do {
        int currentChangeId = checkForBlockChange(block);
        if (changeTracker->isDuplicateChangeId(currentChangeId)) {
            currentChangeId = changeTracker->originalChangeId(currentChangeId);
        }

        if (!currentChangeId) {
            // Encountered a list-item that is not a change
            // So break out of loop and return 0
            changeId = 0;
            break;
        } else {
            // This list-item is a changed cell. Continue further.
            if (changeId == 0) {
                //First list-item and it is a changed list-item
                //Store it and continue
                changeId = currentChangeId;
                block = block.next();
                continue;
            } else {
                if (currentChangeId == changeId) {
                    //Change found and it is the same as the first change.
                    //continue looking
                    block = block.next();
                    continue;
                } else if (changeTracker->isParent(currentChangeId, changeId)) {
                    //The currentChangeId is a parent of changeId
                    changeId = currentChangeId;
                    block = block.next();
                    continue;
                } else if (changeTracker->isParent(changeId, currentChangeId)) {
                    //The current change id is a child of change-id
                    block = block.next();
                    continue;
                } else {
                    //A Change found but not same as the first change
                    //Break-out of loop and return 0
                    changeId = 0;
                    break;
                }
            }
        }
    } while ((textDocument.list(block) == list) && (KoList::level(block) >= topListLevel));
    return changeId;
}

//Check if the whole of table row is a part of a single change
//If so return the changeId else return 0
int KoTextWriter::Private::checkForTableRowChange(int position)
{
    int changeId = 0;
    QTextCursor cursor(document);
    cursor.setPosition(position);
    QTextTable *table = cursor.currentTable();

    if (table) {
        int row = table->cellAt(position).row();
        for (int i=0; i<table->columns(); i++) {
            int currentChangeId = table->cellAt(row,i).format().property(KoCharacterStyle::ChangeTrackerId).toInt();
            if (!currentChangeId) {
                // Encountered a cell that is not a change
                // So break out of loop and return 0
                changeId = 0;
                break;
            } else {
                // This cell is a changed cell. Continue further.
                if (changeId == 0) {
                    //First cell and it is a changed-cell
                    //Store it and continue
                    changeId = currentChangeId;
                    continue;
                } else {
                    if (currentChangeId == changeId) {
                        //Change found and it is the same as the first change.
                        //continue looking
                        continue;
                    } else {
                        //A Change found but not same as the first change
                        //Break-out of loop and return 0
                        changeId = 0;
                        break;
                    }
                }
            }
        }
    }
    return changeId;
}

//Check if the whole of table column is a part of a single change
//If so return the changeId else return 0
int KoTextWriter::Private::checkForTableColumnChange(int position)
{
    int changeId = 0;
    QTextCursor cursor(document);
    cursor.setPosition(position);
    QTextTable *table = cursor.currentTable();

    if (table) {
        int column = table->cellAt(position).column();
        for (int i=0; i<table->rows(); i++) {
            int currentChangeId = table->cellAt(i,column).format().property(KoCharacterStyle::ChangeTrackerId).toInt();
            if (!currentChangeId) {
                // Encountered a cell that is not a change
                // So break out of loop and return 0
                changeId = 0;
                break;
            } else {
                // This cell is a changed cell. Continue further.
                if (changeId == 0) {
                    //First cell and it is a changed-cell
                    //Store it and continue
                    changeId = currentChangeId;
                    continue;
                } else {
                    if (currentChangeId == changeId) {
                        //Change found and it is the same as the first change.
                        //continue looking
                        continue;
                    } else {
                        //A Change found but not same as the first change
                        //Break-out of loop and return 0
                        changeId = 0;
                        break;
                    }
                }
            }
        }
    }
    return changeId;
}

void KoTextWriter::Private::saveTable(QTextTable *table, QHash<QTextList *, QString> &listStyles)
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


    int changeId = openTagRegion(table->firstCursorPosition().position(), KoTextWriter::Private::Table, tableTagInformation);

    for (int c = 0 ; c < table->columns() ; c++) {
        KoTableColumnStyle columnStyle = tcarManager.columnStyle(c);
        int repetition = 0;

        for (; repetition < (table->columns() - c) ; repetition++)
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

        int changeId = openTagRegion(table->cellAt(0,c).firstCursorPosition().position(), KoTextWriter::Private::TableColumn, tableColumnInformation);
        closeTagRegion(changeId);
        c += repetition;
    }

    if (numberHeadingRows)
        writer->startElement("table:table-header-rows");

    for (int r = 0 ; r < table->rows() ; r++) {
        TagInformation tableRowInformation;
        tableRowInformation.setTagName("table:table-row");
        KoTableRowStyle rowStyle = tcarManager.rowStyle(r);
        if (!rowStyle.isEmpty())
        {
            QString rowStyleName = saveTableRowStyle(rowStyle, r, tableStyleName);
            tableRowInformation.addAttribute("table:style-name", rowStyleName);
        }
        int changeId = openTagRegion(table->cellAt(r,0).firstCursorPosition().position(), KoTextWriter::Private::TableRow, tableRowInformation);

        for (int c = 0 ; c < table->columns() ; c++) {
            QTextTableCell cell = table->cellAt(r, c);
            int changeId = 0;

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
                changeId = openTagRegion(table->cellAt(r,c).firstCursorPosition().position(), KoTextWriter::Private::TableCell, tableCellInformation);
                writeBlocks(table->document(), cell.firstPosition(), cell.lastPosition(), listStyles, table);
            } else {
                tableCellInformation.setTagName("table:covered-table-cell");
                if (cell.format().boolProperty(KoTableCellStyle::CellIsProtected))
                {
                    tableCellInformation.addAttribute("table:protected", "true");
                }
                changeId = openTagRegion(table->cellAt(r,c).firstCursorPosition().position(), KoTextWriter::Private::TableCell, tableCellInformation);
            }
            closeTagRegion(changeId);
        }
        closeTagRegion(changeId);

        if (r + 1 == numberHeadingRows) {
            writer->endElement();   // table:table-header-rows
            writer->startElement("table:table-rows");
        }
    }

    if (numberHeadingRows)
        writer->endElement();   // table:table-rows
    closeTagRegion(changeId);

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

    if (changeTracker && changeTracker->saveFormat() == KoChangeTracker::DELTAXML) {
        if ((level == 1) && (!deleteMergeRegionOpened) && !headingLevel) {
            QTextBlock listBlock = block;
            do {
                int endBlockNumber = checkForDeleteMerge(listBlock);
                if (endBlockNumber != -1) {
                    deleteMergeEndBlockNumber = endBlockNumber;
                    deleteMergeRegionOpened = true;
                    openSplitMergeRegion();
                    break;
                }
                listBlock = listBlock.next();
            } while(textDocument.list(listBlock) == list);
        }
    }

    bool closeDelMergeRegion = false;
    if (changeTracker && changeTracker->saveFormat() == KoChangeTracker::DELTAXML) {
        if ((level == 1) && (deleteMergeRegionOpened) && !headingLevel) {
            QTextBlock listBlock = block;
            do {
                if (listBlock.blockNumber() == deleteMergeEndBlockNumber) {
                    closeDelMergeRegion = true;
                }
                listBlock = listBlock.next();
            } while(textDocument.list(listBlock) == list);
        }
    }

    bool splitRegionOpened = false;

    bool listStarted = false;
    int listChangeId = 0;
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

        listChangeId = openTagRegion(block.position(), KoTextWriter::Private::List, listTagInformation);
    }

    if (!headingLevel) {
      int splitEndBlockNumber = -1;
      do {
            if (numberedParagraphLevel) {
                TagInformation paraTagInformation;
                paraTagInformation.setTagName("text:numbered-paragraph");
                paraTagInformation.addAttribute("text:level", numberedParagraphLevel);
                paraTagInformation.addAttribute("text:style-name", listStyles.value(textList));

                QString listId = numberedParagraphListIds.value(list, QString("list-%1").arg(createXmlId()));
                numberedParagraphListIds.insert(list, listId);
                paraTagInformation.addAttribute("text:list-id", listId);

                int changeId = openTagRegion(block.position(), KoTextWriter::Private::NumberedParagraph, paraTagInformation);
                writeBlocks(textDocument.document(), block.position(), block.position() + block.length() - 1, listStyles, currentTable, textList);
                closeTagRegion(changeId);
            } else {
                if (changeTracker && changeTracker->saveFormat() == KoChangeTracker::DELTAXML) {
                    int endBlockNumber = checkForSplit(block);
                    if (!deleteMergeRegionOpened && !splitRegionOpened && (endBlockNumber != -1)) {
                        openSplitMergeRegion();
                        splitRegionOpened = true;
                        splitEndBlockNumber = endBlockNumber;
                    }
                }

                const bool listHeader = blockFormat.boolProperty(KoParagraphStyle::IsListHeader)|| blockFormat.boolProperty(KoParagraphStyle::UnnumberedListItem);
                int listItemChangeId;
                TagInformation listItemTagInformation;
                listItemTagInformation.setTagName(listHeader ? "text:list-header" : "text:list-item");
                if (block.blockFormat().hasProperty(KoParagraphStyle::ListStartValue)) {
                    int startValue = block.blockFormat().intProperty(KoParagraphStyle::ListStartValue);
                    listItemTagInformation.addAttribute("text:start-value", startValue);
                }
                if (textList == topLevelTextList) {
                    listItemChangeId = openTagRegion(block.position(), KoTextWriter::Private::ListItem, listItemTagInformation);
                } else {
                    // This is a sub-list. So check for a list-change
                    listItemChangeId = openTagRegion(block.position(), KoTextWriter::Private::List, listItemTagInformation);
                }

                if (KoListStyle::isNumberingStyle(textList->format().style())) {
                    if (KoTextBlockData *blockData = dynamic_cast<KoTextBlockData *>(block.userData())) {
                        writer->startElement("text:number", false);
                        writer->addTextSpan(blockData->counterText());
                        writer->endElement();
                    }
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

                closeTagRegion(listItemChangeId);

                if (changeTracker && changeTracker->saveFormat() == KoChangeTracker::DELTAXML) {
                    if (splitRegionOpened && (block.blockNumber() == splitEndBlockNumber)) {
                        splitRegionOpened = false;
                        splitEndBlockNumber = -1;
                        closeSplitMergeRegion();
                        postProcessListItemSplit(block.blockFormat().intProperty(KoCharacterStyle::ChangeTrackerId));
                    }
                }
            }
            block = block.next();
            blockFormat = block.blockFormat();
            headingLevel = blockFormat.intProperty(KoParagraphStyle::OutlineLevel);
            numberedParagraphLevel = blockFormat.intProperty(KoParagraphStyle::ListLevel);
            textList = block.textList();
        } while ((textDocument.list(block) == list) && (KoList::level(block) >= topListLevel));
    }

    if (listStarted) {
        closeTagRegion(listChangeId);
    }

    if (closeDelMergeRegion && (changeTracker && changeTracker->saveFormat() == KoChangeTracker::DELTAXML)) {
        closeSplitMergeRegion();
        deleteMergeRegionOpened = false;
        deleteMergeEndBlockNumber = -1;
        postProcessDeleteMergeXml();
    }

    return block;
}

void KoTextWriter::Private::postProcessListItemSplit(int changeId)
{
    QString change = changeTransTable.value(changeId);

    QString generatedXmlString(generatedXmlArray);

    //Add the name-space definitions so that this can be parsed
    addNameSpaceDefinitions(generatedXmlString);

    //Now Parse the generatedXML and if successful generate the final output
    QString errorMsg;
    int errorLine, errorColumn;
    KoXmlDocument doc;

    QXmlStreamReader reader(generatedXmlString);
    reader.setNamespaceProcessing(true);

    bool ok = doc.setContent(&reader, &errorMsg, &errorLine, &errorColumn);

    if (!ok)
        return;

    QString outputXml;
    QTextStream outputXmlStream(&outputXml);

    KoXmlElement rootElement = doc.documentElement();
    KoXmlElement listItemElement = rootElement.firstChild().toElement();
    removeLeavingContentStart(outputXmlStream, listItemElement, change, 1);

    KoXmlElement pElement = rootElement.firstChild().firstChild().toElement();
    removeLeavingContentStart(outputXmlStream, pElement, change, 2);

    KoXmlElement childElement;
    forEachElement(childElement, rootElement) {
        if (childElement.localName() == "list-item") {
            insertAroundContent(outputXmlStream, childElement, change);
            KoXmlElement pElement = childElement.firstChild().toElement();
            insertAroundContent(outputXmlStream, pElement, change);
            writeNode(outputXmlStream, pElement, true);
            outputXmlStream << "</text:p>";
            outputXmlStream << "</text:list-item>";
        } else {
            writeNode(outputXmlStream, pElement);
        }
    }

    removeLeavingContentEnd(outputXmlStream, 2);
    removeLeavingContentEnd(outputXmlStream, 1);
    writer->addCompleteElement(outputXml.toUtf8());
}


int KoTextWriter::Private::checkForSplit(const QTextBlock &block)
{
    return checkForMergeOrSplit(block, KoGenChange::InsertChange);
}

int KoTextWriter::Private::checkForDeleteMerge(const QTextBlock &block)
{
    return checkForMergeOrSplit(block, KoGenChange::DeleteChange);
}

int KoTextWriter::Private::checkForMergeOrSplit(const QTextBlock &block, KoGenChange::Type changeType)
{
    QTextBlock endBlock = block;
    int endBlockNumber = -1;

    int splitMergeChangeId = 0, changeId = 0;
    do {
        if (!endBlock.next().isValid())
            break;

        int nextBlockChangeId;
        QTextTable *currentTable;

        if ((currentTable = QTextCursor(endBlock.next()).currentTable())) {
            nextBlockChangeId = currentTable->format().intProperty(KoCharacterStyle::ChangeTrackerId);
        } else {
            nextBlockChangeId = endBlock.next().blockFormat().property(KoCharacterStyle::ChangeTrackerId).toInt();
        }

        if (changeTracker && changeTracker->isDuplicateChangeId(nextBlockChangeId)) {
            nextBlockChangeId = changeTracker->originalChangeId(nextBlockChangeId);
        }

        if (!changeId) {
            splitMergeChangeId = changeId = nextBlockChangeId;
            if ((changeId) && (changeTracker && changeTracker->elementById(nextBlockChangeId)->getChangeType() == changeType)) {
                endBlock = endBlock.next();
            } else {
                changeId = 0;
            }
        } else {
            if (nextBlockChangeId == changeId) {
                endBlock = endBlock.next();
            } else {
                changeId = 0;
            }
        }
    } while(changeId);

    if ((endBlock.blockNumber() != block.blockNumber()) && (endBlock.text().length()) && !(QTextCursor(endBlock).currentTable())) {
        //Check that the last fragment of this block is not a part of this change. If so, it is not a merge or a split
        QTextFragment lastFragment = (--(endBlock.end())).fragment();
        QTextCharFormat lastFragmentFormat = lastFragment.charFormat();
        int lastFragmentChangeId = lastFragmentFormat.intProperty(KoCharacterStyle::ChangeTrackerId);
        if (changeTracker && changeTracker->isDuplicateChangeId(lastFragmentChangeId)) {
            lastFragmentChangeId = changeTracker->originalChangeId(lastFragmentChangeId);
        }

        if (lastFragmentChangeId != splitMergeChangeId) {
            endBlockNumber = endBlock.blockNumber();
        }
    }

    return endBlockNumber;
}

void KoTextWriter::Private::openSplitMergeRegion()
{
    //Save the current writer
    oldXmlWriter = writer;

    //Create a new KoXmlWriter pointing to a QBuffer
    generatedXmlArray.clear();
    generatedXmlBuffer.setBuffer(&generatedXmlArray);
    newXmlWriter = new KoXmlWriter(&generatedXmlBuffer);

    //Set our xmlWriter as the writer to be used
    writer = newXmlWriter;
    context.setXmlWriter(*newXmlWriter);
}

void KoTextWriter::Private::closeSplitMergeRegion()
{
    //delete the new writer
    delete newXmlWriter;

    //Restore the actual xml writer
    writer = oldXmlWriter;
    context.setXmlWriter(*oldXmlWriter);
}

void KoTextWriter::Private::postProcessDeleteMergeXml()
{
    QString generatedXmlString(generatedXmlArray);

    //Add the name-space definitions so that this can be parsed
    addNameSpaceDefinitions(generatedXmlString);

    //Now Parse the generatedXML and if successful generate the final output
    QString errorMsg;
    int errorLine, errorColumn;
    KoXmlDocument doc;

    QXmlStreamReader reader(generatedXmlString);
    reader.setNamespaceProcessing(true);

    bool ok = doc.setContent(&reader, &errorMsg, &errorLine, &errorColumn);
    if (ok) {
        //Generate the final XML output and save it
        QString outputXml;
        QTextStream outputXmlStream(&outputXml);
        generateFinalXml(outputXmlStream, doc.documentElement());
        writer->addCompleteElement(outputXml.toUtf8());
    }
}

void KoTextWriter::Private::addNameSpaceDefinitions(QString &generatedXmlString)
{
    //Generate the name-space definitions so that it can be parsed. Like what is office:text, office:delta etc
    QString nameSpaceDefinitions;
    QTextStream nameSpacesStream(&nameSpaceDefinitions);

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
    nameSpacesStream << "xmlns:delta=\"" << KoXmlNS::delta << "\" ";
    nameSpacesStream << "xmlns:split=\"" << KoXmlNS::split << "\" ";
    nameSpacesStream << "xmlns:ac=\"" << KoXmlNS::ac << "\" ";
    nameSpacesStream << ">";

    generatedXmlString.prepend(nameSpaceDefinitions);
    generatedXmlString.append("</generated-xml>");
}

void KoTextWriter::Private::generateFinalXml(QTextStream &outputXmlStream, const KoXmlElement &element)
{
    QString firstChild = element.firstChild().toElement().localName();
    KoXmlElement secondChildElement = element.firstChild().nextSibling().toElement();
    QString secondChild;

    do {
        secondChild = secondChildElement.localName();
        secondChildElement = secondChildElement.nextSibling().toElement();
    } while (secondChild == "removed-content");

    if ((firstChild == "p") && (secondChild == "h")) {
        handleParagraphOrHeaderMerge(outputXmlStream, element);
    } else if ((firstChild == "h") && (secondChild == "p")) {
        handleParagraphOrHeaderMerge(outputXmlStream, element);
    } else if ((firstChild == "p") && (secondChild == "p")) {
        handleParagraphOrHeaderMerge(outputXmlStream, element);
    } else if ((firstChild == "h") && (secondChild == "h")) {
        handleParagraphOrHeaderMerge(outputXmlStream, element);
    } else if ((firstChild == "p") && (secondChild == "list")) {
        handleParagraphWithListItemMerge(outputXmlStream, element);
    } else if ((firstChild == "h") && (secondChild == "list")) {
        handleParagraphWithListItemMerge(outputXmlStream, element);
    } else if ((firstChild == "list") && (secondChild == "p")) {
        handleListItemWithParagraphMerge(outputXmlStream, element);
    } else if ((firstChild == "list") && (secondChild == "h")) {
        handleListItemWithParagraphMerge(outputXmlStream, element);
    } else if ((firstChild == "list") && (secondChild == "list")) {
        handleListWithListMerge(outputXmlStream, element);
    } else if ((firstChild == "list") && (secondChild.isEmpty())) {
        handleListItemWithListItemMerge(outputXmlStream, element);
    } else {
        //Not Possible
    }

}

void KoTextWriter::Private::removeLeavingContentStart(QTextStream &outputXmlStream, KoXmlElement &element, QString &changeId, int endIdCounter)
{
    outputXmlStream << "<delta:remove-leaving-content-start";
    outputXmlStream << " delta:removal-change-idref=" << "\"" << changeId << "\"";
    outputXmlStream << " delta:end-element-idref=" << "\"end" << endIdCounter << "\">";

    outputXmlStream << "<text:" << element.localName();
    writeAttributes(outputXmlStream, element);
    outputXmlStream << "/>";

    outputXmlStream << "</delta:remove-leaving-content-start>";
}

void KoTextWriter::Private::removeLeavingContentEnd(QTextStream &outputXmlStream, int endIdCounter)
{
    outputXmlStream << "<delta:remove-leaving-content-end delta:end-element-id=\"end" << endIdCounter << "\"/>";
}

void KoTextWriter::Private::insertAroundContent(QTextStream &outputXmlStream, KoXmlElement &element, QString &changeId)
{
    outputXmlStream << "<text:" << element.localName() << " delta:insertion-change-idref=" << "\"" << changeId << "\"";
    outputXmlStream << " delta:insertion-type=\"insert-around-content\"";
    writeAttributes(outputXmlStream, element);
    outputXmlStream << ">";
}

void KoTextWriter::Private::handleParagraphOrHeaderMerge(QTextStream &outputXmlStream, const KoXmlElement &element)
{
    // Find the first child of the element
    // This is needed because we need to determine whether the final element is going to a <p> or a <h>
    KoXmlElement firstChildElement = element.firstChild().toElement();
    QString firstChild = firstChildElement.localName();

    // Find the Change-id
    KoXmlElement removedContentElement = firstChildElement.lastChild().toElement();
    QString changeId = removedContentElement.attributeNS(KoXmlNS::delta, "removal-change-idref");

    outputXmlStream << "<text:" << firstChild;
    writeAttributes(outputXmlStream, firstChildElement);
    outputXmlStream << ">";

    for (KoXmlNode node = firstChildElement.firstChild(); !node.isNull(); node = node.nextSibling()) {
        if (node.isElement() && node.toElement().localName() == "removed-content" && node.nextSibling().isNull()) {
            outputXmlStream << "<delta:merge delta:removal-change-idref=\"" << changeId << "\">";
            outputXmlStream << "<delta:leading-partial-content>";
            writeNode(outputXmlStream, node, true);
            outputXmlStream << "</delta:leading-partial-content>";
        } else {
            writeNode(outputXmlStream, node);
        }
    }

    outputXmlStream << "<delta:intermediate-content>";
    KoXmlElement mergeEndElement = firstChildElement.nextSibling().toElement();
    while (mergeEndElement.localName() == "removed-content") {
        writeNode(outputXmlStream, mergeEndElement, true);
        mergeEndElement = mergeEndElement.nextSibling().toElement();
    }
    outputXmlStream << "</delta:intermediate-content>";

    for (KoXmlNode node = mergeEndElement.firstChild(); !node.isNull(); node = node.nextSibling()) {
        if (node.isElement() && node.toElement().localName() == "removed-content" && node.previousSibling().isNull()) {
            outputXmlStream << "<delta:trailing-partial-content>";
            outputXmlStream << "<text:" << mergeEndElement.localName();
            writeAttributes(outputXmlStream, mergeEndElement);
            outputXmlStream << ">";
            writeNode(outputXmlStream, node, true);
            outputXmlStream << "</text:" << mergeEndElement.localName() << ">";
            outputXmlStream << "</delta:trailing-partial-content>";
            outputXmlStream << "</delta:merge>";
        } else {
            writeNode(outputXmlStream, node);
        }
    }

    outputXmlStream << "</text:" << firstChild << ">";
}

void KoTextWriter::Private::handleParagraphWithHeaderMerge(QTextStream &outputXmlStream, const KoXmlElement &element)
{
    // Find the first child of the element
    // This is needed because we need to determine whether the final element is going to a <p> or a <h>
    KoXmlElement firstChildElement = element.firstChild().toElement();
    QString firstChild = firstChildElement.localName();

    // Find the Change-id
    KoXmlElement removedContentElement = firstChildElement.lastChild().toElement();
    QString changeId = removedContentElement.attributeNS(KoXmlNS::delta, "removal-change-idref");

    //Start generating the XML
    insertAroundContent(outputXmlStream, firstChildElement, changeId);

    //Start a counter for end-element-idref
    int endIdCounter = 1;

    KoXmlElement childElement;
    forEachElement (childElement, element) {
        if (childElement.localName() == "removed-content") {
            writeNode(outputXmlStream, childElement, false);
        } else {
            removeLeavingContentStart(outputXmlStream, childElement, changeId, endIdCounter);
            writeNode(outputXmlStream, childElement, true);
            removeLeavingContentEnd(outputXmlStream, endIdCounter);
            endIdCounter++;
        }
    }

    outputXmlStream << "</text:" << firstChild << ">";

}

void KoTextWriter::Private::handleParagraphWithListItemMerge(QTextStream &outputXmlStream, const KoXmlElement &element)
{
    // Find the first child of the element
    // This is needed because we need to determine whether the final element is going to a <p> or a <h>
    KoXmlElement firstChildElement = element.firstChild().toElement();
    QString firstChild = firstChildElement.localName();

    // Find the Change-id
    KoXmlElement removedContentElement = firstChildElement.lastChild().toElement();
    QString changeId = removedContentElement.attributeNS(KoXmlNS::delta, "removal-change-idref");

    //Start generating the XML
    insertAroundContent(outputXmlStream, firstChildElement, changeId);

    //Start a counter for end-element-idref
    int endIdCounter = 1;

    KoXmlElement childElement;
    forEachElement (childElement, element) {
        if (childElement.localName() == "removed-content") {
            writeNode(outputXmlStream, childElement, false);
        } else if (childElement.localName() == firstChild) {
            removeLeavingContentStart(outputXmlStream, childElement, changeId, endIdCounter);
            writeNode(outputXmlStream, childElement, true);
            removeLeavingContentEnd(outputXmlStream, endIdCounter);
            endIdCounter++;
        } else if (childElement.localName() == "list"){
            generateListForPWithListMerge(outputXmlStream, childElement, firstChild, changeId, endIdCounter, true);
        }
    }
}

void KoTextWriter::Private::generateListForPWithListMerge(QTextStream &outputXmlStream, KoXmlElement &element,
                                                          QString &mergeResultElement, QString &changeId, int &endIdCounter,
                                                          bool removeLeavingContent)
{
    int listEndIdCounter = endIdCounter;
    if (removeLeavingContent) {
        endIdCounter++;
        removeLeavingContentStart(outputXmlStream, element, changeId, listEndIdCounter);
    } else {
        outputXmlStream << "<text:" << element.localName();
        writeAttributes(outputXmlStream, element);
        outputXmlStream << ">";
    }

    bool tagTypeChangeEnded = false;
    bool listStarted = false;
    KoXmlElement childElement;
    forEachElement (childElement, element) {
        if (childElement.localName() == "removed-content") {
            writeNode(outputXmlStream, childElement, false);
        } else if ((childElement.localName() == "list-item") || (childElement.localName() == "list-header")) {
            generateListItemForPWithListMerge(outputXmlStream, childElement, mergeResultElement,
                                              changeId, endIdCounter, !tagTypeChangeEnded);
            if (!tagTypeChangeEnded) {
                tagTypeChangeEnded = true;
                if (childElement != element.lastChild().toElement()) {
                    listStarted = true;
                    insertAroundContent(outputXmlStream, element, changeId);
                }
            }
        } else {
            //Not Possible
        }
    }
    if (listStarted)
        outputXmlStream << "</text:list>";
    if (removeLeavingContent) {
        removeLeavingContentEnd(outputXmlStream, listEndIdCounter);
    } else {
        outputXmlStream << "</text:" << element.localName() << ">";
    }
}

void KoTextWriter::Private::generateListItemForPWithListMerge(QTextStream &outputXmlStream, KoXmlElement &element,
                                                              QString &mergeResultElement, QString &changeId, int &endIdCounter,
                                                              bool removeLeavingContent)
{
    int listItemEndIdCounter = endIdCounter;

    if (removeLeavingContent) {
        endIdCounter++;
        removeLeavingContentStart(outputXmlStream, element, changeId, listItemEndIdCounter);
    } else {
        outputXmlStream << "<text:" << element.localName();
        writeAttributes(outputXmlStream, element);
        outputXmlStream << ">";
    }

    KoXmlElement childElement;
    forEachElement (childElement, element) {
        if (childElement.localName() == "removed-content") {
            writeNode(outputXmlStream, childElement, false);
        } else if (childElement.localName() == "p") {
            if (removeLeavingContent) {
                int paragraphEndIdCounter = endIdCounter;
                endIdCounter++;
                removeLeavingContentStart(outputXmlStream, childElement, changeId, paragraphEndIdCounter);
                writeNode(outputXmlStream, childElement, true);
                removeLeavingContentEnd(outputXmlStream, paragraphEndIdCounter);
                outputXmlStream << "</text:" << mergeResultElement << ">";
            } else {
                writeNode(outputXmlStream, childElement, false);
            }
        } else if (childElement.localName() == "list") {
            generateListForPWithListMerge(outputXmlStream, childElement, mergeResultElement, changeId, endIdCounter, removeLeavingContent);
        } else {
            //Not Possible
        }
    }

    if (removeLeavingContent) {
        removeLeavingContentEnd(outputXmlStream, listItemEndIdCounter);
    } else {
        outputXmlStream << "</text:" << element.localName() << ">";
    }
}

void KoTextWriter::Private::handleListItemWithParagraphMerge(QTextStream &outputXmlStream, const KoXmlElement &element)
{
    deleteStartDepth = 0;

    // Find the first <p> so that we can get the change-id
    KoXmlElement paragraphElement;
    forEachElement(paragraphElement, element) {
        if (paragraphElement.localName() == "p") {
            break;
        }
    }

    // Find the Change-id
    KoXmlElement removedContentElement = paragraphElement.firstChild().toElement();
    QString changeId = removedContentElement.attributeNS(KoXmlNS::delta, "removal-change-idref");

    int endIdCounter = 1;

    KoXmlElement childElement;
    forEachElement(childElement, element) {
        if (childElement.localName() == "list") {
            generateListForListWithPMerge(outputXmlStream, childElement, changeId, endIdCounter, true);
        } else if (childElement.localName() == "removed-content") {
            writeNode(outputXmlStream, childElement, false);
        } else if (childElement.localName() == "p") {
            int paragraphEndIdCounter = endIdCounter;
            endIdCounter++;
            removeLeavingContentStart(outputXmlStream, childElement, changeId, paragraphEndIdCounter);
            writeNode(outputXmlStream, childElement, true);
            removeLeavingContentEnd(outputXmlStream, paragraphEndIdCounter);
            outputXmlStream << "</text:p>";

            for (int i=0; i < deleteStartDepth; i++) {
                outputXmlStream << "</text:list-item>";
                outputXmlStream << "</text:list>";
            }
            break;
        } else {
        }
    }

}

void KoTextWriter::Private::generateListForListWithPMerge(QTextStream &outputXmlStream, KoXmlElement &element,
                                                          QString &changeId, int &endIdCounter,
                                                          bool removeLeavingContent)
{
    static int listDepth = 0;
    listDepth++;

    if (!deleteStartDepth) {
        KoXmlElement childElement;
        forEachElement(childElement, element) {
            if ((childElement.localName() == "list-item") || (childElement.localName() == "list-header")) {
                bool startOfDeleteMerge = checkForDeleteStartInListItem(childElement, false);
                if (startOfDeleteMerge) {
                    deleteStartDepth = listDepth;
                }
            }
        }
    }

    int listEndIdCounter = endIdCounter;
    if (removeLeavingContent) {
        endIdCounter++;
        removeLeavingContentStart(outputXmlStream, element, changeId, listEndIdCounter);
        insertAroundContent(outputXmlStream, element, changeId);
    } else {
        outputXmlStream << "<text:" << element.localName();
        writeAttributes(outputXmlStream, element);
        outputXmlStream << ">";
    }

    KoXmlElement childElement;
    forEachElement(childElement, element) {
        if ((childElement.localName() == "list-item") || (childElement.localName() == "list-header")) {
            bool startOfDeleteMerge = checkForDeleteStartInListItem(childElement);
            generateListItemForListWithPMerge(outputXmlStream, childElement, changeId, endIdCounter, startOfDeleteMerge);
        } else if (childElement.localName() == "removed-content") {
            writeNode(outputXmlStream, childElement, false);
        }
    }

    if (removeLeavingContent) {
        removeLeavingContentEnd(outputXmlStream, listEndIdCounter);
    } else {
        outputXmlStream << "</text:" << element.localName() << ">";
    }
}

void KoTextWriter::Private::generateListItemForListWithPMerge(QTextStream &outputXmlStream, KoXmlElement &element,
                                                              QString &changeId, int &endIdCounter, bool removeLeavingContent)
{
    if (!removeLeavingContent) {
        writeNode(outputXmlStream, element, false);
    } else {
        int listItemEndIdCounter = endIdCounter;
        endIdCounter++;
        insertAroundContent(outputXmlStream, element, changeId);
        removeLeavingContentStart(outputXmlStream, element, changeId, listItemEndIdCounter);

        KoXmlElement childElement;
        forEachElement (childElement, element) {
            if (childElement.localName() == "p") {
                int paragraphEndIdCounter = endIdCounter;
                endIdCounter++;
                insertAroundContent(outputXmlStream, childElement, changeId);
                removeLeavingContentStart(outputXmlStream, childElement, changeId, paragraphEndIdCounter);
                writeNode(outputXmlStream, childElement, true);
                removeLeavingContentEnd(outputXmlStream, paragraphEndIdCounter);
            } else if(childElement.localName() == "list") {
                generateListForListWithPMerge(outputXmlStream, childElement, changeId, endIdCounter, removeLeavingContent);
            }
        }
        removeLeavingContentEnd(outputXmlStream, listItemEndIdCounter);
    }
}

bool KoTextWriter::Private::checkForDeleteStartInListItem(KoXmlElement &element, bool checkRecursively)
{
    bool returnValue = false;
    KoXmlElement childElement;
    forEachElement(childElement, element) {
        if (childElement.localName() == "p") {
            if (childElement.lastChild().toElement().localName() == "removed-content") {
                returnValue = true;
                break;
            }
        } else if ((childElement.localName() == "list") && (checkRecursively)) {
            KoXmlElement listItem;
            forEachElement(listItem, childElement) {
                returnValue = checkForDeleteStartInListItem(listItem);
                if (returnValue)
                    break;
            }
        } else {
        }

        if (returnValue)
            break;
    }

    return returnValue;
}

void KoTextWriter::Private::handleListWithListMerge(QTextStream &outputXmlStream, const KoXmlElement &element)
{
    int endIdCounter = 1;

    KoXmlElement listElement = element.firstChild().toElement();
    QString changeId = findChangeIdForListItemMerge(listElement);

    KoXmlElement firstChildElement = element.firstChild().toElement();
    generateListForListItemMerge(outputXmlStream, firstChildElement, changeId, endIdCounter, true, false);

    KoXmlElement secondChildElement = element.firstChild().nextSibling().toElement();
    QString secondChild = secondChildElement.localName();
    while (secondChild == "removed-content")
    {
        writeNode(outputXmlStream, secondChildElement, false);
        secondChildElement = secondChildElement.nextSibling().toElement();
        secondChild = secondChildElement.localName();
    };

    generateListForListItemMerge(outputXmlStream, secondChildElement, changeId, endIdCounter, false, true);
}

void KoTextWriter::Private::handleListItemWithListItemMerge(QTextStream &outputXmlStream, const KoXmlElement &element)
{
    int endIdCounter = 1;
    KoXmlElement listElement = element.firstChild().toElement();
    QString changeId = findChangeIdForListItemMerge(listElement);

    generateListForListItemMerge(outputXmlStream, listElement, changeId, endIdCounter, false, false);
}

void KoTextWriter::Private::generateListForListItemMerge(QTextStream &outputXmlStream, KoXmlElement &element,
                                                         QString &changeId, int &endIdCounter, bool listMergeStart, bool listMergeEnd)
{
    int listEndIdCounter = endIdCounter;
    if (listMergeStart || listMergeEnd) {
        endIdCounter++;
        removeLeavingContentStart(outputXmlStream, element, changeId, listEndIdCounter);

        if (listMergeStart) {
            insertAroundContent(outputXmlStream, element, changeId);
        }

    } else {
        outputXmlStream << "<text:" << element.localName();
        writeAttributes(outputXmlStream, element);
        outputXmlStream << ">";
    }

    KoXmlElement childElement;
    bool deleteRangeStarted = false;

    if (listMergeEnd) {
        deleteRangeStarted = true;
    }

    forEachElement(childElement, element) {
        if ((childElement.localName() == "list-item") || (childElement.localName() == "list-header")) {
            if (!deleteRangeStarted) {
                deleteRangeStarted = checkForDeleteStartInListItem(childElement);
                generateListItemForListItemMerge(outputXmlStream, childElement, changeId, endIdCounter, deleteRangeStarted, false);
            } else {
                bool endOfDeleteRange = checkForDeleteEndInListItem(childElement);
                deleteRangeStarted = !endOfDeleteRange;
                generateListItemForListItemMerge(outputXmlStream, childElement, changeId, endIdCounter, false, endOfDeleteRange);
            }
        } else if (childElement.localName() == "removed-content") {
            writeNode(outputXmlStream, childElement, false);
        }
    }

    if (listMergeStart || listMergeEnd) {
        removeLeavingContentEnd(outputXmlStream, listEndIdCounter);
        if (listMergeEnd) {
            outputXmlStream << "</text:list>";
        }
    } else {
        outputXmlStream << "</text:list>";
    }
}

void KoTextWriter::Private::generateListItemForListItemMerge(QTextStream &outputXmlStream, KoXmlElement &element,
                                                             QString &changeId, int &endIdCounter, bool listItemMergeStart, bool listItemMergeEnd)
{
    if (!listItemMergeStart && !listItemMergeEnd) {
        writeNode(outputXmlStream, element, false);
    } else {
        int listItemEndIdCounter = endIdCounter;
        endIdCounter++;

        if (listItemMergeStart) {
            insertAroundContent(outputXmlStream, element, changeId);
        }

        removeLeavingContentStart(outputXmlStream, element, changeId, listItemEndIdCounter);

        KoXmlElement childElement;
        forEachElement (childElement, element) {
            if (childElement.localName() == "p") {
                int paragraphEndIdCounter = endIdCounter;
                endIdCounter++;
                if (listItemMergeStart) {
                    insertAroundContent(outputXmlStream, childElement, changeId);
                }

                removeLeavingContentStart(outputXmlStream, childElement, changeId, paragraphEndIdCounter);
                writeNode(outputXmlStream, childElement, true);
                removeLeavingContentEnd(outputXmlStream, paragraphEndIdCounter);

                if (listItemMergeEnd) {
                    outputXmlStream << "</text:" << childElement.localName() << ">";
                }
            } else if(childElement.localName() == "list") {
                generateListForListItemMerge(outputXmlStream, childElement, changeId, endIdCounter, listItemMergeStart ,listItemMergeEnd);
            }
        }

        removeLeavingContentEnd(outputXmlStream, listItemEndIdCounter);
        if (listItemMergeEnd) {
            outputXmlStream << "</text:list-item>";
        }
    }
}

bool KoTextWriter::Private::checkForDeleteEndInListItem(KoXmlElement &element, bool checkRecursively)
{
    bool returnValue = false;
    KoXmlElement childElement;
    forEachElement(childElement, element) {
        if (childElement.localName() == "p") {
            if (childElement.firstChild().toElement().localName() == "removed-content") {
                returnValue = true;
                break;
            }
        } else if ((childElement.localName() == "list") && (checkRecursively)) {
            KoXmlElement listItem;
            forEachElement(listItem, childElement) {
                returnValue = checkForDeleteStartInListItem(listItem);
                if (returnValue)
                    break;
            }
        } else {
        }

        if (returnValue)
            break;
    }

    return returnValue;
}

QString KoTextWriter::Private::findChangeIdForListItemMerge(const KoXmlElement &element)
{
    QString changeId;

    KoXmlElement listItemElement;
    forEachElement (listItemElement, element) {
        if ((listItemElement.localName() == "list-item") || (listItemElement.localName() == "list-header")) {
            KoXmlElement childElement;
            forEachElement (childElement, listItemElement) {
                if (childElement.localName() == "p") {
                    KoXmlElement removedContentElement = childElement.lastChild().toElement();
                    if (removedContentElement.localName() == "removed-content") {
                        changeId = removedContentElement.attributeNS(KoXmlNS::delta, "removal-change-idref");
                        break;
                    }
                } else if (childElement.localName() == "list") {
                    changeId = findChangeIdForListItemMerge(childElement);
                    break;
                } else {
                    // Not Needed
                }
            }
        }
    }

    return changeId;
}

void KoTextWriter::Private::writeAttributes(QTextStream &outputXmlStream, KoXmlElement &element)
{
    QList<QPair<QString, QString> > attributes = element.attributeFullNames();

    QPair<QString, QString> attributeNamePair;
    foreach (attributeNamePair, attributes) {
        if (attributeNamePair.first == KoXmlNS::text) {
            outputXmlStream << " text:" << attributeNamePair.second << "=";
            outputXmlStream << "\"" << element.attributeNS(KoXmlNS::text, attributeNamePair.second) << "\"";
        } else if (attributeNamePair.first == KoXmlNS::delta) {
            outputXmlStream << " delta:" << attributeNamePair.second << "=";
            outputXmlStream << "\"" << element.attributeNS(KoXmlNS::delta, attributeNamePair.second) << "\"";
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
