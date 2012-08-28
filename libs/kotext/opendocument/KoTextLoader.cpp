/* This file is part of the KDE project
 * Copyright (C) 2001-2006 David Faure <faure@kde.org>
 * Copyright (C) 2007,2009,2011 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007 Sebastian Sauer <mail@dipe.org>
 * Copyright (C) 2007,2011 Pierre Ducroquet <pinaraf@pinaraf.info>
 * Copyright (C) 2007-2011 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2008 Girish Ramakrishnan <girish@forwardbias.in>
 * Copyright (C) 2009 KO GmbH <cbo@kogmbh.com>
 * Copyright (C) 2009 Pierre Stirnweiss <pstirnweiss@googlemail.com>
 * Copyright (C) 2010 KO GmbH <ben.martin@kogmbh.com>
 * Copyright (C) 2011 Pavol Korinek <pavol.korinek@ixonos.com>
 * Copyright (C) 2011 Lukáš Tvrdý <lukas.tvrdy@ixonos.com>
 * Copyright (C) 2011 Boudewijn Rempt <boud@kogmbh.com>
 * Copyright (C) 2011-2012 Gopalakrishna Bhat A <gopalakbhat@gmail.com>
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
#include "KoTextLoader.h"

#include <KoTextMeta.h>
#include <KoBookmark.h>
#include <KoBookmarkManager.h>
#include <KoInlineNote.h>
#include <KoInlineCite.h>
#include <KoInlineTextObjectManager.h>
#include "KoList.h"
#include <KoOdfLoadingContext.h>
#include <KoOdfStylesReader.h>
#include <KoProperties.h>
#include <KoShapeContainer.h>
#include <KoShapeFactoryBase.h>
#include <KoShapeLoadingContext.h>
#include <KoShapeRegistry.h>
#include <KoTableColumnAndRowStyleManager.h>
#include <KoTextAnchor.h>
#include <KoTextBlockData.h>
#include "KoTextDebug.h"
#include "KoTextDocument.h"
#include "KoTextSharedLoadingData.h"
#include <KoUnit.h>
#include <KoVariable.h>
#include <KoVariableManager.h>
#include <KoInlineObjectRegistry.h>
#include <KoXmlNS.h>
#include <KoXmlReader.h>
#include "KoTextInlineRdf.h"
#include "KoTableOfContentsGeneratorInfo.h"
#include "KoBibliographyInfo.h"
#include "KoSection.h"
#include "KoTextSoftPageBreak.h"
#include "KoDocumentRdfBase.h"
#include "KoElementReference.h"
#include "KoTextTableTemplate.h"

#include "changetracker/KoChangeTracker.h"
#include "changetracker/KoChangeTrackerElement.h"
#include "changetracker/KoDeleteChangeMarker.h"
#include "changetracker/KoFormatChangeInformation.h"
#include "styles/KoStyleManager.h"
#include "styles/KoParagraphStyle.h"
#include "styles/KoCharacterStyle.h"
#include "styles/KoListStyle.h"
#include "styles/KoListLevelProperties.h"
#include "styles/KoTableStyle.h"
#include "styles/KoTableColumnStyle.h"
#include "styles/KoTableCellStyle.h"
#include "styles/KoSectionStyle.h"

#include <klocale.h>
#include <kdebug.h>

#include <QList>
#include <QVector>
#include <QMap>
#include <QRect>
#include <QStack>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextList>
#include <QTextTable>
#include <QTime>
#include <QString>
#include <QTextInlineObject>
#include <QTextStream>
#include <QXmlStreamReader>

// if defined then debugging is enabled
// #define KOOPENDOCUMENTLOADER_DEBUG

/// \internal d-pointer class.
class KoTextLoader::Private
{
public:
    KoShapeLoadingContext &context;
    KoTextSharedLoadingData *textSharedData;
    // store it here so that you don't need to get it all the time from
    // the KoOdfLoadingContext.
    bool stylesDotXml;

    QTextBlockFormat defaultBlockFormat;
    QTextCharFormat defaultCharFormat;
    int bodyProgressTotal;
    int bodyProgressValue;
    int nextProgressReportMs;
    QTime progressTime;

    QVector<KoList *> currentLists;
    KoListStyle *currentListStyle;
    int currentListLevel;
    // Two lists that follow the same style are considered as one for numbering purposes
    // This hash keeps all the lists that have the same style in one KoList.
    QHash<KoListStyle *, KoList *> lists;

    KoCharacterStyle *endCharStyle; // charstyle from empty span used at end of paragraph

    KoStyleManager *styleManager;

    KoChangeTracker *changeTracker;

    KoShape *shape;

    int loadSpanLevel;
    int loadSpanInitialPos;
    QStack<int> changeStack;
    QMap<QString, int> changeTransTable;
    QMap<QString, KoXmlElement> deleteChangeTable;
    QMap<QString, QString> endIdMap;
    QMap<QString, int> splitPositionMap;

    // For handling complex-deletes i.e delete changes that merges elements of different types
    int openedElements;
    QMap<QString, QString> removeLeavingContentMap;
    QMap<QString, QString> removeLeavingContentChangeIdMap;
    QVector<QString> nameSpacesList;
    bool deleteMergeStarted;
    QList<QVariant> openingSections;
    void copyRemoveLeavingContentStart(const KoXmlNode &node, QTextStream &xmlStream);
    void copyRemoveLeavingContentEnd(const KoXmlNode &node, QTextStream &xmlStream);
    void copyInsertAroundContent(const KoXmlNode &node, QTextStream &xmlStream);
    void copyNode(const KoXmlNode &node, QTextStream &xmlStream, bool copyOnlyChildren = false);
    void copyTagStart(const KoXmlElement &element, QTextStream &xmlStream, bool ignoreChangeAttributes = false);
    void copyTagEnd(const KoXmlElement &element, QTextStream &xmlStream);

    //For handling delete changes
    KoDeleteChangeMarker *insertDeleteChangeMarker(QTextCursor &cursor, const QString &id);
    void processDeleteChange(QTextCursor &cursor);

    // For Merging consecutive delete changes into a single change
    bool checkForDeleteMerge(QTextCursor &cursor, const QString &id, int startPosition);
    QMap<KoDeleteChangeMarker *, QPair<int, int> > deleteChangeMarkerMap;

    QMap<QString, KoList *> xmlIdToListMap;
    QVector<KoList *> m_previousList;

    QMap<QString, KoList *> numberedParagraphListId;

    QStringList rdfIdList;

    /// level is between 1 and 10
    void setCurrentList(KoList *currentList, int level);
    /// level is between 1 and 10
    KoList *previousList(int level);

    // For Loading of list item splits
    bool checkForListItemSplit(const KoXmlElement &element);
    KoXmlNode loadListItemSplit(const KoXmlElement &element, QString *generatedXmlString);

    explicit Private(KoShapeLoadingContext &context, KoShape *s)
        : context(context),
          textSharedData(0),
          // stylesDotXml says from where the office:automatic-styles are to be picked from:
          // the content.xml or the styles.xml (in a multidocument scenario). It does not
          // decide from where the office:styles are to be picked (always picked from styles.xml).
          // For our use here, stylesDotXml is always false (see ODF1.1 spec §2.1).
          stylesDotXml(context.odfLoadingContext().useStylesAutoStyles()),
          bodyProgressTotal(0),
          bodyProgressValue(0),
          nextProgressReportMs(0),
          currentLists(10),
          currentListStyle(0),
          currentListLevel(1),
          endCharStyle(0),
          styleManager(0),
          changeTracker(0),
          shape(s),
          loadSpanLevel(0),
          loadSpanInitialPos(0),
          openedElements(0),
          deleteMergeStarted(false)
        , m_previousList(10)
    {
        progressTime.start();
    }

    ~Private() {
        kDebug(32500) << "Loading took" << (float)(progressTime.elapsed()) / 1000 << " seconds";
    }

    KoList *list(const QTextDocument *document, KoListStyle *listStyle, bool mergeSimilarStyledList);

    void openChangeRegion(const KoXmlElement &element);
    void closeChangeRegion(const KoXmlElement &element);
    void splitStack(int id);
};

class AttributeChangeRecord {
public:
    AttributeChangeRecord():isValid(false){}

    void setChangeRecord(const QString& changeRecord)
    {
        QStringList strList = changeRecord.split(',');
        this->changeId = strList.value(0);
        this->changeType = strList.value(1);
        this->attributeName = strList.value(2);
        this->attributeValue = strList.value(3);
        this->isValid = true;
    }

    bool isValid;
    QString changeId;
    QString changeType;
    QString attributeName;
    QString attributeValue;
};

void KoTextLoader::Private::openChangeRegion(const KoXmlElement& element)
{
    if (!changeTracker)
        return;

    QString id;
    AttributeChangeRecord attributeChange;

    if (element.localName() == "change-start") {
        //This is a ODF 1.1 Change
        id = element.attributeNS(KoXmlNS::text, "change-id");
    } else if (element.localName() == "inserted-text-start") {
        //This is a ODF 1.2 Change
        id = element.attributeNS(KoXmlNS::delta, "insertion-change-idref");
        QString textEndId = element.attributeNS(KoXmlNS::delta, "inserted-text-end-idref");
        endIdMap.insert(textEndId, id);
    } else if ((element.localName() == "removed-content") || (element.localName() == "merge")) {
        id = element.attributeNS(KoXmlNS::delta, "removal-change-idref");
    } else if (element.localName() == "remove-leaving-content-start") {
        id = element.attributeNS(KoXmlNS::delta, "removal-change-idref");
        QString endId = element.attributeNS(KoXmlNS::delta, "end-element-idref");
        endIdMap.insert(endId, id);
    } else if (!element.attributeNS(KoXmlNS::delta, "insertion-type").isEmpty()) {
        QString insertionType = element.attributeNS(KoXmlNS::delta, "insertion-type");
        if ((insertionType == "insert-with-content") || (insertionType == "insert-around-content")) {
            id = element.attributeNS(KoXmlNS::delta, "insertion-change-idref");
        }
    } else if (!element.attributeNS(KoXmlNS::ac, "change001").isEmpty()) {
        attributeChange.setChangeRecord(element.attributeNS(KoXmlNS::ac, "change001"));
        id = attributeChange.changeId;
    } else {
    }

    int changeId = changeTracker->getLoadedChangeId(id);
    if (!changeId)
        return;

    if (!changeStack.empty() && (changeStack.top() != changeId)) {
        //Parent child relationship is defined by the order in which the change meta-data is seen
        //So check the changeId to set the parent-child relationship
        if (changeId > changeStack.top()) {
            changeTracker->setParent(changeId, changeStack.top());
            changeStack.push(changeId);
        } else {
            int duplicateId = changeTracker->createDuplicateChangeId(changeStack.top());
            changeTracker->setParent(duplicateId, changeId);
            changeStack.push(duplicateId);
        }
    } else {
        changeStack.push(changeId);
    }

    changeTransTable.insert(id, changeId);

    KoChangeTrackerElement *changeElement = changeTracker->elementById(changeId);
    changeElement->setEnabled(true);

    if ((element.localName() == "remove-leaving-content-start")) {
        changeElement->setChangeType(KoGenChange::FormatChange);

        KoXmlElement spanElement = element.firstChild().toElement();
        QString styleName = spanElement.attributeNS(KoXmlNS::text, "style-name", QString());

        QTextCharFormat cf;
        KoCharacterStyle *characterStyle = textSharedData->characterStyle(styleName, stylesDotXml);
        if (characterStyle) {
            characterStyle->applyStyle(cf);
        }

        KoTextStyleChangeInformation *formatChangeInformation = new KoTextStyleChangeInformation();
        formatChangeInformation->setPreviousCharFormat(cf);
        changeTracker->setFormatChangeInformation(changeId, formatChangeInformation);
    } else if((element.localName() == "p") && attributeChange.isValid) {
        changeElement->setChangeType(KoGenChange::FormatChange);
        QTextBlockFormat blockFormat;
        if (attributeChange.attributeName == "text:style-name") {
            QString styleName = attributeChange.attributeValue;
            KoParagraphStyle *paragraphStyle = textSharedData->paragraphStyle(styleName, stylesDotXml);
            if (paragraphStyle) {
                paragraphStyle->applyStyle(blockFormat);
            }
        }

        KoParagraphStyleChangeInformation *paragraphChangeInformation = new KoParagraphStyleChangeInformation();
        paragraphChangeInformation->setPreviousBlockFormat(blockFormat);
        changeTracker->setFormatChangeInformation(changeId, paragraphChangeInformation);
    } else if((element.localName() == "list-item") && attributeChange.isValid) {
        changeElement->setChangeType(KoGenChange::FormatChange);
        if (attributeChange.changeType == "insert") {
            KoListItemNumChangeInformation *listItemChangeInformation = new KoListItemNumChangeInformation(KoListItemNumChangeInformation::eNumberingRestarted);
            changeTracker->setFormatChangeInformation(changeId, listItemChangeInformation);
        } else if (attributeChange.changeType == "remove") {
            KoListItemNumChangeInformation *listItemChangeInformation = new KoListItemNumChangeInformation(KoListItemNumChangeInformation::eRestartRemoved);
            listItemChangeInformation->setPreviousStartNumber(attributeChange.attributeValue.toInt());
            changeTracker->setFormatChangeInformation(changeId, listItemChangeInformation);
        }
    } else if((element.attributeNS(KoXmlNS::delta, "insertion-type") == "insert-around-content")) {
        changeElement->setChangeType(KoGenChange::FormatChange);
    } else if ((element.localName() == "removed-content") || (element.localName() == "merge")) {
        changeElement->setChangeType(KoGenChange::DeleteChange);
    }
}

void KoTextLoader::Private::closeChangeRegion(const KoXmlElement& element)
{
    if (!changeTracker)
        return;

    QString id;
    int changeId;
    if (element.localName() == "change-end") {
        //This is a ODF 1.1 Change
        id = element.attributeNS(KoXmlNS::text, "change-id");
    } else if (element.localName() == "inserted-text-end"){
        // This is a ODF 1.2 Change
        QString textEndId = element.attributeNS(KoXmlNS::delta, "inserted-text-end-id");
        id = endIdMap.value(textEndId);
        endIdMap.remove(textEndId);
    } else if ((element.localName() == "removed-content") || (element.localName() == "merge")) {
        id = element.attributeNS(KoXmlNS::delta, "removal-change-idref");
    } else if (element.localName() == "remove-leaving-content-end"){
        QString endId = element.attributeNS(KoXmlNS::delta, "end-element-id");
        id = endIdMap.value(endId);
        endIdMap.remove(endId);
    } else if (!element.attributeNS(KoXmlNS::delta, "insertion-type").isEmpty()){
        QString insertionType = element.attributeNS(KoXmlNS::delta, "insertion-type");
        if ((insertionType == "insert-with-content") || (insertionType == "insert-around-content")) {
            id = element.attributeNS(KoXmlNS::delta, "insertion-change-idref");
        }
    } else if (!element.attributeNS(KoXmlNS::ac, "change001").isEmpty()) {
        AttributeChangeRecord attributeChange;
        attributeChange.setChangeRecord(element.attributeNS(KoXmlNS::ac, "change001"));
        id = attributeChange.changeId;
    } else {
    }

    changeId = changeTracker->getLoadedChangeId(id);
    splitStack(changeId);
}

void KoTextLoader::Private::splitStack(int id)
{
    if (changeStack.isEmpty())
        return;
    int oldId = changeStack.top();
    changeStack.pop();
    if ((id == oldId) || changeTracker->isParent(id, oldId))
        return;
    int newId = changeTracker->split(oldId);
    splitStack(id);
    changeTracker->setParent(newId, changeStack.top());
    changeStack.push(newId);
}

KoList *KoTextLoader::Private::list(const QTextDocument *document, KoListStyle *listStyle, bool mergeSimilarStyledList)
{
    //TODO: Remove mergeSimilarStyledList parameter by finding a way to put the numbered-paragraphs of same level
    //      to a single QTextList while loading rather than maintaining a hash list
    if (mergeSimilarStyledList) {
        if (lists.contains(listStyle)) {
            return lists[listStyle];
        }
    }
    KoList *newList = new KoList(document, listStyle);
    lists[listStyle] = newList;
    return newList;
}

void KoTextLoader::Private::setCurrentList(KoList *currentList, int level)
{
    Q_ASSERT(level > 0 && level <= 10);
    currentLists[level - 1] = currentList;
    m_previousList[level - 1] = currentList;
}

KoList *KoTextLoader::Private::previousList(int level)
{
    Q_ASSERT(level > 0 && level <= 10);

    if (m_previousList.size() < level) {
        return 0;
    }

    return m_previousList.at(level - 1);
}

inline static bool isspace(ushort ch)
{
    // options are ordered by likelyhood
    return ch == ' ' || ch== '\n' || ch == '\r' ||  ch == '\t';
}

QString KoTextLoader::normalizeWhitespace(const QString &in, bool leadingSpace)
{
    QString textstring = in;
    ushort *text = (ushort*)textstring.data(); // this detaches from the string 'in'
    int r, w = 0;
    int len = textstring.length();
    for (r = 0; r < len; ++r) {
        const ushort ch = text[r];
        // check for space, tab, line feed, carriage return
        if (isspace(ch)) {
            // if we were lead by whitespace in some parent or previous sibling element,
            // we completely collapse this space
            if (r != 0 || !leadingSpace)
                text[w++] = ' ';
            // find the end of the whitespace run
            while (r < len && isspace(text[r]))
                ++r;
            // and then record the next non-whitespace character
            if (r < len)
                text[w++] = text[r];
        } else {
            text[w++] = ch;
        }
    }
    // and now trim off the unused part of the string
    textstring.truncate(w);
    return textstring;
}

/////////////KoTextLoader

KoTextLoader::KoTextLoader(KoShapeLoadingContext &context, KoShape *shape)
    : QObject()
    , d(new Private(context, shape))
{
    KoSharedLoadingData *sharedData = context.sharedData(KOTEXT_SHARED_LOADING_ID);
    if (sharedData) {
        d->textSharedData = dynamic_cast<KoTextSharedLoadingData *>(sharedData);
    }

    //kDebug(32500) << "sharedData" << sharedData << "textSharedData" << d->textSharedData;

    if (!d->textSharedData) {
        d->textSharedData = new KoTextSharedLoadingData();
        KoDocumentResourceManager *rm = context.documentResourceManager();
        KoStyleManager *styleManager = rm->resource(KoText::StyleManager).value<KoStyleManager*>();
        d->textSharedData->loadOdfStyles(context, styleManager);
        if (!sharedData) {
            context.addSharedData(KOTEXT_SHARED_LOADING_ID, d->textSharedData);
        } else {
            kWarning(32500) << "A different type of sharedData was found under the" << KOTEXT_SHARED_LOADING_ID;
            Q_ASSERT(false);
        }
    }

    if (context.documentRdf()) {
        d->rdfIdList = qobject_cast<KoDocumentRdfBase*>(context.documentRdf())->idrefList();
    }
}

KoTextLoader::~KoTextLoader()
{
    delete d;
}

void KoTextLoader::loadBody(const KoXmlElement &bodyElem, QTextCursor &cursor)
{
    const QTextDocument *document = cursor.block().document();

    static int rootCallChecker = 0;
    if (rootCallChecker == 0) {
        if (document->resource(KoTextDocument::FrameCharFormat, KoTextDocument::FrameCharFormatUrl).isValid()) {
            d->defaultBlockFormat = KoTextDocument(document).frameBlockFormat();
            d->defaultCharFormat = KoTextDocument(document).frameCharFormat();
        } else {
            // This is the first call of loadBody on the document.
            // Store the default block and char formats
            // Will be used whenever a new block is inserted
            d->defaultCharFormat = cursor.charFormat();
            KoTextDocument(document).setFrameCharFormat(cursor.blockCharFormat());
            d->defaultBlockFormat = cursor.blockFormat();
            KoTextDocument(document).setFrameBlockFormat(cursor.blockFormat());
        }
    }
    rootCallChecker++;

    cursor.beginEditBlock();

    if (! d->openingSections.isEmpty()) {
        QTextBlock block = cursor.block();
        QTextBlockFormat format = block.blockFormat();
        QVariant v;
        v = format.property(KoParagraphStyle::SectionStartings);
        d->openingSections.append(v.value<QList<QVariant> >()); // if we had some already we need to append the new ones
        v.setValue<QList<QVariant> >(d->openingSections);
        format.setProperty(KoParagraphStyle::SectionStartings, v);
        cursor.setBlockFormat(format);
        d->openingSections.clear();
    }

    KoOdfNotesConfiguration *notesConfiguration =
            new KoOdfNotesConfiguration(d->context.odfLoadingContext()
                                        .stylesReader()
                                        .globalNotesConfiguration(KoOdfNotesConfiguration::Endnote));
    //KoTextDocument(document).setNotesConfiguration(notesConfiguration);
    KoTextDocument(document).styleManager()->setNotesConfiguration(notesConfiguration);

    notesConfiguration =
            new KoOdfNotesConfiguration(d->context.odfLoadingContext()
                                        .stylesReader()
                                        .globalNotesConfiguration(KoOdfNotesConfiguration::Footnote));
    //KoTextDocument(document).setNotesConfiguration(notesConfiguration);
    KoTextDocument(document).styleManager()->setNotesConfiguration(notesConfiguration);

    KoOdfLineNumberingConfiguration *lineNumberingConfiguration =
            new KoOdfLineNumberingConfiguration(d->context.odfLoadingContext()
                                                .stylesReader()
                                                .lineNumberingConfiguration());
    KoTextDocument(document).setLineNumberingConfiguration(lineNumberingConfiguration);

    KoOdfBibliographyConfiguration *bibConfiguration =
            new KoOdfBibliographyConfiguration(d->context.odfLoadingContext()
                                               .stylesReader()
                                               .globalBibliographyConfiguration());
    KoTextDocument(document).styleManager()->setBibliographyConfiguration(bibConfiguration);

    d->styleManager = KoTextDocument(document).styleManager();
    d->changeTracker = KoTextDocument(document).changeTracker();
    //    if (!d->changeTracker)
    //        d->changeTracker = dynamic_cast<KoChangeTracker *>(d->context.dataCenterMap().value("ChangeTracker"));
    //    Q_ASSERT(d->changeTracker);

#ifdef KOOPENDOCUMENTLOADER_DEBUG
    kDebug(32500) << "text-style:" << KoTextDebug::textAttributes(cursor.blockCharFormat());
#endif
    bool usedParagraph = false; // set to true if we found a tag that used the paragraph, indicating that the next round needs to start a new one.

    if (bodyElem.namespaceURI() == KoXmlNS::table && bodyElem.localName() == "table") {
        if (!bodyElem.attributeNS(KoXmlNS::delta, "insertion-type").isEmpty()) {
            d->openChangeRegion(bodyElem);
        }

        loadTable(bodyElem, cursor);

        if (!bodyElem.attributeNS(KoXmlNS::delta, "insertion-type").isEmpty()) {
            d->closeChangeRegion(bodyElem);
        }
    }
    else {
        startBody(KoXml::childNodesCount(bodyElem));

        KoXmlElement tag;
        for (KoXmlNode _node = bodyElem.firstChild(); !_node.isNull(); _node = _node.nextSibling() ) {
            if (!(tag = _node.toElement()).isNull()) {
                const QString localName = tag.localName();
                if (tag.namespaceURI() == KoXmlNS::delta) {

                    if (d->changeTracker && localName == "tracked-changes") {
                        d->changeTracker->loadOdfChanges(tag);
                    }
                    else if (d->changeTracker && localName == "removed-content") {
                        QString changeId = tag.attributeNS(KoXmlNS::delta, "removal-change-idref");
                        int deleteStartPosition = cursor.position();
                        if ((usedParagraph) && (tag.firstChild().toElement().localName() != "table"))
                            cursor.insertBlock(d->defaultBlockFormat, d->defaultCharFormat);

                        d->openChangeRegion(tag);
                        loadBody(tag, cursor);
                        d->closeChangeRegion(tag);

                        if (!d->checkForDeleteMerge(cursor, changeId, deleteStartPosition)) {
                            QTextCursor tempCursor(cursor);
                            tempCursor.setPosition(deleteStartPosition);
                            KoDeleteChangeMarker *marker = d->insertDeleteChangeMarker(tempCursor, changeId);
                            d->deleteChangeMarkerMap.insert(marker, QPair<int,int>(deleteStartPosition+1, cursor.position()));
                        }

                        if (tag.lastChild().toElement().localName() == "table") {
                            usedParagraph = false;
                        }

                    } else if (d->changeTracker && localName == "remove-leaving-content-start"){
                        if (usedParagraph)
                            cursor.insertBlock(d->defaultBlockFormat, d->defaultCharFormat);
                        usedParagraph = true;
                        QString generatedXmlString;
                        _node = loadDeleteMerges(tag,&generatedXmlString);
                        //Parse and Load the generated xml
                        QString errorMsg;
                        int errorLine, errorColumn;
                        KoXmlDocument doc;

                        QXmlStreamReader reader(generatedXmlString);
                        reader.setNamespaceProcessing(true);

                        bool ok = doc.setContent(&reader, &errorMsg, &errorLine, &errorColumn);
                        if (ok) {
                            loadBody(doc.documentElement(), cursor);
                        }
                    } else {
                    }
                }

                if (tag.namespaceURI() == KoXmlNS::text) {
                    if ((usedParagraph) && (tag.localName() != "table"))
                        cursor.insertBlock(d->defaultBlockFormat, d->defaultCharFormat);
                    usedParagraph = true;
                    if (d->changeTracker && localName == "tracked-changes") {
                        d->changeTracker->loadOdfChanges(tag);
                        storeDeleteChanges(tag);
                        usedParagraph = false;
                    } else if (d->changeTracker && localName == "change-start") {
                        d->openChangeRegion(tag);
                        usedParagraph = false;
                    } else if (d->changeTracker && localName == "change-end") {
                        d->closeChangeRegion(tag);
                        usedParagraph = false;
                    } else if (d->changeTracker && localName == "change") {
                        QString id = tag.attributeNS(KoXmlNS::text, "change-id");
                        int changeId = d->changeTracker->getLoadedChangeId(id);
                        if (changeId) {
                            if (d->changeStack.count() && (d->changeStack.top() != changeId))
                                d->changeTracker->setParent(changeId, d->changeStack.top());
                            KoDeleteChangeMarker *deleteChangemarker = new KoDeleteChangeMarker(d->changeTracker);
                            deleteChangemarker->setChangeId(changeId);
                            KoChangeTrackerElement *changeElement = d->changeTracker->elementById(changeId);
                            changeElement->setDeleteChangeMarker(deleteChangemarker);
                            changeElement->setEnabled(true);
                            KoInlineTextObjectManager *textObjectManager = KoTextDocument(cursor.block().document()).inlineTextObjectManager();
                            if (textObjectManager) {
                                textObjectManager->insertInlineObject(cursor, deleteChangemarker);
                            }
                        }

                        loadDeleteChangeOutsidePorH(id, cursor);
                        usedParagraph = false;
                    } else if (localName == "p") {    // text paragraph
                        if (tag.attributeNS(KoXmlNS::delta, "insertion-type") != "insert-around-content") {
                            if (!tag.attributeNS(KoXmlNS::split, "split001-idref").isEmpty())
                                d->splitPositionMap.insert(tag.attributeNS(KoXmlNS::split, "split001-idref"),cursor.position());

                            if (!tag.attributeNS(KoXmlNS::delta, "insertion-type").isEmpty()) {
                                QString insertionType = tag.attributeNS(KoXmlNS::delta, "insertion-type");
                                if (insertionType == "insert-with-content") {
                                    d->openChangeRegion(tag);
                                }

                                if (insertionType == "split") {
                                    QString splitId = tag.attributeNS(KoXmlNS::delta, "split-id");
                                    QString changeId = tag.attributeNS(KoXmlNS::delta, "insertion-change-idref");
                                    markBlocksAsInserted(cursor, d->splitPositionMap.value(splitId), changeId);
                                    d->splitPositionMap.remove(splitId);
                                }
                            } else if (!tag.attributeNS(KoXmlNS::ac, "change001").isEmpty()) {
                                d->openChangeRegion(tag);
                            }

                            loadParagraph(tag, cursor);

                            if ((!tag.attributeNS(KoXmlNS::delta, "insertion-type").isEmpty()) ||
                                    (!tag.attributeNS(KoXmlNS::ac, "change001").isEmpty())) {
                                d->closeChangeRegion(tag);
                            }

                        } else {
                            QString generatedXmlString;
                            _node = loadDeleteMerges(tag,&generatedXmlString);
                            //Parse and Load the generated xml
                            QString errorMsg;
                            int errorLine, errorColumn;
                            KoXmlDocument doc;

                            QXmlStreamReader reader(generatedXmlString);
                            reader.setNamespaceProcessing(true);

                            bool ok = doc.setContent(&reader, &errorMsg, &errorLine, &errorColumn);
                            if (ok) {
                                loadBody(doc.documentElement(), cursor);
                            }
                        }
                    } else if (localName == "h") {  // heading
                        if (tag.attributeNS(KoXmlNS::delta, "insertion-type") != "insert-around-content") {
                            if (!tag.attributeNS(KoXmlNS::split, "split001-idref").isEmpty())
                                d->splitPositionMap.insert(tag.attributeNS(KoXmlNS::split, "split001-idref"),cursor.position());

                            if (!tag.attributeNS(KoXmlNS::delta, "insertion-type").isEmpty()) {
                                QString insertionType = tag.attributeNS(KoXmlNS::delta, "insertion-type");
                                if (insertionType == "insert-with-content")
                                    d->openChangeRegion(tag);
                                if (insertionType == "split") {
                                    QString splitId = tag.attributeNS(KoXmlNS::delta, "split-id");
                                    QString changeId = tag.attributeNS(KoXmlNS::delta, "insertion-change-idref");
                                    markBlocksAsInserted(cursor, d->splitPositionMap.value(splitId), changeId);
                                    d->splitPositionMap.remove(splitId);
                                }
                            }
                            loadHeading(tag, cursor);

                            if (!tag.attributeNS(KoXmlNS::delta, "insertion-type").isEmpty())
                                d->closeChangeRegion(tag);
                        } else {
                            QString generatedXmlString;
                            _node = loadDeleteMerges(tag,&generatedXmlString);
                            // Parse and Load the generated xml
                            QString errorMsg;
                            int errorLine, errorColumn;
                            KoXmlDocument doc;

                            QXmlStreamReader reader(generatedXmlString);
                            reader.setNamespaceProcessing(true);

                            bool ok = doc.setContent(&reader, &errorMsg, &errorLine, &errorColumn);
                            if (ok) {
                                loadBody(doc.documentElement(), cursor);
                            }
                        }
                    } else if (localName == "unordered-list" || localName == "ordered-list" // OOo-1.1
                               || localName == "list" || localName == "numbered-paragraph") {  // OASIS
                        if (tag.attributeNS(KoXmlNS::delta, "insertion-type") != "insert-around-content") {
                            if (!tag.attributeNS(KoXmlNS::delta, "insertion-type").isEmpty())
                                d->openChangeRegion(tag);
                            loadList(tag, cursor);
                            if (!tag.attributeNS(KoXmlNS::delta, "insertion-type").isEmpty())
                                d->closeChangeRegion(tag);
                        } else {
                            QString generatedXmlString;
                            _node = loadDeleteMerges(tag,&generatedXmlString);
                            //Parse and Load the generated xml
                            QString errorMsg;
                            int errorLine, errorColumn;
                            KoXmlDocument doc;

                            QXmlStreamReader reader(generatedXmlString);
                            reader.setNamespaceProcessing(true);

                            bool ok = doc.setContent(&reader, &errorMsg, &errorLine, &errorColumn);
                            if (ok) {
                                loadBody(doc.documentElement(), cursor);
                            }
                        }
                    } else if (localName == "section") {
                        loadSection(tag, cursor);
                    } else if (localName == "table-of-content") {
                        loadTableOfContents(tag, cursor);
                    } else if (localName == "bibliography") {
                        loadBibliography(tag, cursor);
                    } else {
                        KoInlineObject *obj = KoInlineObjectRegistry::instance()->createFromOdf(tag, d->context);
                        if (obj) {
                            KoInlineTextObjectManager *textObjectManager = KoTextDocument(cursor.block().document()).inlineTextObjectManager();
                            if (textObjectManager) {
                                KoVariableManager *varManager = textObjectManager->variableManager();
                                if (varManager) {
                                    textObjectManager->insertInlineObject(cursor, obj);
                                }
                            }
                        } else {
                            usedParagraph = false;
                            kWarning(32500) << "unhandled text:" << localName;
                        }
                    }
                } else if (tag.namespaceURI() == KoXmlNS::draw) {
                    loadShape(tag, cursor);
                } else if (tag.namespaceURI() == KoXmlNS::table) {
                    if (localName == "table") {
                        if (!tag.attributeNS(KoXmlNS::delta, "insertion-type").isEmpty())
                            d->openChangeRegion(tag);

                        loadTable(tag, cursor);
                        usedParagraph = false;

                        if (!tag.attributeNS(KoXmlNS::delta, "insertion-type").isEmpty())
                            d->closeChangeRegion(tag);
                    } else {
                        kWarning(32500) << "KoTextLoader::loadBody unhandled table::" << localName;
                    }
                }
                processBody();
            }
        }
        endBody();
    }

    rootCallChecker--;
    if (rootCallChecker == 0) {
        d->processDeleteChange(cursor);
    }
    cursor.endEditBlock();
}

KoXmlNode KoTextLoader::loadDeleteMerges(const KoXmlElement& elem, QString *generatedXmlString)
{
    KoXmlNode lastProcessedNode = elem;
    d->nameSpacesList.clear();

    QString generatedXml;
    QTextStream xmlStream(&generatedXml);
    do {
        KoXmlElement element;
        bool isElementNode = lastProcessedNode.isElement();
        if (isElementNode)
            element = lastProcessedNode.toElement();

        if (isElementNode && (element.localName() == "remove-leaving-content-start")) {
            d->copyRemoveLeavingContentStart(element, xmlStream);
        } else if (isElementNode && (element.localName() == "remove-leaving-content-end")) {
            d->copyRemoveLeavingContentEnd(element, xmlStream);
        } else if (isElementNode && (element.attributeNS(KoXmlNS::delta, "insertion-type") == "insert-around-content")) {
            d->copyInsertAroundContent(element, xmlStream);
        } else {
            d->copyNode(element, xmlStream);
        }

        lastProcessedNode = lastProcessedNode.nextSibling();
    } while(d->openedElements && !lastProcessedNode.isNull());

    QTextStream docStream(generatedXmlString);

    docStream << "<generated-xml";
    for (int i=0; i<d->nameSpacesList.size();i++) {
        docStream << " xmlns:ns" << i << "=";
        docStream << "\"" << d->nameSpacesList.at(i) << "\"";
    }
    docStream << ">";
    docStream << generatedXml;
    docStream << "</generated-xml>";

    return lastProcessedNode.previousSibling();
}

void KoTextLoader::Private::copyRemoveLeavingContentStart(const KoXmlNode &node, QTextStream &xmlStream)
{
    KoXmlElement element = node.firstChild().toElement();
    QString changeEndId = node.toElement().attributeNS(KoXmlNS::delta, "end-element-idref");
    int index = nameSpacesList.indexOf(element.namespaceURI());
    if (index == -1) {
        nameSpacesList.append(element.namespaceURI());
        index = nameSpacesList.size() - 1;
    }
    QString nodeName  = QString("ns%1") + ':' + element.localName();
    nodeName = nodeName.arg(index);

    removeLeavingContentMap.insert(changeEndId, nodeName);
    openedElements++;

    QString changeId = node.toElement().attributeNS(KoXmlNS::delta, "removal-change-idref");
    removeLeavingContentChangeIdMap.insert(changeEndId, changeId);

    xmlStream << "<" << nodeName;
    QList<QPair<QString, QString> > attributeNSNames = element.attributeFullNames();

    QPair<QString, QString> attributeName;
    foreach(attributeName, attributeNSNames) {
        QString nameSpace = attributeName.first;
        if (nameSpace != "http://www.w3.org/XML/1998/namespace") {
            int index = nameSpacesList.indexOf(nameSpace);
            if (index == -1) {
                nameSpacesList.append(nameSpace);
                index = nameSpacesList.size() - 1;
            }
            xmlStream << " " << "ns" << index << ":" << attributeName.second << "=";
        } else {
            xmlStream << " " << "xml:" << attributeName.second << "=";
        }
        xmlStream << "\"" << element.attributeNS(nameSpace, attributeName.second) << "\"";
    }

    xmlStream << ">";

    if (deleteMergeStarted && (nodeName.endsWith(":p") || nodeName.endsWith(":h"))) {
        KoXmlElement nextElement = node.nextSibling().toElement();
        if (nextElement.localName() != "removed-content") {
            int deltaIndex = nameSpacesList.indexOf(KoXmlNS::delta);
            xmlStream << "<" << "ns" << deltaIndex << ":removed-content ";
            QString changeId = removeLeavingContentChangeIdMap.value(changeEndId);
            xmlStream << "ns" << deltaIndex << ":removal-change-idref=" << "\"" << changeId << "\"" << ">";
            xmlStream << "</" << "ns" << deltaIndex << ":removed-content>";
        }
    }
}

void KoTextLoader::Private::copyRemoveLeavingContentEnd(const KoXmlNode &node, QTextStream &xmlStream)
{
    QString changeEndId = node.toElement().attributeNS(KoXmlNS::delta, "end-element-id");
    QString nodeName = removeLeavingContentMap.value(changeEndId);
    removeLeavingContentMap.remove(changeEndId);
    openedElements--;

    if (nodeName.endsWith(":p") || nodeName.endsWith(":h")) {
        if (!deleteMergeStarted) {
            //We are not already inside a simple delete merge
            //Check Whether the previous sibling is a removed-content.
            //If not, then this is the starting p or h of a simple merge.
            KoXmlElement previousElement = node.previousSibling().toElement();
            if (previousElement.localName() != "removed-content") {
                int deltaIndex = nameSpacesList.indexOf(KoXmlNS::delta);
                if (deltaIndex == -1) {
                    nameSpacesList.append(KoXmlNS::delta);
                    deltaIndex = nameSpacesList.size() - 1;
                }
                xmlStream << "<" << "ns" << deltaIndex << ":removed-content ";
                QString changeId = removeLeavingContentChangeIdMap.value(changeEndId);
                xmlStream << "ns" << deltaIndex << ":removal-change-idref=" << "\"" << changeId << "\"" << ">";
                xmlStream << "</" << "ns" << deltaIndex << ":removed-content>";
            }
            deleteMergeStarted = true;
        } else {
            deleteMergeStarted = false;
        }
    }

    removeLeavingContentChangeIdMap.remove(changeEndId);
    xmlStream << "</" << nodeName << ">";
}

void KoTextLoader::Private::copyInsertAroundContent(const KoXmlNode &node, QTextStream &xmlStream)
{
    copyNode(node, xmlStream, true);
}

void KoTextLoader::Private::copyNode(const KoXmlNode &node, QTextStream &xmlStream, bool copyOnlyChildren)
{
    if (node.isText()) {
        xmlStream << node.toText().data();
    } else if (node.isElement()) {
        KoXmlElement element = node.toElement();
        if (!copyOnlyChildren) {
            copyTagStart(element, xmlStream);
        }

        for ( KoXmlNode node = element.firstChild(); !node.isNull(); node = node.nextSibling() ) {
            KoXmlElement childElement;
            bool isElementNode = node.isElement();
            if (isElementNode)
                childElement = node.toElement();


            if (isElementNode && (childElement.localName() == "remove-leaving-content-start")) {
                copyRemoveLeavingContentStart(childElement, xmlStream);
            } else if (isElementNode && (childElement.localName() == "remove-leaving-content-end")) {
                copyRemoveLeavingContentEnd(childElement, xmlStream);
            } else if (isElementNode && (childElement.attributeNS(KoXmlNS::delta, "insertion-type") == "insert-around-content")) {
                copyInsertAroundContent(childElement, xmlStream);
            } else {
                copyNode(node, xmlStream);
            }
        }

        if (!copyOnlyChildren) {
            copyTagEnd(element, xmlStream);
        }
    } else {
    }
}

void KoTextLoader::Private::copyTagStart(const KoXmlElement &element, QTextStream &xmlStream, bool ignoreChangeAttributes)
{
    int index = nameSpacesList.indexOf(element.namespaceURI());
    if (index == -1) {
        nameSpacesList.append(element.namespaceURI());
        index = nameSpacesList.size() - 1;
    }
    QString nodeName  = QString("ns%1") + ':' + element.localName();
    nodeName = nodeName.arg(index);
    xmlStream << "<" << nodeName;
    QList<QPair<QString, QString> > attributeNSNames = element.attributeFullNames();

    QPair<QString, QString> attributeName;
    foreach(attributeName, attributeNSNames) {
        QString nameSpace = attributeName.first;
        if (nameSpace == KoXmlNS::delta && ignoreChangeAttributes) {
            continue;
        }
        if (nameSpace != "http://www.w3.org/XML/1998/namespace") {
            int index = nameSpacesList.indexOf(nameSpace);
            if (index == -1) {
                nameSpacesList.append(nameSpace);
                index = nameSpacesList.size() - 1;
            }
            xmlStream << " " << "ns" << index << ":" << attributeName.second << "=";
        } else {
            xmlStream << " " << "xml:" << attributeName.second << "=";
        }
        xmlStream << "\"" << element.attributeNS(nameSpace, attributeName.second) << "\"";
    }
    xmlStream << ">";
}

void KoTextLoader::Private::copyTagEnd(const KoXmlElement &element, QTextStream &xmlStream)
{
    int index = nameSpacesList.indexOf(element.namespaceURI());
    QString nodeName  = QString("ns%1") + ':' + element.localName();
    nodeName = nodeName.arg(index);
    xmlStream << "</" << nodeName << ">";
}

void KoTextLoader::loadDeleteChangeOutsidePorH(QString id, QTextCursor &cursor)
{
    int startPosition = cursor.position();
    int changeId = d->changeTracker->getLoadedChangeId(id);

    if (changeId) {
        KoChangeTrackerElement *changeElement = d->changeTracker->elementById(changeId);
        KoXmlElement element = d->deleteChangeTable.value(id);

        //Call loadBody with this element
        loadBody(element, cursor);

        int endPosition = cursor.position();

        //Set the char format to the changeId
        cursor.setPosition(startPosition);
        cursor.setPosition(endPosition, QTextCursor::KeepAnchor);
        QTextCharFormat format;
        format.setProperty(KoCharacterStyle::ChangeTrackerId, changeId);
        cursor.mergeCharFormat(format);

        //Get the QTextDocumentFragment from the selection and store it in the changeElement
        QTextDocumentFragment deletedFragment(cursor);
        changeElement->setDeleteData(deletedFragment);

        //Now Remove this from the document. Will be re-inserted whenever changes have to be seen
        cursor.removeSelectedText();
    }
}

void KoTextLoader::loadParagraph(const KoXmlElement &element, QTextCursor &cursor)
{
    // TODO use the default style name a default value?
    const QString styleName = element.attributeNS(KoXmlNS::text, "style-name",
                                                  QString());

    KoParagraphStyle *paragraphStyle = d->textSharedData->paragraphStyle(styleName, d->stylesDotXml);

    Q_ASSERT(d->styleManager);
    if (!paragraphStyle) {
        // Either the paragraph has no style or the style-name could not be found.
        // Fix up the paragraphStyle to be our default paragraph style in either case.
        if (!styleName.isEmpty())
            kWarning(32500) << "paragraph style " << styleName << "not found - using default style";
        paragraphStyle = d->styleManager->defaultParagraphStyle();

    }

    QTextCharFormat cf = cursor.charFormat(); // store the current cursor char format

    if (paragraphStyle && (cursor.position() == cursor.block().position())) {
        QTextBlock block = cursor.block();
        // Apply list style when loading a list but we don't have a list style
        paragraphStyle->applyStyle(block, d->currentLists[d->currentListLevel - 1] && !d->currentListStyle);
        // Clear the outline level property. If a default-outline-level was set, it should not
        // be applied when loading a document, only on user action.
        block.blockFormat().clearProperty(KoParagraphStyle::OutlineLevel);
    }

    // Some paragraph have id's defined which we need to store so that we can eg
    // attach text animations to this specific paragraph later on
    KoElementReference id;
    id.loadOdf(element);

    if (id.isValid() && d->shape) {
        QTextBlock block = cursor.block();
        KoTextBlockData *data = dynamic_cast<KoTextBlockData*>(block.userData());
        if (!data) {
            data = new KoTextBlockData();
            block.setUserData(data);
        }
        d->context.addShapeSubItemId(d->shape, QVariant::fromValue(data), id.toString());
    }

    // attach Rdf to cursor.block()
    // remember inline Rdf metadata -- if the xml-id is actually
    // about rdf.
    if (element.hasAttributeNS(KoXmlNS::xhtml, "property")
            || d->rdfIdList.contains(id.toString()))
    {
        QTextBlock block = cursor.block();
        KoTextInlineRdf* inlineRdf =
                new KoTextInlineRdf((QTextDocument*)block.document(), block);
        if (inlineRdf->loadOdf(element)) {
                KoTextInlineRdf::attach(inlineRdf, cursor);
        }
        else {
            delete inlineRdf;
            inlineRdf = 0;
        }
    }

#ifdef KOOPENDOCUMENTLOADER_DEBUG
    kDebug(32500) << "text-style:" << KoTextDebug::textAttributes(cursor.blockCharFormat()) << d->currentLists[d->currentListLevel - 1] << d->currentListStyle;
#endif

    bool stripLeadingSpace = true;
    loadSpan(element, cursor, &stripLeadingSpace);
    QTextBlock block = cursor.block();
    QString text = block.text();
    if (text.length() == 0 || text.at(text.length()-1) == QChar(0x2028)) {
        if (d->endCharStyle) {
            QTextBlockFormat blockFormat = block.blockFormat();
            blockFormat.setProperty(KoParagraphStyle::EndCharStyle, QVariant::fromValue< QSharedPointer<KoCharacterStyle> >(QSharedPointer<KoCharacterStyle>(d->endCharStyle->clone())));
            cursor.setBlockFormat(blockFormat);
        }
    }
    d->endCharStyle = 0;

    cursor.setCharFormat(cf);   // restore the cursor char format
}

void KoTextLoader::loadHeading(const KoXmlElement &element, QTextCursor &cursor)
{
    Q_ASSERT(d->styleManager);
    int level = qMax(-1, element.attributeNS(KoXmlNS::text, "outline-level", "-1").toInt());
    // This will fallback to the default-outline-level applied by KoParagraphStyle

    QString styleName = element.attributeNS(KoXmlNS::text, "style-name", QString());

    QTextBlock block = cursor.block();
    // Set the paragraph-style on the block
    KoParagraphStyle *paragraphStyle = d->textSharedData->paragraphStyle(styleName, d->stylesDotXml);
    if (!paragraphStyle) {
        paragraphStyle = d->styleManager->defaultParagraphStyle();
    }
    if (paragraphStyle) {
        // Apply list style when loading a list but we don't have a list style
        paragraphStyle->applyStyle(block, (d->currentListLevel > 1) &&
                                   d->currentLists[d->currentListLevel - 2] && !d->currentListStyle);
    }

    QTextCharFormat cf = cursor.charFormat(); // store the current cursor char format

    bool stripLeadingSpace = true;
    loadSpan(element, cursor, &stripLeadingSpace);
    cursor.setCharFormat(cf);   // restore the cursor char format

    if ((block.blockFormat().hasProperty(KoParagraphStyle::OutlineLevel)) && (level == -1)) {
        level = block.blockFormat().property(KoParagraphStyle::OutlineLevel).toInt();
    } else {
        if (level == -1)
            level = 1;
        QTextBlockFormat blockFormat;
        blockFormat.setProperty(KoParagraphStyle::OutlineLevel, level);
        cursor.mergeBlockFormat(blockFormat);
    }

    if (element.hasAttributeNS(KoXmlNS::text, "is-list-header")) {
        QTextBlockFormat blockFormat;
        blockFormat.setProperty(KoParagraphStyle::IsListHeader, element.attributeNS(KoXmlNS::text, "is-list-header") == "true");
        cursor.mergeBlockFormat(blockFormat);
    }

    //we are defining our default behaviour here
    //Case 1: If text:outline-style is specified then we use the outline style to determine the numbering style
    //Case 2: If text:outline-style is not specified then if the <text:h> element is inside a <text:list> then it is numbered
    //        otherwise it is not
    KoListStyle *outlineStyle = d->styleManager->outlineStyle();
    if (!outlineStyle) {
        outlineStyle = d->styleManager->defaultOutlineStyle()->clone();
        d->styleManager->setOutlineStyle(outlineStyle);
    }

    //if outline style is not specified and this is not inside a list then we do not number it
    if (outlineStyle->styleId() == d->styleManager->defaultOutlineStyle()->styleId()) {
        if (d->currentListLevel <= 1) {
            QTextBlockFormat blockFormat;
            blockFormat.setProperty(KoParagraphStyle::UnnumberedListItem, true);
            cursor.mergeBlockFormat(blockFormat);
        } else { //inside a list then take the numbering from the list style
            int level = d->currentListLevel - 1;
            KoListLevelProperties llp;
            if (!d->currentListStyle->hasLevelProperties(level)) {
                // Look if one of the lower levels are defined to we can copy over that level.
                for(int i = level - 1; i >= 0; --i) {
                    if(d->currentLists[level - 1]->style()->hasLevelProperties(i)) {
                        llp = d->currentLists[level - 1]->style()->levelProperties(i);
                        break;
                    }
                }
            } else {
                llp = d->currentListStyle->levelProperties(level);
            }
            llp.setLevel(level);
            outlineStyle->setLevelProperties(llp);
        }
    }

    KoList *list = KoTextDocument(block.document()).headingList();
    if (!list) {
        list = d->list(block.document(), outlineStyle, false);
        KoTextDocument(block.document()).setHeadingList(list);
    }
    list->setStyle(outlineStyle);
    list->add(block, level);

    // attach Rdf to cursor.block()
    // remember inline Rdf metadata
    KoElementReference id;
    id.loadOdf(element);

    if (element.hasAttributeNS(KoXmlNS::xhtml, "property")
            || d->rdfIdList.contains(id.toString())) {
        QTextBlock block = cursor.block();
        KoTextInlineRdf* inlineRdf =
                new KoTextInlineRdf((QTextDocument*)block.document(), block);
        if (inlineRdf->loadOdf(element)) {
            KoTextInlineRdf::attach(inlineRdf, cursor);
        }
        else {
            delete inlineRdf;
            inlineRdf = 0;
        }
    }

#ifdef KOOPENDOCUMENTLOADER_DEBUG
    kDebug(32500) << "text-style:" << KoTextDebug::textAttributes(cursor.blockCharFormat());
#endif
}

void KoTextLoader::loadList(const KoXmlElement &element, QTextCursor &cursor)
{
    const bool numberedParagraph = element.localName() == "numbered-paragraph";

    QString styleName = element.attributeNS(KoXmlNS::text, "style-name", QString());
    KoListStyle *listStyle = d->textSharedData->listStyle(styleName, d->stylesDotXml);
    KoList *continuedList = 0;
    int level;

    if (d->currentLists[d->currentListLevel - 1] || d->currentListLevel == 1) {
        d->currentLists[d->currentListLevel - 1] = 0;
    } else {
        d->currentLists[d->currentListLevel - 1] = d->currentLists[d->currentListLevel - 2];
    }

    if (element.hasAttributeNS(KoXmlNS::text, "continue-list")) {
        if (d->xmlIdToListMap.contains(element.attributeNS(KoXmlNS::text, "continue-list", QString()))) {
           continuedList = d->xmlIdToListMap.value(element.attributeNS(KoXmlNS::text, "continue-list", QString()));
        }
    } else {
        //the ODF spec says that continue-numbering is considered only if continue-list is not specified
        if (element.hasAttributeNS(KoXmlNS::text, "continue-numbering")) {
            const QString continueNumbering = element.attributeNS(KoXmlNS::text, "continue-numbering", QString());
            if (continueNumbering == "true") {
                //since ODF spec says "and the numbering style of the preceding list is the same as the current list"
                KoList *prevList = d->previousList(d->currentListLevel);
                if (prevList && listStyle && prevList->style()->hasLevelProperties(d->currentListLevel)
                        && listStyle->hasLevelProperties(d->currentListLevel)
                        && (prevList->style()->levelProperties(d->currentListLevel).style() ==
                       listStyle->levelProperties(d->currentListLevel).style())) {
                   continuedList = prevList;
                }
            }
        }
    }

    // TODO: get level from the style, if it has a style:list-level attribute (new in ODF-1.2)
    if (numberedParagraph) {
        if (element.hasAttributeNS(KoXmlNS::text, "list-id")) {
            QString listId = element.attributeNS(KoXmlNS::text, "list-id");
            if (d->numberedParagraphListId.contains(listId)) {
                d->currentLists.fill(d->numberedParagraphListId.value(listId));
            } else {
                KoList *currentList = d->list(cursor.block().document(), listStyle, false);
                d->currentLists.fill(currentList);
                d->numberedParagraphListId.insert(listId, currentList);
            }
        } else {
            d->currentLists.fill(d->list(cursor.block().document(), listStyle, true));
        }

        level = element.attributeNS(KoXmlNS::text, "level", "1").toInt();

        d->currentListStyle = listStyle;
    } else {
        if (!listStyle)
            listStyle = d->currentListStyle;
        level = d->currentListLevel++;

        KoList *currentList = d->currentLists[d->currentListLevel - 2];
        if (!currentList) {
            currentList = d->list(cursor.block().document(), listStyle, false);
            currentList->setListContinuedFrom(continuedList);
            d->currentLists[d->currentListLevel - 2] = currentList;
        }
        d->currentListStyle = listStyle;
    }

    if (element.hasAttributeNS(KoXmlNS::xml, "id")) {
        d->xmlIdToListMap.insert(element.attributeNS(KoXmlNS::xml, "id", QString()), d->currentLists[d->currentListLevel - 2]);
    }

    if (level < 0 || level > 10) { // should not happen but if it does then we should not crash/assert
        kWarning() << "Out of bounds list-level=" << level;
        level = qBound(0, level, 10);
    }

    if (! numberedParagraph) {
        d->setCurrentList(d->currentLists[d->currentListLevel - 2], level);
    }

#ifdef KOOPENDOCUMENTLOADER_DEBUG
    if (d->currentListStyle)
        kDebug(32500) << "styleName =" << styleName << "listStyle =" << d->currentListStyle->name()
                      << "level =" << level << "hasLevelProperties =" << d->currentListStyle->hasLevelProperties(level)
                         //<<" style="<<props.style()<<" prefix="<<props.listItemPrefix()<<" suffix="<<props.listItemSuffix()
                         ;
    else
        kDebug(32500) << "styleName =" << styleName << " currentListStyle = 0";
#endif

    KoXmlElement e;
    QList<KoXmlElement> childElementsList;

    QString generatedXmlString;
    KoXmlDocument doc;
    QXmlStreamReader reader;

    for (KoXmlNode _node = element.firstChild(); !_node.isNull(); _node = _node.nextSibling()) {
        if (!(e = _node.toElement()).isNull()) {
            if ((e.attributeNS(KoXmlNS::delta, "insertion-type") == "insert-around-content") || (e.localName() == "remove-leaving-content-start")) {
                //Check whether this is a list-item split or a merge
                if ((e.localName() == "remove-leaving-content-start") && d->checkForListItemSplit(e)) {
                    _node = d->loadListItemSplit(e, &generatedXmlString);
                } else {
                    _node = loadDeleteMerges(e,&generatedXmlString);
                }

                //Parse and Load the generated xml
                QString errorMsg;
                int errorLine, errorColumn;

                reader.addData(generatedXmlString);
                reader.setNamespaceProcessing(true);

                bool ok = doc.setContent(&reader, &errorMsg, &errorLine, &errorColumn);
                QDomDocument dom;
                if (ok) {
                    KoXmlElement childElement;
                    forEachElement (childElement, doc.documentElement()) {
                        childElementsList.append(childElement);
                    }
                }
            } else {
                childElementsList.append(e);
            }
        }
    }

    // Iterate over list items and add them to the textlist
    bool firstTime = true;
    foreach (e, childElementsList) {
        if (e.localName() == "removed-content") {
            QString changeId = e.attributeNS(KoXmlNS::delta, "removal-change-idref");
            int deleteStartPosition = cursor.position();
            d->openChangeRegion(e);
            KoXmlElement deletedElement;
            forEachElement(deletedElement, e) {
                if (!firstTime && !numberedParagraph)
                    cursor.insertBlock(d->defaultBlockFormat, d->defaultCharFormat);
                firstTime = false;
                loadListItem(deletedElement, cursor, level);
            }
            d->closeChangeRegion(e);
            if(!d->checkForDeleteMerge(cursor, changeId, deleteStartPosition)) {
                QTextCursor tempCursor(cursor);
                tempCursor.setPosition(deleteStartPosition);
                KoDeleteChangeMarker *marker = d->insertDeleteChangeMarker(tempCursor, changeId);
                d->deleteChangeMarkerMap.insert(marker, QPair<int,int>(deleteStartPosition+1, cursor.position()));
            }
        } else {
            if (!firstTime && !numberedParagraph)
                cursor.insertBlock(d->defaultBlockFormat, d->defaultCharFormat);
            firstTime = false;
            loadListItem(e, cursor, level);
        }
    }

    if (numberedParagraph || --d->currentListLevel == 1) {
        d->currentListStyle = 0;
        d->currentLists.fill(0);
    }
}

void KoTextLoader::loadListItem(KoXmlElement &e, QTextCursor &cursor, int level)
{
    bool numberedParagraph = e.parentNode().toElement().localName() == "numbered-paragraph";

    if (!numberedParagraph && e.parentNode().toElement().localName() == "removed-content") {
        numberedParagraph = e.parentNode().parentNode().toElement().localName() == "numbered-paragraph";
    }

    if (e.isNull() || e.namespaceURI() != KoXmlNS::text)
        return;

    const bool listHeader = e.tagName() == "list-header";

    if (!numberedParagraph && e.tagName() != "list-item" && !listHeader)
        return;

    if (!e.attributeNS(KoXmlNS::delta, "insertion-type").isEmpty()) {
        d->openChangeRegion(e);
    } else if (!e.attributeNS(KoXmlNS::ac, "change001").isEmpty()) {
        d->openChangeRegion(e);
    }

    QTextBlock current = cursor.block();

    QTextBlockFormat blockFormat;
    if (numberedParagraph) {
        if (e.localName() == "p") {
            loadParagraph(e, cursor);
        } else if (e.localName() == "h") {
            loadHeading(e, cursor);
        }
        blockFormat.setProperty(KoParagraphStyle::ListLevel, level);
    } else {
        loadBody(e, cursor);
    }

    if (!cursor.blockFormat().boolProperty(KoParagraphStyle::ForceDisablingList)) {
        if (!current.textList()) {
            if (!d->currentLists[level - 1]->style()->hasLevelProperties(level)) {
                KoListLevelProperties llp;
                // Look if one of the lower levels are defined to we can copy over that level.
                for(int i = level - 1; i >= 0; --i) {
                    if(d->currentLists[level - 1]->style()->hasLevelProperties(i)) {
                        llp = d->currentLists[level - 1]->style()->levelProperties(i);
                        break;
                    }
                }
                llp.setLevel(level);
            // TODO make the 10 configurable
                llp.setIndent(level * 10.0);
                d->currentLists[level - 1]->style()->setLevelProperties(llp);
            }

            d->currentLists[level - 1]->add(current, level);
        }

        if (listHeader)
            blockFormat.setProperty(KoParagraphStyle::IsListHeader, true);

        if (e.hasAttributeNS(KoXmlNS::text, "start-value")) {
            int startValue = e.attributeNS(KoXmlNS::text, "start-value", QString()).toInt();
            blockFormat.setProperty(KoParagraphStyle::ListStartValue, startValue);
        }


        // mark intermediate paragraphs as unnumbered items
        QTextCursor c(current);
        c.mergeBlockFormat(blockFormat);
        while (c.block() != cursor.block()) {
            c.movePosition(QTextCursor::NextBlock);
            if (c.block().textList()) // a sublist
                break;
            blockFormat = c.blockFormat();
            blockFormat.setProperty(listHeader ? KoParagraphStyle::IsListHeader : KoParagraphStyle::UnnumberedListItem, true);
            c.setBlockFormat(blockFormat);
            d->currentLists[level - 1]->add(c.block(), level);
        }
    }

    if (!e.attributeNS(KoXmlNS::delta, "insertion-type").isEmpty())
        d->closeChangeRegion(e);
    kDebug(32500) << "text-style:" << KoTextDebug::textAttributes(cursor.blockCharFormat());
}

bool KoTextLoader::Private::checkForListItemSplit(const KoXmlElement &element)
{
    QString endId = element.attributeNS(KoXmlNS::delta, "end-element-idref");
    int insertedListItems = 0;
    KoXmlElement currentElement = element;
    bool isSplitListItem = false;

    while(true) {
        currentElement = currentElement.nextSibling().toElement();

        if (currentElement.isNull()) {
            continue;
        }

        if ((currentElement.localName() == "list-item") &&
                (currentElement.attributeNS(KoXmlNS::delta, "insertion-type") == "insert-around-content")) {
            insertedListItems++;
        }

        if ((currentElement.localName() == "remove-leaving-content-end") &&
                (currentElement.attributeNS(KoXmlNS::delta, "end-element-id") == endId)) {
            break;
        }
    }

    isSplitListItem = (insertedListItems > 1)?true:false;
    return isSplitListItem;
}

KoXmlNode KoTextLoader::Private::loadListItemSplit(const KoXmlElement &elem, QString *generatedXmlString)
{
    KoXmlNode lastProcessedNode = elem;

    nameSpacesList.clear();
    nameSpacesList.append(KoXmlNS::split);
    nameSpacesList.append(KoXmlNS::delta);

    QString generatedXml;
    QTextStream xmlStream(&generatedXml);

    static int splitIdCounter = 0;
    bool splitStarted = false;

    QString endId = elem.attributeNS(KoXmlNS::delta, "end-element-idref");
    QString changeId = elem.attributeNS(KoXmlNS::delta, "removal-change-idref");

    while (true) {
        KoXmlElement element;
        lastProcessedNode = lastProcessedNode.nextSibling();
        bool isElementNode = lastProcessedNode.isElement();

        if (isElementNode)
            element = lastProcessedNode.toElement();

        if (isElementNode && (element.localName() == "remove-leaving-content-start")) {
            //Ignore this...
        } else if (isElementNode && (element.localName() == "remove-leaving-content-end")) {
            if(element.attributeNS(KoXmlNS::delta, "end-element-id") == endId) {
                break;
            }
        } else if (isElementNode && (element.attributeNS(KoXmlNS::delta, "insertion-type") == "insert-around-content")) {
            copyTagStart(element, xmlStream, true);
            KoXmlElement childElement;
            forEachElement(childElement, element) {
                if (childElement.attributeNS(KoXmlNS::delta, "insertion-type") == "insert-around-content") {
                    copyTagStart(childElement, xmlStream, true);

                    if (splitStarted) {
                        generatedXml.remove((generatedXml.length() - 1), 1);
                        xmlStream << " ns1:split-id=\"split" << splitIdCounter << "\"";
                        xmlStream << " ns1:insertion-change-idref=\"" << changeId << "\"";
                        xmlStream << " ns1:insertion-type=\"split\"";
                        xmlStream << ">";
                        splitStarted = false;
                        splitIdCounter++;
                    } else {
                        generatedXml.remove((generatedXml.length() - 1), 1);
                        xmlStream << " ns0:split001-idref=\"split" << splitIdCounter << "\"";
                        xmlStream << ">";
                        splitStarted = true;
                    }

                    copyNode(childElement, xmlStream, true);
                    copyTagEnd(childElement, xmlStream);
                } else {
                    copyNode(childElement, xmlStream);
                }
            }
            copyTagEnd(element, xmlStream);
        } else {
            copyNode(element, xmlStream);
        }
    }

    QTextStream docStream(generatedXmlString);
    docStream << "<generated-xml";
    for (int i=0; i<nameSpacesList.size();i++) {
        docStream << " xmlns:ns" << i << "=";
        docStream << "\"" << nameSpacesList.at(i) << "\"";
    }
    docStream << ">";
    docStream << generatedXml;
    docStream << "</generated-xml>";

    return lastProcessedNode;
}

void KoTextLoader::loadSection(const KoXmlElement &sectionElem, QTextCursor &cursor)
{
    KoSection *section = new KoSection();
    if (!section->loadOdf(sectionElem, d->textSharedData, d->stylesDotXml)) {
        delete section;
        kWarning(32500) << "Could not load section";
        return;
    }

    QVariant v;
    v.setValue<void *>(section);
    d->openingSections.append(v);

    loadBody(sectionElem, cursor);

    // Close the section on the last block of text we have loaded just now.
    KoSectionEnd *sectionEnd = new KoSectionEnd();
    sectionEnd->name = section->name();
    v.setValue<void *>(sectionEnd);

    QTextBlock block = cursor.block();
    QTextBlockFormat format = block.blockFormat();
    QVariant listv;
    listv = format.property(KoParagraphStyle::SectionEndings);
    QList<QVariant> sectionEndings = v.value<QList<QVariant> >();
    sectionEndings.append(v);
    listv.setValue<QList<QVariant> >(sectionEndings);
    format.setProperty(KoParagraphStyle::SectionEndings, listv);
    cursor.setBlockFormat(format);
}

void KoTextLoader::loadNote(const KoXmlElement &noteElem, QTextCursor &cursor)
{
    KoInlineTextObjectManager *textObjectManager = KoTextDocument(cursor.block().document()).inlineTextObjectManager();
    if (textObjectManager) {
        QString className = noteElem.attributeNS(KoXmlNS::text, "note-class");
        KoInlineNote *note = 0;
        int position = cursor.position(); // need to store this as the following might move is
        if (className == "footnote") {
            note = new KoInlineNote(KoInlineNote::Footnote);
            note->setMotherFrame(KoTextDocument(cursor.block().document()).auxillaryFrame());
        } else {
            note = new KoInlineNote(KoInlineNote::Endnote);
            note->setMotherFrame(KoTextDocument(cursor.block().document()).auxillaryFrame());
        }
        if (note->loadOdf(noteElem, d->context)) {
            cursor.setPosition(position); // restore the position before inserting the note
            textObjectManager->insertInlineObject(cursor, note);
        } else {
            cursor.setPosition(position); // restore the position
            delete note;
        }
    }
}

void KoTextLoader::loadCite(const KoXmlElement &noteElem, QTextCursor &cursor)
{
    KoInlineTextObjectManager *textObjectManager = KoTextDocument(cursor.block().document()).inlineTextObjectManager();
    if (textObjectManager) {
        //Now creating citation with default type KoInlineCite::Citation.
        KoInlineCite *cite = new KoInlineCite(KoInlineCite::Citation);

        // the manager is needed during loading so set it now
        cite->setManager(textObjectManager);
        if (cite->loadOdf(noteElem, d->context)) {
            textObjectManager->insertInlineObject(cursor, cite);
        } else {
            delete cite;
        }
    }
}

void KoTextLoader::loadText(const QString &fulltext, QTextCursor &cursor,
                            bool *stripLeadingSpace, bool isLastNode)
{
    QString text = normalizeWhitespace(fulltext, *stripLeadingSpace);
#ifdef KOOPENDOCUMENTLOADER_DEBUG
    kDebug(32500) << "  <text> text=" << text << text.length() << *stripLeadingSpace;
#endif

    if (!text.isEmpty()) {
        // if present text ends with a space,
        // we can remove the leading space in the next text
        *stripLeadingSpace = text[text.length() - 1].isSpace();

        if (d->changeTracker && d->changeStack.count()) {
            QTextCharFormat format;
            format.setProperty(KoCharacterStyle::ChangeTrackerId, d->changeStack.top());
            cursor.mergeCharFormat(format);
        } else {
            QTextCharFormat format = cursor.charFormat();
            if (format.hasProperty(KoCharacterStyle::ChangeTrackerId)) {
                format.clearProperty(KoCharacterStyle::ChangeTrackerId);
                cursor.setCharFormat(format);
            }
        }
        cursor.insertText(text);

        if (d->loadSpanLevel == 1 && isLastNode
                && cursor.position() > d->loadSpanInitialPos) {
            QTextCursor tempCursor(cursor);
            tempCursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, 1); // select last char loaded
            if (tempCursor.selectedText() == " " && *stripLeadingSpace) {            // if it's a collapsed blankspace
                tempCursor.removeSelectedText();                                    // remove it
            }
        }
    }
}

void KoTextLoader::loadSpan(const KoXmlElement &element, QTextCursor &cursor, bool *stripLeadingSpace)
{
#ifdef KOOPENDOCUMENTLOADER_DEBUG
    kDebug(32500) << "text-style:" << KoTextDebug::textAttributes(cursor.blockCharFormat());
#endif
    Q_ASSERT(stripLeadingSpace);
    if (d->loadSpanLevel++ == 0)
        d->loadSpanInitialPos = cursor.position();

    for (KoXmlNode node = element.firstChild(); !node.isNull(); node = node.nextSibling()) {
        KoXmlElement ts = node.toElement();
        const QString localName(ts.localName());
        const bool isTextNS = ts.namespaceURI() == KoXmlNS::text;
        const bool isDrawNS = ts.namespaceURI() == KoXmlNS::draw;
        const bool isDeltaNS = ts.namespaceURI() == KoXmlNS::delta;
        //        const bool isOfficeNS = ts.namespaceURI() == KoXmlNS::office;

#ifdef KOOPENDOCUMENTLOADER_DEBUG
        kDebug(32500) << "load" << localName << *stripLeadingSpace << node.toText().data();
#endif

        if (!(isTextNS && localName == "span")) {
            d->endCharStyle = 0;
        }

        if (node.isText()) {
            bool isLastNode = node.nextSibling().isNull();
            loadText(node.toText().data(), cursor, stripLeadingSpace,
                     isLastNode);
        } else if (isDeltaNS && localName == "inserted-text-start") {
            d->openChangeRegion(ts);
        } else if (isDeltaNS && localName == "inserted-text-end") {
            d->closeChangeRegion(ts);
        } else if (isDeltaNS && localName == "remove-leaving-content-start") {
            d->openChangeRegion(ts);
        } else if (isDeltaNS && localName == "remove-leaving-content-end") {
            d->closeChangeRegion(ts);
        } else if (isDeltaNS && localName == "removed-content") {
            QTextCharFormat cf = cursor.charFormat(); // store the current cursor char format
            QString changeId = ts.attributeNS(KoXmlNS::delta, "removal-change-idref");
            int deleteStartPosition = cursor.position();
            bool stripLeadingSpace = true;
            d->openChangeRegion(ts);
            loadSpan(ts,cursor,&stripLeadingSpace);
            d->closeChangeRegion(ts);
            if(!d->checkForDeleteMerge(cursor, changeId, deleteStartPosition)) {
                QTextCursor tempCursor(cursor);
                tempCursor.setPosition(deleteStartPosition);
                KoDeleteChangeMarker *marker = d->insertDeleteChangeMarker(tempCursor, changeId);
                d->deleteChangeMarkerMap.insert(marker, QPair<int,int>(deleteStartPosition+1, cursor.position()));
            }
            cursor.setCharFormat(cf); // restore the cursor char format
        } else if (isDeltaNS && localName == "merge") {
            loadMerge(ts, cursor);
        } else if (isTextNS && localName == "change-start") { // text:change-start
            d->openChangeRegion(ts);
        } else if (isTextNS && localName == "change-end") {
            d->closeChangeRegion(ts);
        } else if (d->changeTracker && isTextNS && localName == "change") {
            QString id = ts.attributeNS(KoXmlNS::text, "change-id");
            int changeId = d->changeTracker->getLoadedChangeId(id);
            if (changeId) {
                if (d->changeStack.count() && (d->changeStack.top() != changeId))
                    d->changeTracker->setParent(changeId, d->changeStack.top());
                KoDeleteChangeMarker *deleteChangemarker = new KoDeleteChangeMarker(d->changeTracker);
                deleteChangemarker->setChangeId(changeId);
                KoChangeTrackerElement *changeElement = d->changeTracker->elementById(changeId);
                changeElement->setDeleteChangeMarker(deleteChangemarker);
                changeElement->setEnabled(true);
                KoInlineTextObjectManager *textObjectManager = KoTextDocument(cursor.block().document()).inlineTextObjectManager();
                if (textObjectManager) {
                    textObjectManager->insertInlineObject(cursor, deleteChangemarker);
                }

                loadDeleteChangeWithinPorH(id, cursor);
            }
        } else if (isTextNS && localName == "span") { // text:span
#ifdef KOOPENDOCUMENTLOADER_DEBUG
            kDebug(32500) << "  <span> localName=" << localName;
#endif
            if (!ts.attributeNS(KoXmlNS::delta, "insertion-type").isEmpty())
                d->openChangeRegion(ts);
            QString styleName = ts.attributeNS(KoXmlNS::text, "style-name", QString());

            QTextCharFormat cf = cursor.charFormat(); // store the current cursor char format

            KoCharacterStyle *characterStyle = d->textSharedData->characterStyle(styleName, d->stylesDotXml);
            if (characterStyle) {
                characterStyle->applyStyle(&cursor);
                if (ts.firstChild().isNull()) {
                    // empty span so let's save the characterStyle for possible use at end of par
                    d->endCharStyle = characterStyle;
                }
            } else if (!styleName.isEmpty()) {
                kWarning(32500) << "character style " << styleName << " not found";
            }

            loadSpan(ts, cursor, stripLeadingSpace);   // recurse
            cursor.setCharFormat(cf); // restore the cursor char format
            if (!ts.attributeNS(KoXmlNS::delta, "insertion-type").isEmpty())
                d->closeChangeRegion(ts);
        } else if (isTextNS && localName == "s") { // text:s
            int howmany = 1;
            if (!ts.attributeNS(KoXmlNS::delta, "insertion-type").isEmpty())
                d->openChangeRegion(ts);
            if (ts.hasAttributeNS(KoXmlNS::text, "c")) {
                howmany = ts.attributeNS(KoXmlNS::text, "c", QString()).toInt();
            }
            cursor.insertText(QString().fill(32, howmany));
            if (!ts.attributeNS(KoXmlNS::delta, "insertion-type").isEmpty())
                d->closeChangeRegion(ts);
            *stripLeadingSpace = false;
        } else if ( (isTextNS && localName == "note")) { // text:note
            loadNote(ts, cursor);
        } else if (isTextNS && localName == "bibliography-mark") { // text:bibliography-mark
            loadCite(ts,cursor);
        } else if (isTextNS && localName == "tab") { // text:tab
            cursor.insertText("\t");
            *stripLeadingSpace = false;
        } else if (isTextNS && localName == "a") { // text:a
            if (!ts.attributeNS(KoXmlNS::delta, "insertion-type").isEmpty())
                d->openChangeRegion(ts);
            QString target = ts.attributeNS(KoXmlNS::xlink, "href");
            QString styleName = ts.attributeNS(KoXmlNS::text, "style-name", QString());
            QTextCharFormat cf = cursor.charFormat(); // store the current cursor char format

            if (!styleName.isEmpty()) {
                KoCharacterStyle *characterStyle = d->textSharedData->characterStyle(styleName, d->stylesDotXml);
                if (characterStyle) {
                    characterStyle->applyStyle(&cursor);
                } else {
                    kWarning(32500) << "character style " << styleName << " not found";
                }
            }
            QTextCharFormat newCharFormat = cursor.charFormat();
            newCharFormat.setAnchor(true);
            newCharFormat.setProperty(KoCharacterStyle::AnchorType, KoCharacterStyle::Anchor);
            newCharFormat.setAnchorHref(target);
            cursor.setCharFormat(newCharFormat);

            loadSpan(ts, cursor, stripLeadingSpace);   // recurse
            cursor.setCharFormat(cf); // restore the cursor char format

            if (!ts.attributeNS(KoXmlNS::delta, "insertion-type").isEmpty())
                d->closeChangeRegion(ts);
        } else if (isTextNS && localName == "line-break") { // text:line-break
#ifdef KOOPENDOCUMENTLOADER_DEBUG
            kDebug(32500) << "  <line-break> Node localName=" << localName;
#endif
            cursor.insertText(QChar(0x2028));
            *stripLeadingSpace = false;
        } else if (isTextNS && localName == "soft-page-break") { // text:soft-page-break
            KoInlineTextObjectManager *textObjectManager = KoTextDocument(cursor.block().document()).inlineTextObjectManager();
            if (textObjectManager) {
                textObjectManager->insertInlineObject(cursor, new KoTextSoftPageBreak());
            }
        } else if (isTextNS && localName == "meta") {
#ifdef KOOPENDOCUMENTLOADER_DEBUG
            kDebug(30015) << "loading a text:meta";
#endif
            KoInlineTextObjectManager *textObjectManager = KoTextDocument(cursor.block().document()).inlineTextObjectManager();
            if (textObjectManager) {
                const QTextDocument *document = cursor.block().document();
                KoTextMeta* startmark = new KoTextMeta(document);
                textObjectManager->insertInlineObject(cursor, startmark);

                // Add inline Rdf here.
                KoElementReference id;
                id.loadOdf(ts);

                if (ts.hasAttributeNS(KoXmlNS::xhtml, "property")
                        || (id.isValid() && d->rdfIdList.contains(id.toString()))) {
                    KoTextInlineRdf* inlineRdf =
                            new KoTextInlineRdf((QTextDocument*)document, startmark);
                    if (inlineRdf->loadOdf(ts)) {
                        startmark->setInlineRdf(inlineRdf);
                    }
                    else {
                        delete inlineRdf;
                        inlineRdf = 0;
                    }

                }

                loadSpan(ts, cursor, stripLeadingSpace);   // recurse

                KoTextMeta* endmark = new KoTextMeta(document);
                textObjectManager->insertInlineObject(cursor, endmark);
                startmark->setEndBookmark(endmark);
            }
        }
        // text:bookmark, text:bookmark-start and text:bookmark-end
        else if (isTextNS && (localName == "bookmark" || localName == "bookmark-start" || localName == "bookmark-end")) {

            KoInlineTextObjectManager *textObjectManager = KoTextDocument(cursor.block().document()).inlineTextObjectManager();

            KoBookmark *bookmark = new KoBookmark(cursor.block().document());
            bookmark->setManager(textObjectManager);
            if (textObjectManager && bookmark->loadOdf(ts, d->context)) {
                textObjectManager->insertInlineObject(cursor, bookmark);
            }
            else {
                kWarning(32500) << "Could not load bookmark";
                delete bookmark;
            }

        } else if (isTextNS && localName == "bookmark-ref") {
            QString bookmarkName = ts.attribute("ref-name");
            QTextCharFormat cf = cursor.charFormat(); // store the current cursor char format
            if (!bookmarkName.isEmpty()) {
                QTextCharFormat linkCf(cf); // and copy it to alter it
                linkCf.setAnchor(true);
                linkCf.setProperty(KoCharacterStyle::AnchorType, KoCharacterStyle::Bookmark);
                QStringList anchorName;
                anchorName << bookmarkName;
                linkCf.setAnchorHref('#'+ bookmarkName);
                cursor.setCharFormat(linkCf);
            }
            // TODO add support for loading text:reference-format
            loadSpan(ts, cursor, stripLeadingSpace);   // recurse
            cursor.setCharFormat(cf);   // restore the cursor char format
        } else if (isTextNS && localName == "number") { // text:number
            /*                ODF Spec, §4.1.1, Formatted Heading Numbering
            If a heading has a numbering applied, the text of the formatted number can be included in a
            <text:number> element. This text can be used by applications that do not support numbering of
            headings, but it will be ignored by applications that support numbering.                   */
        } else if (isTextNS && localName == "dde-connection") {
            // TODO: load actual connection (or at least preserve it)
            // For now: just load the text
            for (KoXmlNode n = ts.firstChild(); !n.isNull(); n = n.nextSibling()) {
                if (n.isText()) {
                    loadText(n.toText().data(), cursor, stripLeadingSpace, false);
                }
            }
        } else if ((isDrawNS) && localName == "a") { // draw:a
            loadShapeWithHyperLink(ts, cursor);
        } else if (isDrawNS) {
            loadShape(ts, cursor);
        } else {
            KoInlineObject *obj = KoInlineObjectRegistry::instance()->createFromOdf(ts, d->context);

            KoInlineTextObjectManager *textObjectManager = KoTextDocument(cursor.block().document()).inlineTextObjectManager();
            if (obj && textObjectManager) {
                KoVariableManager *varManager = textObjectManager->variableManager();
                if (varManager) {
                    textObjectManager->insertInlineObject(cursor, obj);
                    // we need to update whitespace stripping here so we don't remove to many whitespaces.
                    // this is simplified as it assumes the first child it the text item but that should be the case
                    // most of the time with variables so it should be fine.
                    KoXmlNode child = ts.firstChild();
                    if (child.isText()) {
                        QString text = normalizeWhitespace(child.toText().data(), *stripLeadingSpace);
                        if (!text.isEmpty()) {
                            // if present text ends with a space,
                            // we can remove the leading space in the next text
                            *stripLeadingSpace = text[text.length() - 1].isSpace();
                        }
                    }
                }
            } else {
#if 0 //1.6:
                bool handled = false;
                // Check if it's a variable
                KoVariable *var = context.variableCollection().loadOasisField(textDocument(), ts, context);
                if (var) {
                    textData = "#";     // field placeholder
                    customItem = var;
                    handled = true;
                }
                if (!handled) {
                    handled = textDocument()->loadSpanTag(ts, context, this, pos, textData, customItem);
                    if (!handled) {
                        kWarning(32500) << "Ignoring tag " << ts.tagName();
                        context.styleStack().restore();
                        continue;
                    }
                }
#else
#ifdef KOOPENDOCUMENTLOADER_DEBUG
                kDebug(32500) << "Node '" << localName << "' unhandled";
#endif
            }
#endif
        }
    }
    --d->loadSpanLevel;
}

