/* This file is part of the KDE project
 * Copyright (C) 2008 Pierre Stirnweiss \pierre.stirnweiss_calligra@gadz.org>
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

#include "KoChangeTracker.h"

//Calligra includes
#include "styles/KoCharacterStyle.h"
#include "KoChangeTrackerElement.h"
#include <KoXmlReader.h>
#include <KoXmlNS.h>
#include <KoInlineTextObjectManager.h>
#include <KoTextDocument.h>
#include <KoList.h>
#include <KoListStyle.h>
#include <KoParagraphStyle.h>
#include <KoGenChanges.h>
#include <KoFormatChangeInformation.h>
#include <kundo2magicstring.h>

//KDE includes
#include "TextDebug.h"
#include <klocalizedstring.h>

//Qt includes
#include <QColor>
#include <QList>
#include <QString>
#include <QHash>
#include <QMultiHash>
#include <QTextCursor>
#include <QTextFormat>
#include <QTextDocument>
#include <QTextDocumentFragment>
#include <QTextList>
#include <QTextTable>
#include <QDateTime>
#include <QLocale>

class Q_DECL_HIDDEN KoChangeTracker::Private
{
public:
    Private()
      : changeId(1),
        recordChanges(false),
        displayChanges(false),
        insertionBgColor(101,255,137),
        deletionBgColor(255,185,185),
        formatChangeBgColor(195,195,255),
        changeSaveFormat(UNKNOWN)

    {
    }
    ~Private() { }

    QMultiHash<int, int> children;
    QMultiHash<int, int> duplicateIds;
    QHash<int, int> parents;
    QHash<int, KoChangeTrackerElement *> changes;
    QHash<QString, int> loadedChanges;
    QHash<int, KoFormatChangeInformation *> changeInformation;
    QList<int> saveChanges;
    QList<int> acceptedRejectedChanges;
    int changeId;
    bool recordChanges;
    bool displayChanges;
    QColor insertionBgColor, deletionBgColor, formatChangeBgColor;
    QString changeAuthorName;
    KoChangeTracker::ChangeSaveFormat changeSaveFormat;
};

KoChangeTracker::KoChangeTracker(QObject *parent)
    : QObject(parent),
    d(new Private())
{
    d->changeId = 1;
}

KoChangeTracker::~KoChangeTracker()
{
    delete d;
}

void KoChangeTracker::setRecordChanges(bool enabled)
{
    d->recordChanges = enabled;
}

bool KoChangeTracker::recordChanges() const
{
    return d->recordChanges;
}

void KoChangeTracker::setDisplayChanges(bool enabled)
{
    d->displayChanges = enabled;
}

bool KoChangeTracker::displayChanges() const
{
    return d->displayChanges;
}

QString KoChangeTracker::authorName() const
{
    return d->changeAuthorName;
}

void KoChangeTracker::setAuthorName(const QString &authorName)
{
    d->changeAuthorName = authorName;
}

KoChangeTracker::ChangeSaveFormat KoChangeTracker::saveFormat() const
{
    return d->changeSaveFormat;
}

void KoChangeTracker::setSaveFormat(ChangeSaveFormat saveFormat)
{
    d->changeSaveFormat = saveFormat;
}

int KoChangeTracker::getFormatChangeId(const KUndo2MagicString &title, const QTextFormat &format, const QTextFormat &prevFormat, int existingChangeId)
{
    if ( existingChangeId ) {
        d->children.insert(existingChangeId, d->changeId);
        d->parents.insert(d->changeId, existingChangeId);
    }

    KoChangeTrackerElement *changeElement = new KoChangeTrackerElement(title, KoGenChange::FormatChange);
    changeElement->setChangeFormat(format);
    changeElement->setPrevFormat(prevFormat);

    QLocale l;
    changeElement->setDate(l.toString(QDateTime::currentDateTime()).replace(QLocale().decimalPoint(), QString(".")));

    changeElement->setCreator(d->changeAuthorName);

    changeElement->setEnabled(d->recordChanges);

    d->changes.insert(d->changeId, changeElement);

    return d->changeId++;
}

int KoChangeTracker::getInsertChangeId(const KUndo2MagicString &title, int existingChangeId)
{
    if ( existingChangeId ) {
        d->children.insert(existingChangeId, d->changeId);
        d->parents.insert(d->changeId, existingChangeId);
    }

    KoChangeTrackerElement *changeElement = new KoChangeTrackerElement(title, KoGenChange::InsertChange);

    QLocale l;
    changeElement->setDate(l.toString(QDateTime::currentDateTime()).replace(QLocale().decimalPoint(), QString(".")));

    changeElement->setCreator(d->changeAuthorName);

    changeElement->setEnabled(d->recordChanges);

    d->changes.insert(d->changeId, changeElement);

    return d->changeId++;
}

int KoChangeTracker::getDeleteChangeId(const KUndo2MagicString &title, const QTextDocumentFragment &selection, int existingChangeId)
{
    if ( existingChangeId ) {
        d->children.insert(existingChangeId, d->changeId);
        d->parents.insert(d->changeId, existingChangeId);
    }

    KoChangeTrackerElement *changeElement = new KoChangeTrackerElement(title, KoGenChange::DeleteChange);

    QLocale l;
    changeElement->setDate(l.toString(QDateTime::currentDateTime()).replace(QLocale().decimalPoint(), QString(".")));

    changeElement->setCreator(d->changeAuthorName);
    changeElement->setDeleteData(selection);

    changeElement->setEnabled(d->recordChanges);

    d->changes.insert(d->changeId, changeElement);

    return d->changeId++;
}

KoChangeTrackerElement* KoChangeTracker::elementById(int id) const
{
    if (isDuplicateChangeId(id)) {
        id = originalChangeId(id);
    }
    return d->changes.value(id);
}

bool KoChangeTracker::removeById(int id, bool freeMemory)
{
    if (freeMemory) {
      KoChangeTrackerElement *temp = d->changes.value(id);
      delete temp;
    }
    return d->changes.remove(id);
}

bool KoChangeTracker::containsInlineChanges(const QTextFormat &format) const
{
    if (format.property(KoCharacterStyle::ChangeTrackerId).toInt())
        return true;

    return false;
}

int KoChangeTracker::mergeableId(KoGenChange::Type type, const KUndo2MagicString &title, int existingId) const
{
    if (!existingId || !d->changes.value(existingId))
        return 0;

    if (d->changes.value(existingId)->getChangeType() == type && d->changes.value(existingId)->getChangeTitle() == title) {
        return existingId;
    }
    else {
        if (d->parents.contains(existingId)) {
            return mergeableId(type, title, d->parents.value(existingId));
        }
        else {
            return 0;
        }
    }
}

int KoChangeTracker::split(int changeId)
{
    KoChangeTrackerElement *element = new KoChangeTrackerElement(*d->changes.value(changeId));
    d->changes.insert(d->changeId, element);
    return d->changeId++;
}

bool KoChangeTracker::isParent(int testedParentId, int testedChildId) const
{
    if ((testedParentId == testedChildId) && !d->acceptedRejectedChanges.contains(testedParentId))
        return true;
    else if (d->parents.contains(testedChildId))
        return isParent(testedParentId, d->parents.value(testedChildId));
    else
        return false;
}

void KoChangeTracker::setParent(int child, int parent)
{
    if (!d->children.values(parent).contains(child)) {
        d->children.insert(parent, child);
    }
    if (!d->parents.contains(child)) {
        d->parents.insert(child, parent);
    }
}

int KoChangeTracker::parent(int changeId) const
{
    if (!d->parents.contains(changeId))
        return 0;
    if (d->acceptedRejectedChanges.contains(d->parents.value(changeId)))
        return parent(d->parents.value(changeId));
    return d->parents.value(changeId);
}

int KoChangeTracker::createDuplicateChangeId(int existingChangeId)
{
    int duplicateChangeId = d->changeId;
    d->changeId++;

    d->duplicateIds.insert(existingChangeId, duplicateChangeId);

    return duplicateChangeId;
}

bool KoChangeTracker::isDuplicateChangeId(int duplicateChangeId) const
{
    return d->duplicateIds.values().contains(duplicateChangeId);
}

int KoChangeTracker::originalChangeId(int duplicateChangeId) const
{
    int originalChangeId = 0;
    QMultiHash<int, int>::const_iterator i = d->duplicateIds.constBegin();

    while (i != d->duplicateIds.constEnd()) {
        if (duplicateChangeId == i.value()) {
            originalChangeId = i.key();
            break;
        }
        ++i;
    }

    return originalChangeId;
}

void KoChangeTracker::acceptRejectChange(int changeId, bool set)
{
    if (set) {
        if (!d->acceptedRejectedChanges.contains(changeId))
            d->acceptedRejectedChanges.append(changeId);
    }
    else {
        if (d->acceptedRejectedChanges.contains(changeId))
            d->acceptedRejectedChanges.removeAll(changeId);
    }

    d->changes.value(changeId)->setAcceptedRejected(set);
}

bool KoChangeTracker::saveInlineChange(int changeId, KoGenChange &change)
{
    if (!d->changes.contains(changeId))
        return false;

    change.setType(d->changes.value(changeId)->getChangeType());
    change.addChangeMetaData("dc-creator", d->changes.value(changeId)->getCreator());
    change.addChangeMetaData("dc-date", d->changes.value(changeId)->getDate());
    if (d->changes.value(changeId)->hasExtraMetaData())
        change.addChildElement("changeMetaData", d->changes.value(changeId)->getExtraMetaData());

    return true;
}

QMap<int, QString> KoChangeTracker::saveInlineChanges(QMap<int, QString> changeTransTable, KoGenChanges &genChanges)
{
    foreach (int changeId, d->changes.keys()) {

        // return if the id we find in the changetranstable already has a length.
        if (changeTransTable.value(changeId).length()) {
            continue;
        }

        if ((elementById(changeId)->getChangeType() == KoGenChange::DeleteChange) &&
                (saveFormat() == KoChangeTracker::ODF_1_2)) {
            continue;
        }

        KoGenChange change;
        if (saveFormat() == KoChangeTracker::ODF_1_2) {
            change.setChangeFormat(KoGenChange::ODF_1_2);
        } else {
            change.setChangeFormat(KoGenChange::DELTAXML);
        }

        saveInlineChange(changeId, change);
        QString changeName = genChanges.insert(change);
        changeTransTable.insert(changeId, changeName);
    }
    return changeTransTable;
}

void KoChangeTracker::setFormatChangeInformation(int formatChangeId, KoFormatChangeInformation *formatInformation)
{
    d->changeInformation.insert(formatChangeId, formatInformation);
}

KoFormatChangeInformation *KoChangeTracker::formatChangeInformation(int formatChangeId) const
{
    return d->changeInformation.value(formatChangeId);
}

void KoChangeTracker::loadOdfChanges(const KoXmlElement& element)
{
    if (element.namespaceURI() == KoXmlNS::text) {
        KoXmlElement tag;
        forEachElement(tag, element) {
            if (! tag.isNull()) {
                const QString localName = tag.localName();
                if (localName == "changed-region") {
                    KoChangeTrackerElement *changeElement = 0;
                    KoXmlElement region;
                    forEachElement(region, tag) {
                        if (!region.isNull()) {
                            if (region.localName() == "insertion") {
                                changeElement = new KoChangeTrackerElement(kundo2_noi18n(tag.attributeNS(KoXmlNS::text,"id")),KoGenChange::InsertChange);
                            } else if (region.localName() == "format-change") {
                                changeElement = new KoChangeTrackerElement(kundo2_noi18n(tag.attributeNS(KoXmlNS::text,"id")),KoGenChange::FormatChange);
                            } else if (region.localName() == "deletion") {
                                changeElement = new KoChangeTrackerElement(kundo2_noi18n(tag.attributeNS(KoXmlNS::text,"id")),KoGenChange::DeleteChange);
                            }
                            KoXmlElement metadata = region.namedItemNS(KoXmlNS::office,"change-info").toElement();
                            if (!metadata.isNull()) {
                                KoXmlElement date = metadata.namedItem("dc:date").toElement();
                                if (!date.isNull()) {
                                    changeElement->setDate(date.text());
                                }
                                KoXmlElement creator = metadata.namedItem("dc:creator").toElement();
                                if (!date.isNull()) {
                                    changeElement->setCreator(creator.text());
                                }
                                //TODO load comments
/*                              KoXmlElement extra = metadata.namedItem("dc-").toElement();
                                if (!date.isNull()) {
                                    debugText << "creator: " << creator.text();
                                    changeElement->setCreator(creator.text());
                                }*/
                            }
                            changeElement->setEnabled(d->recordChanges);
                            d->changes.insert( d->changeId, changeElement);
                            d->loadedChanges.insert(tag.attributeNS(KoXmlNS::text,"id"), d->changeId++);
                        }
                    }
                }
            }
        }
    } else {
        //This is the ODF 1.2 Change Format
        KoXmlElement tag;
        forEachElement(tag, element) {
            if (! tag.isNull()) {
                const QString localName = tag.localName();
                if (localName == "change-transaction") {
                    KoChangeTrackerElement *changeElement = 0;
                    //Set the change element as an insertion element for now
                    //Will be changed to the correct type when actual changes referencing this change-id are encountered
                    changeElement = new KoChangeTrackerElement(kundo2_noi18n(tag.attributeNS(KoXmlNS::delta,"change-id")),KoGenChange::InsertChange);
                    KoXmlElement metadata = tag.namedItemNS(KoXmlNS::delta,"change-info").toElement();
                    if (!metadata.isNull()) {
                           KoXmlElement date = metadata.namedItem("dc:date").toElement();
                           if (!date.isNull()) {
                                changeElement->setDate(date.text());
                            }
                            KoXmlElement creator = metadata.namedItem("dc:creator").toElement();
                            if (!creator.isNull()) {
                                changeElement->setCreator(creator.text());
                            }
                    }
                    changeElement->setEnabled(d->recordChanges);
                    d->changes.insert( d->changeId, changeElement);
                    d->loadedChanges.insert(tag.attributeNS(KoXmlNS::delta,"change-id"), d->changeId++);
               }
           }
        }
    }
}