void KoTextLoader::loadDeleteChangeWithinPorH(QString id, QTextCursor &cursor)
{
    int startPosition = cursor.position();
    int changeId = d->changeTracker->getLoadedChangeId(id);

    QTextCharFormat charFormat = cursor.block().charFormat();
    QTextBlockFormat blockFormat = cursor.block().blockFormat();

    if (changeId) {
        KoChangeTrackerElement *changeElement = d->changeTracker->elementById(changeId);
        KoXmlElement element = d->deleteChangeTable.value(id);
        KoXmlElement tag;
        int loadedTags = 0;
        forEachElement(tag, element) {
            QString localName = tag.localName();
            if (localName == "p") {
                if (loadedTags)
                    cursor.insertBlock(blockFormat, charFormat);
                bool stripLeadingSpace = true;
                loadSpan(tag, cursor, &stripLeadingSpace);
                loadedTags++;
            } else if (localName == "unordered-list" || localName == "ordered-list" // OOo-1.1
                       || localName == "list" || localName == "numbered-paragraph") {  // OASIS
                cursor.insertBlock(blockFormat, charFormat);
                loadList(tag, cursor);
            } else if (localName == "table") {
                loadTable(tag, cursor);
            }
        }

        int endPosition = cursor.position();

        //Set the char format to the changeId
        cursor.setPosition(startPosition);
        cursor.setPosition(endPosition, QTextCursor::KeepAnchor);
        QTextCharFormat format;
        format.setProperty(KoCharacterStyle::ChangeTrackerId, changeId);
        cursor.mergeCharFormat(format);

        //Get the QTextDocumentFragment from the selection and store it in the changeElement
        QTextDocumentFragment deletedFragment = KoChangeTracker::generateDeleteFragment(cursor, changeElement->getDeleteChangeMarker());
        changeElement->setDeleteData(deletedFragment);

        //Now Remove this from the document. Will be re-inserted whenever changes have to be seen
        cursor.removeSelectedText();
    }
}

void KoTextLoader::loadMerge(const KoXmlElement &element, QTextCursor &cursor)
{
    d->openChangeRegion(element);
    QString changeId = element.attributeNS(KoXmlNS::delta, "removal-change-idref");
    int deleteStartPosition = cursor.position();

    for (KoXmlNode node = element.firstChild(); !node.isNull(); node = node.nextSibling()) {
        KoXmlElement ts = node.toElement();
        const QString localName(ts.localName());
        const bool isDeltaNS = ts.namespaceURI() == KoXmlNS::delta;

        if (isDeltaNS && localName == "leading-partial-content") {
            bool stripLeadingSpaces = false;
            loadSpan(ts, cursor, &stripLeadingSpaces);
        } else if (isDeltaNS && localName == "intermediate-content") {
            if (ts.hasChildNodes()) {
                if (ts.firstChild().toElement().localName() != "table") {
                    cursor.insertBlock(d->defaultBlockFormat, d->defaultCharFormat);
                }
                loadBody(ts, cursor);
            }
        } else if (isDeltaNS && localName == "trailing-partial-content") {
            if (ts.previousSibling().lastChild().toElement().localName() != "table") {
                cursor.insertBlock(d->defaultBlockFormat, d->defaultCharFormat);
            }
            loadBody(ts, cursor);
        }
    }

    if(!d->checkForDeleteMerge(cursor, changeId, deleteStartPosition)) {
        QTextCursor tempCursor(cursor);
        tempCursor.setPosition(deleteStartPosition);
        KoDeleteChangeMarker *marker = d->insertDeleteChangeMarker(tempCursor, changeId);
        d->deleteChangeMarkerMap.insert(marker, QPair<int,int>(deleteStartPosition+1, cursor.position()));
    }
    d->closeChangeRegion(element);
}