int KoChangeTracker::getLoadedChangeId(const QString &odfId) const
{
    return d->loadedChanges.value(odfId);
}

int KoChangeTracker::getDeletedChanges(QVector<KoChangeTrackerElement *>& deleteVector) const
{
    int numAppendedItems = 0;
    foreach (KoChangeTrackerElement *element, d->changes.values()) {
        if(element->getChangeType() == KoGenChange::DeleteChange && !element->acceptedRejected()) {
          deleteVector << element;
          numAppendedItems++;
        }
    }

    return numAppendedItems;
}

QColor KoChangeTracker::getInsertionBgColor() const
{
    return d->insertionBgColor;
}

QColor KoChangeTracker::getDeletionBgColor() const
{
    return d->deletionBgColor;
}

QColor KoChangeTracker::getFormatChangeBgColor() const
{
    return d->formatChangeBgColor;
}

void KoChangeTracker::setInsertionBgColor(const QColor& bgColor)
{
    d->insertionBgColor = bgColor;
}

void KoChangeTracker::setDeletionBgColor(const QColor& bgColor)
{
    d->deletionBgColor = bgColor;
}

void KoChangeTracker::setFormatChangeBgColor(const QColor& bgColor)
{
    d->formatChangeBgColor = bgColor;
}

////A convenience function to get a ListIdType from a format
//static KoListStyle::ListIdType ListId(const QTextListFormat &format)
//{
//    KoListStyle::ListIdType listId;