KoDeleteChangeMarker * KoTextLoader::Private::insertDeleteChangeMarker(QTextCursor &cursor, const QString &id)
{
    KoDeleteChangeMarker *retMarker = NULL;
    int changeId = changeTracker->getLoadedChangeId(id);
    if (changeId) {
        KoDeleteChangeMarker *deleteChangemarker = new KoDeleteChangeMarker(changeTracker);
        deleteChangemarker->setChangeId(changeId);
        KoChangeTrackerElement *changeElement = changeTracker->elementById(changeId);
        changeElement->setDeleteChangeMarker(deleteChangemarker);
        changeElement->setEnabled(true);
        changeElement->setChangeType(KoGenChange::DeleteChange);
        KoInlineTextObjectManager *textObjectManager = KoTextDocument(cursor.block().document()).inlineTextObjectManager();
        if (textObjectManager) {
            deleteChangemarker->updatePosition(cursor.block().document(), cursor.position(), QTextCharFormat());
            textObjectManager->insertInlineObject(cursor, deleteChangemarker);
        }
        retMarker = deleteChangemarker;
    }
    return retMarker;
}

bool KoTextLoader::Private::checkForDeleteMerge(QTextCursor &cursor, const QString &id, int startPosition)
{
    bool result = false;

    int changeId = changeTracker->getLoadedChangeId(id);
    if (changeId) {
        KoChangeTrackerElement *changeElement = changeTracker->elementById(changeId);
        //Check if this change is at the beginning of the block and if there is a
        //delete-change at the end of the previous block with the same change-id
        //If both the conditions are true, then merge both these deletions.
        int prevChangeId = 0;
        if ( startPosition == (cursor.block().position())) {
            QTextCursor tempCursor(cursor);
            tempCursor.setPosition(cursor.block().previous().position() + cursor.block().previous().length() - 1);
            prevChangeId = tempCursor.charFormat().property(KoCharacterStyle::ChangeTrackerId).toInt();

            if (!prevChangeId) {
                KoInlineTextObjectManager *textObjectManager = KoTextDocument(cursor.block().document()).inlineTextObjectManager();
                if (textObjectManager) {
                    KoInlineObject *inlineObject = textObjectManager->inlineTextObject(tempCursor.charFormat());
                    KoDeleteChangeMarker *deleteChangeMarker = dynamic_cast<KoDeleteChangeMarker *>(inlineObject);
                    if (deleteChangeMarker) {
                        prevChangeId = deleteChangeMarker->changeId();
                    }
                }
            }

        } else {
            QTextCursor tempCursor(cursor);
            tempCursor.setPosition(startPosition - 1);
            prevChangeId = tempCursor.charFormat().property(KoCharacterStyle::ChangeTrackerId).toInt();
        }

        if ((prevChangeId) && (prevChangeId == changeId)) {
            QPair<int, int> deleteMarkerRange = deleteChangeMarkerMap.value(changeElement->getDeleteChangeMarker());
            deleteMarkerRange.second = cursor.position();
            deleteChangeMarkerMap.insert(changeElement->getDeleteChangeMarker(), deleteMarkerRange);
            result = true;
        }
    }
    return result;
}

void KoTextLoader::Private::processDeleteChange(QTextCursor &cursor)
{
    QList<KoDeleteChangeMarker *> markersList = deleteChangeMarkerMap.keys();

    KoDeleteChangeMarker *marker;
    foreach (marker, markersList) {
        if (!marker) {
            kWarning() << "Empty KoDeleteChangeMarker in the markerslist.";
            continue;
        }
        int changeId = marker->changeId();

        KoChangeTrackerElement *changeElement = changeTracker->elementById(changeId);
        QPair<int, int> rangeValue = deleteChangeMarkerMap.value(marker);
        int startPosition = rangeValue.first;
        int endPosition = rangeValue.second;

        cursor.setPosition(startPosition);
        cursor.setPosition(endPosition, QTextCursor::KeepAnchor);

        //Get the QTextDocumentFragment from the selection and store it in the changeElement
        QTextDocumentFragment deletedFragment = KoChangeTracker::generateDeleteFragment(cursor, changeElement->getDeleteChangeMarker());
        changeElement->setDeleteData(deletedFragment);

        cursor.removeSelectedText();
    }
}

void KoTextLoader::loadTable(const KoXmlElement &tableElem, QTextCursor &cursor)
{
    QTextTableFormat tableFormat;
    QString tableStyleName = tableElem.attributeNS(KoXmlNS::table, "style-name", "");
    if (!tableStyleName.isEmpty()) {
        KoTableStyle *tblStyle = d->textSharedData->tableStyle(tableStyleName, d->stylesDotXml);
        if (tblStyle)
            tblStyle->applyStyle(tableFormat);
    }

    QString tableTemplateName = tableElem.attributeNS(KoXmlNS::table, "template-name", "");
    if (! tableTemplateName.isEmpty()) {
        if(KoTextTableTemplate *tableTemplate = d->styleManager->tableTemplate(tableTemplateName)) {
            tableFormat.setProperty(KoTableStyle::TableTemplate, tableTemplate->styleId());
        }
    }

    if (tableElem.attributeNS(KoXmlNS::table, "use-banding-columns-styles", "false") == "true") {
        tableFormat.setProperty(KoTableStyle::UseBandingColumnStyles, true);
    }

    if (tableElem.attributeNS(KoXmlNS::table, "use-banding-rows-styles", "false") == "true") {
        tableFormat.setProperty(KoTableStyle::UseBandingRowStyles, true);
    }

    if (tableElem.attributeNS(KoXmlNS::table, "use-first-column-styles", "false") == "true") {
        tableFormat.setProperty(KoTableStyle::UseFirstColumnStyles, true);
    }

    if (tableElem.attributeNS(KoXmlNS::table, "use-first-row-styles", "false") == "true") {
        tableFormat.setProperty(KoTableStyle::UseFirstRowStyles, true);
    }

    if (tableElem.attributeNS(KoXmlNS::table, "use-last-column-styles", "false") == "true") {
        tableFormat.setProperty(KoTableStyle::UseLastColumnStyles, true);
    }

    if (tableElem.attributeNS(KoXmlNS::table, "use-last-row-styles", "false") == "true") {
        tableFormat.setProperty(KoTableStyle::UseLastRowStyles, true);
    }

    // Let's try to figure out when to hide the current block
    QTextBlock currentBlock = cursor.block();
    QTextTable *outerTable = cursor.currentTable();
    bool hide = cursor.position() == 0;
    if (outerTable) {
        QTextTableCell cell = outerTable->cellAt(cursor.position());
        if (cursor.position() == cell.firstCursorPosition().position()) {
            hide = true;
        }
    }
    if (!hide) {
        // Let's insert an extra block so that will be the one we hide instead
        cursor.insertBlock(cursor.blockFormat(), cursor.blockCharFormat());
        currentBlock = cursor.block();
    }


    if (d->changeTracker && d->changeStack.count()) {
        tableFormat.setProperty(KoCharacterStyle::ChangeTrackerId, d->changeStack.top());
    }
    if (tableElem.attributeNS(KoXmlNS::table, "protected", "false") == "true") {
        tableFormat.setProperty(KoTableStyle::TableIsProtected, true);
    }
    QTextTable *tbl = cursor.insertTable(1, 1, tableFormat);

    // 'Hide' the block before the table (possibly the extra block we just inserted)
    QTextBlockFormat blockFormat;
    //blockFormat.setFont(currentBlock.blockFormat().font());
    QTextCursor tmpCursor(currentBlock);
    blockFormat.setProperty(KoParagraphStyle::HiddenByTable, true);
    QVariant masterStyle = tableFormat.property(KoTableStyle::MasterPageName);
    if (!masterStyle.isNull()) {
        // if table has a master page style property, copy it to block before table, because
        // this block is a hidden block so let's make it belong to the same masterpage
        // so we don't end up with a page break at the wrong place
        blockFormat.setProperty(KoParagraphStyle::MasterPageName, masterStyle);
    }
    tmpCursor.setBlockFormat(blockFormat);

    KoTableColumnAndRowStyleManager tcarManager = KoTableColumnAndRowStyleManager::getManager(tbl);
    int rows = 0;
    int columns = 0;
    QList<QRect> spanStore; //temporary list to store spans until the entire table have been created
    KoXmlElement tblTag;
    int headingRowCounter = 0;
    QList<KoXmlElement> rowTags;

    forEachElement(tblTag, tableElem) {
        if (! tblTag.isNull()) {
            const QString tblLocalName = tblTag.localName();
            if (tblTag.namespaceURI() == KoXmlNS::table) {
                if (tblLocalName == "table-column") {
                    loadTableColumn(tblTag, tbl, columns);
                } else if (tblLocalName == "table-columns") {
                    KoXmlElement e;
                    forEachElement(e, tblTag) {
                        if (e.localName() == "table-column") {
                            loadTableColumn(e, tbl, columns);
                        }
                    }
                } else if (tblLocalName == "table-row") {
                    if (!tblTag.attributeNS(KoXmlNS::delta, "insertion-type").isEmpty())
                        d->openChangeRegion(tblTag);
                    loadTableRow(tblTag, tbl, spanStore, cursor, rows);
                    if (!tblTag.attributeNS(KoXmlNS::delta, "insertion-type").isEmpty())
                        d->closeChangeRegion(tblTag);
                } else if (tblLocalName == "table-rows") {
                    KoXmlElement subTag;
                    forEachElement(subTag, tblTag) {
                        if (!subTag.isNull()) {
                            if ((subTag.namespaceURI() == KoXmlNS::table) && (subTag.localName() == "table-row")) {
                                loadTableRow(subTag, tbl, spanStore, cursor, rows);
                            }
                        }
                    }
                } else if (tblLocalName == "table-header-rows") {
                    KoXmlElement subTag;
                    forEachElement(subTag, tblTag) {
                        if (!subTag.isNull()) {
                            if ((subTag.namespaceURI() == KoXmlNS::table) && (subTag.localName() == "table-row")) {
                                headingRowCounter++;
                                loadTableRow(subTag, tbl, spanStore, cursor, rows);
                            }
                        }
                    }
                }
            } else if(tblTag.namespaceURI() == KoXmlNS::delta) {
                if (tblLocalName == "removed-content")
                    d->openChangeRegion(tblTag);

                KoXmlElement deltaTblTag;
                forEachElement (deltaTblTag, tblTag) {
                    if (!deltaTblTag.isNull() && (deltaTblTag.namespaceURI() == KoXmlNS::table)) {
                        const QString deltaTblLocalName = deltaTblTag.localName();
                        if (deltaTblLocalName == "table-column") {
                            loadTableColumn(deltaTblTag, tbl, columns);
                        } else if (deltaTblLocalName == "table-row") {
                            loadTableRow(deltaTblTag, tbl, spanStore, cursor, rows);
                        }
                    }
                }

                if (tblLocalName == "removed-content")
                    d->closeChangeRegion(tblTag);
            }
        }
    }

    if (headingRowCounter > 0) {
        QTextTableFormat fmt = tbl->format();
        fmt.setProperty(KoTableStyle::NumberHeadingRows, headingRowCounter);
        tbl->setFormat(fmt);
    }

    // Finally create spans
    foreach(const QRect &span, spanStore) {
        tbl->mergeCells(span.y(), span.x(), span.height(), span.width()); // for some reason Qt takes row, column
    }
    cursor = tbl->lastCursorPosition();
    cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, 1);
}