//    if (sizeof(KoListStyle::ListIdType) == sizeof(uint)) {
//        listId = format.property(KoListStyle::ListId).toUInt();
//    }
//    else {
//        listId = format.property(KoListStyle::ListId).toULongLong();
//    }

//    return listId;
//}

QTextDocumentFragment KoChangeTracker::generateDeleteFragment(const QTextCursor &cursor)
{
    QTextCursor editCursor(cursor);
    QTextDocument *document = cursor.document();

    QTextDocument deletedDocument;
    QTextDocument deleteCursor(&deletedDocument);

    KoInlineTextObjectManager *textObjectManager = KoTextDocument(document).inlineTextObjectManager();
    if (textObjectManager) {
        for (int i = cursor.anchor();i <= cursor.position(); i++) {
            if (document->characterAt(i) == QChar::ObjectReplacementCharacter) {
                editCursor.setPosition(i+1);
	    }
        }
    }

    QTextBlock currentBlock = document->findBlock(cursor.anchor());
    QTextBlock startBlock = currentBlock;
    QTextBlock endBlock = document->findBlock(cursor.position()).next();

    currentBlock = document->findBlock(cursor.anchor());
    startBlock = currentBlock;
    endBlock = document->findBlock(cursor.position()).next();

    for (;currentBlock != endBlock; currentBlock = currentBlock.next()) {
        editCursor.setPosition(currentBlock.position());
        if (editCursor.currentTable()) {
            QTextTableFormat tableFormat = editCursor.currentTable()->format();
            editCursor.currentTable()->setFormat(tableFormat);
        }

        if (currentBlock != startBlock) {
            QTextBlockFormat blockFormat;
            editCursor.mergeBlockFormat(blockFormat);
        }
    }

    return cursor.selection();
}