void KoTextLoader::loadTableColumn(KoXmlElement &tblTag, QTextTable *tbl, int &columns)
{
    KoTableColumnAndRowStyleManager tcarManager = KoTableColumnAndRowStyleManager::getManager(tbl);
    int rows = tbl->rows();
    int repeatColumn = tblTag.attributeNS(KoXmlNS::table, "number-columns-repeated", "1").toInt();
    QString columnStyleName = tblTag.attributeNS(KoXmlNS::table, "style-name", "");
    if (!columnStyleName.isEmpty()) {
        KoTableColumnStyle *columnStyle = d->textSharedData->tableColumnStyle(columnStyleName, d->stylesDotXml);
        if (columnStyle) {
            for (int c = columns; c < columns + repeatColumn; c++) {
                tcarManager.setColumnStyle(c, *columnStyle);
            }
        }
    }

    QString defaultCellStyleName = tblTag.attributeNS(KoXmlNS::table, "default-cell-style-name", "");
    if (!defaultCellStyleName.isEmpty()) {
        KoTableCellStyle *cellStyle = d->textSharedData->tableCellStyle(defaultCellStyleName, d->stylesDotXml);
        for (int c = columns; c < columns + repeatColumn; c++) {
            tcarManager.setDefaultColumnCellStyle(c, cellStyle);
        }
    }

    columns = columns + repeatColumn;
    if (rows > 0)
        tbl->resize(rows, columns);
    else
        tbl->resize(1, columns);
}

void KoTextLoader::loadTableRow(KoXmlElement &tblTag, QTextTable *tbl, QList<QRect> &spanStore, QTextCursor &cursor, int &rows)
{
    KoTableColumnAndRowStyleManager tcarManager = KoTableColumnAndRowStyleManager::getManager(tbl);

    int columns = tbl->columns();
    QString rowStyleName = tblTag.attributeNS(KoXmlNS::table, "style-name", "");
    if (!rowStyleName.isEmpty()) {
        KoTableRowStyle *rowStyle = d->textSharedData->tableRowStyle(rowStyleName, d->stylesDotXml);
        if (rowStyle) {
            tcarManager.setRowStyle(rows, *rowStyle);
        }
    }

    QString defaultCellStyleName = tblTag.attributeNS(KoXmlNS::table, "default-cell-style-name", "");
    if (!defaultCellStyleName.isEmpty()) {
        KoTableCellStyle *cellStyle = d->textSharedData->tableCellStyle(defaultCellStyleName, d->stylesDotXml);
        tcarManager.setDefaultRowCellStyle(rows, cellStyle);
    }

    rows++;
    if (columns > 0)
        tbl->resize(rows, columns);
    else
        tbl->resize(rows, 1);

    // Added a row
    int currentCell = 0;
    KoXmlElement rowTag;
    forEachElement(rowTag, tblTag) {
        if (!rowTag.isNull()) {
            const QString rowLocalName = rowTag.localName();
            if (rowTag.namespaceURI() == KoXmlNS::table) {
                if (rowLocalName == "table-cell") {
                    loadTableCell(rowTag, tbl, spanStore, cursor, currentCell);
                    currentCell++;
                } else if (rowLocalName == "covered-table-cell") {
                    currentCell++;
                }
            } else if (rowTag.namespaceURI() == KoXmlNS::delta) {
                if (rowLocalName == "removed-content")
                    d->openChangeRegion(rowTag);

                KoXmlElement deltaRowTag;
                forEachElement (deltaRowTag, rowTag) {
                    if (!deltaRowTag.isNull() && (deltaRowTag.namespaceURI() == KoXmlNS::table)) {
                        const QString deltaRowLocalName = deltaRowTag.localName();
                        if (deltaRowLocalName == "table-cell") {
                            loadTableCell(deltaRowTag, tbl, spanStore, cursor, currentCell);
                            currentCell++;
                        } else if (deltaRowLocalName == "covered-table-cell") {
                            currentCell++;
                        }
                    }
                }

                if (rowLocalName == "removed-content")
                    d->closeChangeRegion(rowTag);
            }
        }
    }
}