bool KoChangeTracker::checkListDeletion(const QTextList &list, const QTextCursor &cursor)
{
    int startOfList = (list.item(0).position() - 1);
    int endOfList = list.item(list.count() -1).position() + list.item(list.count() -1).length() - 1;
    if ((cursor.anchor() <= startOfList) && (cursor.position() >= endOfList))
        return true;
    else {
        /***************************************************************************************************/
        /*                                    Qt Quirk Work-Around                                         */
        /***************************************************************************************************/
        if ((cursor.anchor() == (startOfList + 1)) && (cursor.position() > endOfList)) {
            return true;
        /***************************************************************************************************/
        } else if((cursor.anchor() <= startOfList) && (list.count() == 1)) {
            return true;
        } else {
            return false;
        }
    }
}

void KoChangeTracker::insertDeleteFragment(QTextCursor &cursor)
{
    QTextDocument tempDoc;
    QTextCursor tempCursor(&tempDoc);

    bool deletedListItem = false;

    for (QTextBlock currentBlock = tempDoc.begin(); currentBlock != tempDoc.end(); currentBlock = currentBlock.next()) {
        //This condition is for the work-around for a Qt behaviour
        //Even if a delete ends at the end of a table, the fragment will have an empty block after the table
        //If such a block is detected then, just ignore it
        if ((currentBlock.next() == tempDoc.end()) && (currentBlock.text().length() == 0) && (QTextCursor(currentBlock.previous()).currentTable())) {
            continue;
        }

        tempCursor.setPosition(currentBlock.position());
        QTextList *textList = tempCursor.currentList();
        int outlineLevel = currentBlock.blockFormat().property(KoParagraphStyle::OutlineLevel).toInt();

        KoList *currentList = KoTextDocument(cursor.document()).list(cursor.block());
        int docOutlineLevel = cursor.block().blockFormat().property(KoParagraphStyle::OutlineLevel).toInt();
        if (docOutlineLevel) {
            //Even though we got a list, it is actually a list for storing headings. So don't consider it
            currentList = NULL;
        }

        QTextList *previousTextList = currentBlock.previous().isValid() ? QTextCursor(currentBlock.previous()).currentList():NULL;
        if (textList && previousTextList && (textList != previousTextList) && (KoList::level(currentBlock) == KoList::level(currentBlock.previous()))) {
            //Even though we are already in a list, the QTextList* of the current block is differnt from that of the previous block
            //Also the levels of the list-items ( previous and current ) are the same.
            //This can happen only when two lists are merged together without any intermediate content.
            //So we need to create a new list.
            currentList = NULL;
        }

        if (textList) {
            if (deletedListItem && currentBlock != tempDoc.begin()) {
                // Found a deleted list item in the fragment. So insert a new list-item
                int deletedListItemLevel = KoList::level(currentBlock);

                if (!(QTextCursor(currentBlock.previous()).currentTable())) {
                    cursor.insertBlock(currentBlock.blockFormat(), currentBlock.charFormat());
                } else {
                    cursor.mergeBlockFormat(currentBlock.blockFormat());
                }

                if(!currentList) {
                    if (!outlineLevel) {
                        //This happens when a part of a paragraph and a succeeding list-item are deleted together
                        //So go to the next block and insert it in the list there.
                        QTextCursor tmp(cursor);
                        tmp.setPosition(tmp.block().next().position());
                        currentList = KoTextDocument(tmp.document()).list(tmp.block());
                    } else {
                        // This is a heading. So find the KoList for heading and add the block there
                        KoList *headingList = KoTextDocument(cursor.document()).headingList();
                        currentList = headingList;
                    }
                }
                currentList->add(cursor.block(), deletedListItemLevel);
            }
        } else if (tempCursor.currentTable()) {
            QTextTable *deletedTable = tempCursor.currentTable();
            int numRows = deletedTable->rows();
            int numColumns = deletedTable->columns();
            QTextTable *insertedTable = cursor.insertTable(numRows, numColumns, deletedTable->format());
            for (int i=0; i<numRows; i++) {
                for (int j=0; j<numColumns; j++) {
                    tempCursor.setPosition(deletedTable->cellAt(i,j).firstCursorPosition().position());
                    tempCursor.setPosition(deletedTable->cellAt(i,j).lastCursorPosition().position(), QTextCursor::KeepAnchor);
                    insertedTable->cellAt(i,j).setFormat(deletedTable->cellAt(i,j).format().toTableCellFormat());
                    cursor.setPosition(insertedTable->cellAt(i,j).firstCursorPosition().position());
                    cursor.insertFragment(tempCursor.selection());
                }
            }
            tempCursor.setPosition(deletedTable->cellAt(numRows-1,numColumns-1).lastCursorPosition().position());
            currentBlock = tempCursor.block();
            //Move the cursor outside of table
            cursor.setPosition(cursor.position() + 1);
            continue;
        } else {
            // This block does not contain a list. So no special work here.
            if ((currentBlock != tempDoc.begin()) && !(QTextCursor(currentBlock.previous()).currentTable())) {
                cursor.insertBlock(currentBlock.blockFormat(), currentBlock.charFormat());
            }

            if (QTextCursor(currentBlock.previous()).currentTable()) {
                cursor.mergeBlockFormat(currentBlock.blockFormat());
            }
        }

        /********************************************************************************************************************/
        /*This section of code is a work-around for a bug in the Qt. This work-around is safe. If and when the bug is fixed */
        /*the if condition would never be true and the code would never get executed                                        */
        /********************************************************************************************************************/
        if ((KoList::level(cursor.block()) != KoList::level(currentBlock)) && currentBlock.text().length()) {
            if (!currentList) {
                QTextCursor tmp(cursor);
                tmp.setPosition(tmp.block().previous().position());
                currentList = KoTextDocument(tmp.document()).list(tmp.block());
            }
            currentList->add(cursor.block(), KoList::level(currentBlock));
        }
        /********************************************************************************************************************/

        // Finally insert all the contents of the block into the main document.
        QTextBlock::iterator it;
        for (it = currentBlock.begin(); !(it.atEnd()); ++it) {
            QTextFragment currentFragment = it.fragment();
            if (currentFragment.isValid()) {
                cursor.insertText(currentFragment.text(), currentFragment.charFormat());
            }
        }
    }
}

int KoChangeTracker::fragmentLength(const QTextDocumentFragment &fragment)
{
    QTextDocument tempDoc;
    QTextCursor tempCursor(&tempDoc);
    tempCursor.insertFragment(fragment);
    int length = 0;
    bool deletedListItem = false;
    for (QTextBlock currentBlock = tempDoc.begin(); currentBlock != tempDoc.end(); currentBlock = currentBlock.next()) {
        tempCursor.setPosition(currentBlock.position());
        if (tempCursor.currentList()) {
            if (currentBlock != tempDoc.begin() && deletedListItem)
                length += 1; //For the Block separator
        } else if (tempCursor.currentTable()) {
            QTextTable *deletedTable = tempCursor.currentTable();
            int numRows = deletedTable->rows();
            int numColumns = deletedTable->columns();
            for (int i=0; i<numRows; i++) {
                for (int j=0; j<numColumns; j++) {
                    length += 1;
                    length += (deletedTable->cellAt(i,j).lastCursorPosition().position() - deletedTable->cellAt(i,j).firstCursorPosition().position());
                }
            }
            tempCursor.setPosition(deletedTable->cellAt(numRows-1,numColumns-1).lastCursorPosition().position());
            currentBlock = tempCursor.block();
            length += 1;
            continue;
        } else {
            if ((currentBlock != tempDoc.begin()) && !(QTextCursor(currentBlock.previous()).currentTable()))
                length += 1; //For the Block Separator
        }


        QTextBlock::iterator it;
        for (it = currentBlock.begin(); !(it.atEnd()); ++it) {
            QTextFragment currentFragment = it.fragment();
            if (currentFragment.isValid())
                length += currentFragment.text().length();
        }
    }

    return length;
}