void KoTextLoader::loadTableCell(KoXmlElement &rowTag, QTextTable *tbl, QList<QRect> &spanStore, QTextCursor &cursor, int &currentCell)
{
    KoTableColumnAndRowStyleManager tcarManager = KoTableColumnAndRowStyleManager::getManager(tbl);
    const int currentRow = tbl->rows() - 1;
    QTextTableCell cell = tbl->cellAt(currentRow, currentCell);

    if (!rowTag.attributeNS(KoXmlNS::delta, "insertion-type").isEmpty())
        d->openChangeRegion(rowTag);

    // store spans until entire table have been loaded
    int rowsSpanned = rowTag.attributeNS(KoXmlNS::table, "number-rows-spanned", "1").toInt();
    int columnsSpanned = rowTag.attributeNS(KoXmlNS::table, "number-columns-spanned", "1").toInt();
    spanStore.append(QRect(currentCell, currentRow, columnsSpanned, rowsSpanned));

    if (cell.isValid()) {
        QString cellStyleName = rowTag.attributeNS(KoXmlNS::table, "style-name", "");
        KoTableCellStyle *cellStyle = 0;
        if (!cellStyleName.isEmpty()) {
            cellStyle = d->textSharedData->tableCellStyle(cellStyleName, d->stylesDotXml);
        } else if (tcarManager.defaultRowCellStyle(currentRow)) {
            cellStyle = tcarManager.defaultRowCellStyle(currentRow);
        } else if (tcarManager.defaultColumnCellStyle(currentCell)) {
            cellStyle = tcarManager.defaultColumnCellStyle(currentCell);
        }

        if (cellStyle)
            cellStyle->applyStyle(cell);

        QTextTableCellFormat cellFormat = cell.format().toTableCellFormat();

        if (rowTag.attributeNS(KoXmlNS::table, "protected", "false") == "true") {
            cellFormat.setProperty(KoTableCellStyle::CellIsProtected, true);
        }
        if (d->changeTracker && d->changeStack.count()) {
            cellFormat.setProperty(KoCharacterStyle::ChangeTrackerId, d->changeStack.top());
        }

        cell.setFormat(cellFormat);

        // handle inline Rdf
        // rowTag is the current table cell.
        KoElementReference id;
        id.loadOdf(rowTag);

        if (rowTag.hasAttributeNS(KoXmlNS::xhtml, "property") || d->rdfIdList.contains(id.toString())) {
            KoTextInlineRdf* inlineRdf = new KoTextInlineRdf((QTextDocument*)cursor.block().document(),cell);
            if (inlineRdf->loadOdf(rowTag)) {
                QTextTableCellFormat cellFormat = cell.format().toTableCellFormat();
                cellFormat.setProperty(KoTableCellStyle::InlineRdf,QVariant::fromValue(inlineRdf));
                cell.setFormat(cellFormat);
            }
            else {
                delete inlineRdf;
                inlineRdf = 0;
            }
        }

        cursor = cell.firstCursorPosition();
        loadBody(rowTag, cursor);
    }

    if (!rowTag.attributeNS(KoXmlNS::delta, "insertion-type").isEmpty())
        d->closeChangeRegion(rowTag);
}

void KoTextLoader::loadShapeWithHyperLink(const KoXmlElement &element, QTextCursor& cursor)
{
    // get the hyperlink
    QString hyperLink = element.attributeNS(KoXmlNS::xlink, "href");
    KoShape *shape = 0;

    //load the shape for hyperlink
    KoXmlNode node = element.firstChild();
    if (!node.isNull()) {
        KoXmlElement ts = node.toElement();
        shape = loadShape(ts, cursor);
        if (shape) {
            shape->setHyperLink(hyperLink);
        }
    }
}

KoShape *KoTextLoader::loadShape(const KoXmlElement &element, QTextCursor &cursor)
{
    KoShape *shape = KoShapeRegistry::instance()->createShapeFromOdf(element, d->context);
    if (!shape) {
        kDebug(32500) << "shape '" << element.localName() << "' unhandled";
        return 0;
    }

    KoTextAnchor *anchor = new KoTextAnchor(shape);
    anchor->loadOdf(element, d->context);
    d->textSharedData->shapeInserted(shape, element, d->context, anchor);

    // page anchored shapes are handled differently
    if (anchor->anchorType() == KoTextAnchor::AnchorPage && shape->hasAdditionalAttribute("text:anchor-page-number")) {
        // nothing else to do
    } else {
        shape->setVisible(false); // make it invisible until layouting

        KoInlineTextObjectManager *textObjectManager = KoTextDocument(cursor.block().document()).inlineTextObjectManager();
        if (textObjectManager) {
            if (!element.attributeNS(KoXmlNS::delta, "insertion-type").isEmpty())
                d->openChangeRegion(element);

            if (d->changeTracker && d->changeStack.count()) {
                QTextCharFormat format;
                format.setProperty(KoCharacterStyle::ChangeTrackerId, d->changeStack.top());
                cursor.mergeCharFormat(format);
            } else {
                QTextCharFormat format = cursor.charFormat();
                if (format.hasProperty(KoCharacterStyle::ChangeTrackerId)) {
                    format.clearProperty(KoCharacterStyle::ChangeTrackerId);
                    cursor.setCharFormat(format);
                }
            }

            textObjectManager->insertInlineObject(cursor, anchor);

            if (!element.attributeNS(KoXmlNS::delta, "insertion-type").isEmpty())
                d->closeChangeRegion(element);
        }
    }
    return shape;
}


void KoTextLoader::loadTableOfContents(const KoXmlElement &element, QTextCursor &cursor)
{
    // make sure that the tag is table-of-content
    Q_ASSERT(element.tagName() == "table-of-content");
    QTextBlockFormat tocFormat;

    // for "meta-information" about the TOC we use this class
    KoTableOfContentsGeneratorInfo *info = new KoTableOfContentsGeneratorInfo();

    // to store the contents we use an extrafor "meta-information" about the TOC we use this class
    QTextDocument *tocDocument = new QTextDocument();
    KoTextDocument(tocDocument).setStyleManager(d->styleManager);

    info->m_name = element.attribute("name");
    info->m_styleName = element.attribute("style-name");

    KoXmlElement e;
    forEachElement(e, element) {
        if (e.isNull() || e.namespaceURI() != KoXmlNS::text) {
            continue;
        }

        if (e.localName() == "table-of-content-source" && e.namespaceURI() == KoXmlNS::text) {
            info->loadOdf(d->textSharedData, e);
            // uncomment to see what has been loaded
            //info.tableOfContentData()->dump();
            tocFormat.setProperty(KoParagraphStyle::TableOfContentsData, QVariant::fromValue<KoTableOfContentsGeneratorInfo*>(info) );
            tocFormat.setProperty(KoParagraphStyle::GeneratedDocument, QVariant::fromValue<QTextDocument*>(tocDocument) );
            cursor.insertBlock(tocFormat);

            // We'll just try to find displayable elements and add them as paragraphs
        } else if (e.localName() == "index-body") {
            QTextCursor cursorFrame = tocDocument->rootFrame()->lastCursorPosition();

            bool firstTime = true;
            KoXmlElement p;
            forEachElement(p, e) {
                // All elem will be "p" instead of the title, which is particular
                if (p.isNull() || p.namespaceURI() != KoXmlNS::text)
                    continue;

                if (!firstTime) {
                    // use empty formats to not inherit from the prev parag
                    QTextBlockFormat bf;
                    QTextCharFormat cf;
                    cursorFrame.insertBlock(bf, cf);
                }
                firstTime = false;

                QTextBlock current = cursorFrame.block();
                QTextBlockFormat blockFormat;

                if (p.localName() == "p") {
                    loadParagraph(p, cursorFrame);
                } else if (p.localName() == "index-title") {
                    loadBody(p, cursorFrame);
                }

                QTextCursor c(current);
                c.mergeBlockFormat(blockFormat);
            }

        }// index-body
    }
    // Get out of the frame
    cursor.movePosition(QTextCursor::Right);
}

void KoTextLoader::loadBibliography(const KoXmlElement &element, QTextCursor &cursor)
{
    // make sure that the tag is bibliography
    Q_ASSERT(element.tagName() == "bibliography");
    QTextBlockFormat bibFormat;

    // for "meta-information" about the bibliography we use this class
    KoBibliographyInfo *info = new KoBibliographyInfo();

    QTextDocument *bibDocument = new QTextDocument();
    KoTextDocument(bibDocument).setStyleManager(d->styleManager);

    info->m_name = element.attribute("name");
    info->m_styleName = element.attribute("style-name");

    KoXmlElement e;
    forEachElement(e, element) {
        if (e.isNull() || e.namespaceURI() != KoXmlNS::text) {
            continue;
        }

        if (e.localName() == "bibliography-source" && e.namespaceURI() == KoXmlNS::text) {
            info->loadOdf(d->textSharedData, e);

            bibFormat.setProperty(KoParagraphStyle::BibliographyData, QVariant::fromValue<KoBibliographyInfo*>(info));
            bibFormat.setProperty(KoParagraphStyle::GeneratedDocument, QVariant::fromValue<QTextDocument*>(bibDocument));

            cursor.insertBlock(bibFormat);
            // We'll just try to find displayable elements and add them as paragraphs
        } else if (e.localName() == "index-body") {
            QTextCursor cursorFrame = bibDocument->rootFrame()->lastCursorPosition();

            bool firstTime = true;
            KoXmlElement p;
            forEachElement(p, e) {
                // All elem will be "p" instead of the title, which is particular
                if (p.isNull() || p.namespaceURI() != KoXmlNS::text)
                    continue;

                if (!firstTime) {
                    // use empty formats to not inherit from the prev parag
                    QTextBlockFormat bf;
                    QTextCharFormat cf;
                    cursorFrame.insertBlock(bf, cf);
                }
                firstTime = false;

                QTextBlock current = cursorFrame.block();
                QTextBlockFormat blockFormat;

                if (p.localName() == "p") {
                    loadParagraph(p, cursorFrame);
                } else if (p.localName() == "index-title") {
                    loadBody(p, cursorFrame);
                }

                QTextCursor c(current);
                c.mergeBlockFormat(blockFormat);
            }

        }// index-body
    }
    // Get out of the frame
    cursor.movePosition(QTextCursor::Right);
}

void KoTextLoader::startBody(int total)
{
    d->bodyProgressTotal += total;
}

void KoTextLoader::processBody()
{
    d->bodyProgressValue++;
    if (d->progressTime.elapsed() >= d->nextProgressReportMs) {  // update based on elapsed time, don't saturate the queue
        d->nextProgressReportMs = d->progressTime.elapsed() + 333; // report 3 times per second
        Q_ASSERT(d->bodyProgressTotal > 0);
        const int percent = d->bodyProgressValue * 100 / d->bodyProgressTotal;
        emit sigProgress(percent);
    }
}

void KoTextLoader::endBody()
{
}

void KoTextLoader::storeDeleteChanges(KoXmlElement &element)
{
    KoXmlElement tag;
    forEachElement(tag, element) {
        if (! tag.isNull()) {
            const QString localName = tag.localName();
            if (localName == "changed-region") {
                KoXmlElement region;
                forEachElement(region, tag) {
                    if (!region.isNull()) {
                        if (region.localName() == "deletion") {
                            QString id = tag.attributeNS(KoXmlNS::text, "id");
                            d->deleteChangeTable.insert(id, region);
                        }
                    }
                }
            }
        }
    }
}

void KoTextLoader::markBlocksAsInserted(QTextCursor& cursor,int from, const QString& id)
{
    int to = cursor.position();
    QTextCursor editCursor(cursor);
    QTextDocument *document = cursor.document();

    QTextBlock startBlock = document->findBlock(from);
    QTextBlock endBlock = document->findBlock(to);

    int changeId = d->changeTracker->getLoadedChangeId(id);

    QTextBlockFormat format;
    format.setProperty(KoCharacterStyle::ChangeTrackerId, changeId);

    do {
        startBlock = startBlock.next();
        editCursor.setPosition(startBlock.position());
        editCursor.mergeBlockFormat(format);
    } while(startBlock != endBlock);
}

#include <KoTextLoader.moc>
