/* This file is part of the KDE project
 * Copyright (C) 2004-2006 David Faure <faure@kde.org>
 * Copyright (C) 2007,2009 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007 Sebastian Sauer <mail@dipe.org>
 * Copyright (C) 2007 Pierre Ducroquet <pinaraf@gmail.com>
 * Copyright (C) 2007-2009 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2008 Girish Ramakrishnan <girish@forwardbias.in>
 * Copyright (C) 2009 KO GmbH <cbo@kogmbh.com>
 * Copyright (C) 2009 Pierre Stirnweiss <pstirnweiss@googlemail.com>
   Copyright (C) 2010 KO GmbH <ben.martin@kogmbh.com>
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
#include <KoTextDocumentLayout.h>
#include <KoTextShapeData.h>
#include "KoTextSharedLoadingData.h"
#include <KoUnit.h>
#include <KoVariable.h>
#include <KoVariableManager.h>
#include <KoInlineObjectRegistry.h>
#include <KoXmlNS.h>
#include <KoXmlReader.h>
#include "KoTextInlineRdf.h"

#include "changetracker/KoChangeTracker.h"
#include "changetracker/KoChangeTrackerElement.h"
#include "changetracker/KoDeleteChangeMarker.h"
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

#include <rdf/KoDocumentRdfBase.h>
#ifdef SHOULD_BUILD_RDF
#include <Soprano/Soprano>
#endif

#include <kdebug.h>

#include <QList>
#include <QMap>
#include <QRect>
#include <QStack>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextList>
#include <QTextTable>
#include <QTime>
#include <QString>


#include "KoTextLoader_p.h"
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

    int bodyProgressTotal;
    int bodyProgressValue;
    int nextProgressReportMs;
    QTime dt;

    KoList *currentList;
    KoListStyle *currentListStyle;
    int currentListLevel;
    // Two lists that follow the same style are considered as one for numbering purposes
    // This hash keeps all the lists that have the same style in one KoList.
    QHash<KoListStyle *, KoList *> lists;

    KoStyleManager *styleManager;

    KoChangeTracker *changeTracker;

    KoDocumentRdfBase *rdfData;

    int loadSpanLevel;
    int loadSpanInitialPos;
    QStack<int> changeStack;
    QMap<QString, int> changeTransTable;
    QMap<QString, KoXmlElement> deleteChangeTable;

    explicit Private(KoShapeLoadingContext &context)
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
            currentList(0),
            currentListStyle(0),
            currentListLevel(1),
            styleManager(0),
            changeTracker(0),
            loadSpanLevel(0),
            loadSpanInitialPos(0) {
        dt.start();
    }

    ~Private() {
        kDebug(32500) << "Loading took" << (float)(dt.elapsed()) / 1000 << " seconds";
    }

    KoList *list(const QTextDocument *document, KoListStyle *listStyle);

    void openChangeRegion(const KoXmlElement &element);
    void closeChangeRegion(const KoXmlElement &element);
    void splitStack(int id);
};

bool KoTextLoader::containsRichText(const KoXmlElement &element)
{
    KoXmlElement textParagraphElement;
    forEachElement(textParagraphElement, element) {

        if (textParagraphElement.localName() != "p" ||
            textParagraphElement.namespaceURI() != KoXmlNS::text)
            return true;

        // if any of this nodes children are elements, we're dealing with richtext (exceptions: text:s (space character) and text:tab (tab character)
        for (KoXmlNode n = textParagraphElement.firstChild(); !n.isNull(); n = n.nextSibling()) {
            const KoXmlElement e = n.toElement();
            if (!e.isNull() && (e.namespaceURI() != KoXmlNS::text
                || (e.localName() != "s" // space
                && e.localName() != "annotation"
                && e.localName() != "bookmark"
                && e.localName() != "line-break"
                && e.localName() != "meta"
                && e.localName() != "tab" //\\t
                && e.localName() != "tag")))
                return true;
        }
    }
    return false;
}

void KoTextLoader::Private::splitStack(int id)
{
    if (changeStack.isEmpty())
        return;

    int oldId = changeStack.top();
    changeStack.pop();
    if (id == oldId)
        return;
    int newId = changeTracker->split(oldId);
    splitStack(id);
    changeTracker->setParent(newId, changeStack.top());
    changeStack.push(newId);
}

void KoTextLoader::Private::openChangeRegion(const KoXmlElement& element)
{
    QString id = element.attributeNS(KoXmlNS::text, "change-id");
    int changeId = changeTracker->getLoadedChangeId(id);
    if (!changeId)
        return;
    if (!changeStack.empty())
        changeTracker->setParent(changeId, changeStack.top());
    changeStack.push(changeId);
    changeTransTable.insert(id, changeId);

    KoChangeTrackerElement *changeElement = changeTracker->elementById(changeId);
    changeElement->setEnabled(true);
}

void KoTextLoader::Private::closeChangeRegion(const KoXmlElement& element)
{
    QString id = element.attributeNS(KoXmlNS::text, "change-id");
    int changeId = changeTracker->getLoadedChangeId(id);

    splitStack(changeId);
}

KoList *KoTextLoader::Private::list(const QTextDocument *document, KoListStyle *listStyle)
{
    if (lists.contains(listStyle))
        return lists[listStyle];
    KoList *newList = new KoList(document, listStyle);
    lists[listStyle] = newList;
    return newList;
}

/////////////KoTextLoader

KoTextLoader::KoTextLoader(KoShapeLoadingContext &context, KoDocumentRdfBase *rdfData)
        : QObject()
        , d(new Private(context))
{
    d->rdfData = rdfData;
    KoSharedLoadingData *sharedData = context.sharedData(KOTEXT_SHARED_LOADING_ID);
    if (sharedData) {
        d->textSharedData = dynamic_cast<KoTextSharedLoadingData *>(sharedData);
    }

    kDebug(32500) << "sharedData" << sharedData << "textSharedData" << d->textSharedData;

    if (!d->textSharedData) {
        d->textSharedData = new KoTextSharedLoadingData();
        // TODO pass style manager so that on copy and paste we can recognice the same styles
        d->textSharedData->loadOdfStyles(context, 0);
        if (!sharedData) {
            context.addSharedData(KOTEXT_SHARED_LOADING_ID, d->textSharedData);
        } else {
            kWarning(32500) << "A different type of sharedData was found under the" << KOTEXT_SHARED_LOADING_ID;
            Q_ASSERT(false);
        }
    }
}

KoTextLoader::~KoTextLoader()
{
    delete d;
}

void KoTextLoader::loadBody(const KoXmlElement &bodyElem, QTextCursor &cursor, bool isDeleteChange)
{
    cursor.beginEditBlock();
    const QTextBlockFormat defaultBlockFormat = cursor.blockFormat();
    const QTextCharFormat defaultCharFormat = cursor.charFormat();
    const QTextDocument *document = cursor.block().document();

    d->styleManager = KoTextDocument(document).styleManager();
    d->changeTracker = KoTextDocument(document).changeTracker();
//    if (!d->changeTracker)
//        d->changeTracker = dynamic_cast<KoChangeTracker *>(d->context.dataCenterMap().value("ChangeTracker"));
//    Q_ASSERT(d->changeTracker);

    kDebug(32500) << "text-style:" << KoTextDebug::textAttributes(cursor.blockCharFormat());
#if 0
    if ((document->isEmpty()) && (d->styleManager)) {
        QTextBlock block = cursor.block();
        d->styleManager->defaultParagraphStyle()->applyStyle(block);
    }
#endif
    bool usedParagraph = false; // set to true if we found a tag that used the paragraph, indicating that the next round needs to start a new one.
    if (bodyElem.namespaceURI() == KoXmlNS::table && bodyElem.localName() == "table") {
        loadTable(bodyElem, cursor);
    }
    else {
        startBody(KoXml::childNodesCount(bodyElem));
        KoXmlElement tag;

        forEachElement(tag, bodyElem) {
            if (! tag.isNull()) {
                const QString localName = tag.localName();

                if (tag.namespaceURI() == KoXmlNS::text) {
                    if (usedParagraph)
                        cursor.insertBlock(defaultBlockFormat, defaultCharFormat);
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
                            if (d->changeStack.count())
                                d->changeTracker->setParent(changeId, d->changeStack.top());
                            KoDeleteChangeMarker *deleteChangemarker = new KoDeleteChangeMarker(d->changeTracker);
                            deleteChangemarker->setChangeId(changeId);
                            KoChangeTrackerElement *changeElement = d->changeTracker->elementById(changeId);
                            changeElement->setDeleteChangeMarker(deleteChangemarker);
                            changeElement->setEnabled(true);
                            KoTextDocumentLayout *layout = qobject_cast<KoTextDocumentLayout*>(cursor.block().document()->documentLayout());
                            if (layout) {
                                KoInlineTextObjectManager *textObjectManager = layout->inlineTextObjectManager();
                                textObjectManager->insertInlineObject(cursor, deleteChangemarker);
                            }
                        }

                        loadDeleteChangeOutsidePorH(id, cursor);
                        usedParagraph = false;
                    } else if (localName == "p") {    // text paragraph
                        loadParagraph(tag, cursor);
                    } else if (localName == "h") {  // heading
                        loadHeading(tag, cursor);
                    } else if (localName == "unordered-list" || localName == "ordered-list" // OOo-1.1
                            || localName == "list" || localName == "numbered-paragraph") {  // OASIS
                        loadList(tag, cursor, isDeleteChange);
                    } else if (localName == "section") {  // Temporary support (TODO)
                        loadSection(tag, cursor);
                    } else if (localName == "table-of-content") {
                        loadTableOfContents(tag, cursor);
                    } else {
                        KoInlineObject *obj = KoInlineObjectRegistry::instance()->createFromOdf(tag, d->context);
                        if (obj) {
                            KoTextDocumentLayout *layout = qobject_cast<KoTextDocumentLayout*>(cursor.block().document()->documentLayout());
                            if (layout) {
                                KoInlineTextObjectManager *textObjectManager = layout->inlineTextObjectManager();
                                if (textObjectManager) {
                                    KoVariableManager *varManager = textObjectManager->variableManager();
                                    if (varManager) {
                                        textObjectManager->insertInlineObject(cursor, obj);
                                    }
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
                        loadTable(tag, cursor);
                    } else {
                        kWarning(32500) << "KoTextLoader::loadBody unhandled table::" << localName;
                    }
                }
            }
            processBody();
        }
        endBody();
    }
    cursor.endEditBlock();
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
    QString styleName = element.attributeNS(KoXmlNS::text, "style-name", QString());

    KoParagraphStyle *paragraphStyle = d->textSharedData->paragraphStyle(styleName, d->stylesDotXml);

    Q_ASSERT(d->styleManager);
    if (!paragraphStyle) {
        // Either the paragraph has no style or the style-name could not be found.
        // Fix up the paragraphStyle to be our default paragraph style in either case.
        if (!styleName.isEmpty())
            kWarning(32500) << "paragraph style " << styleName << "not found - using default style";
        paragraphStyle = d->styleManager->defaultParagraphStyle();
        kWarning(32500) << "defaultParagraphStyle not found - using default style";
    }

    QTextCharFormat cf = cursor.charFormat(); // store the current cursor char format

    if (paragraphStyle) {
        QTextBlock block = cursor.block();
        // Apply list style when loading a list but we don't have a list style
        paragraphStyle->applyStyle(block, d->currentList && !d->currentListStyle);
        // Clear the outline level property. If a default-outline-level was set, it should not
        // be applied when loading a document, only on user action.
        block.blockFormat().clearProperty(KoParagraphStyle::OutlineLevel);
    } else {
        kWarning(32500) << "paragraph style " << styleName << " not found";
    }

    // Some paragraph have id's defined which we need to store so that we can eg
    // attach text animations to this specific paragraph later on
    if (element.hasAttributeNS(KoXmlNS::text, "id")) {
        QTextBlock block = cursor.block();
        KoTextBlockData *data = dynamic_cast<KoTextBlockData*>(block.userData());
        if (!data) {
            data = new KoTextBlockData();
            block.setUserData(data);
        }
        d->context.addShapeSubItemId(0, qVariantFromValue(data), element.attributeNS(KoXmlNS::text, "id"));
    }

    // attach Rdf to cursor.block()
    // remember inline Rdf metadata
    if (element.hasAttributeNS(KoXmlNS::xhtml, "property")
            || element.hasAttribute("id")) {
        QTextBlock block = cursor.block();
        KoTextInlineRdf* inlineRdf =
            new KoTextInlineRdf((QTextDocument*)block.document(), block);
        inlineRdf->loadOdf(element);
        KoTextInlineRdf::attach(inlineRdf, cursor);
    }
    kDebug(32500) << "text-style:" << KoTextDebug::textAttributes(cursor.blockCharFormat()) << d->currentList << d->currentListStyle;

    bool stripLeadingSpace = true;
    loadSpan(element, cursor, &stripLeadingSpace);
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
        paragraphStyle->applyStyle(block, d->currentList && !d->currentListStyle);
    } else {
        kWarning(32500) << "paragraph style " << styleName << " not found";
    }

    if ((block.blockFormat().hasProperty(KoParagraphStyle::OutlineLevel)) && (level == -1)) {
        level = block.blockFormat().property(KoParagraphStyle::OutlineLevel).toInt();
    } else {
        if (level == -1)
            level = 1;
        QTextBlockFormat blockFormat;
        blockFormat.setProperty(KoParagraphStyle::OutlineLevel, level);
        cursor.mergeBlockFormat(blockFormat);
    }

    if (!d->currentList) { // apply <text:outline-style> (if present) only if heading is not within a <text:list>
        KoListStyle *outlineStyle = d->styleManager->outlineStyle();
        if (outlineStyle) {
            KoList *list = d->list(block.document(), outlineStyle);
            list->applyStyle(block, outlineStyle, level);
        }
    }

    // attach Rdf to cursor.block()
    // remember inline Rdf metadata
    if (element.hasAttributeNS(KoXmlNS::xhtml, "property")
            || element.hasAttribute("id")) {
        QTextBlock block = cursor.block();
        KoTextInlineRdf* inlineRdf =
            new KoTextInlineRdf((QTextDocument*)block.document(), block);
        inlineRdf->loadOdf(element);
        KoTextInlineRdf::attach(inlineRdf, cursor);
    }

    kDebug(32500) << "text-style:" << KoTextDebug::textAttributes(cursor.blockCharFormat());

    QTextCharFormat cf = cursor.charFormat(); // store the current cursor char format

    bool stripLeadingSpace = true;
    loadSpan(element, cursor, &stripLeadingSpace);
    cursor.setCharFormat(cf);   // restore the cursor char format
}

void KoTextLoader::loadList(const KoXmlElement &element, QTextCursor &cursor, bool isDeleteChange)
{
    const bool numberedParagraph = element.localName() == "numbered-paragraph";
    const QTextBlockFormat defaultBlockFormat = cursor.blockFormat();
    const QTextCharFormat defaultCharFormat = cursor.charFormat();

    QString styleName = element.attributeNS(KoXmlNS::text, "style-name", QString());
    KoListStyle *listStyle = d->textSharedData->listStyle(styleName, d->stylesDotXml);

    int level = 1;
    /********************************ODF Bug Work-Around Code That Uses RDF**********************/
    bool listValid = false, levelIncreased = false;
    int deletedListLevel = 0;
    if (element.hasAttribute("id")) {
        QString xmlId = element.attribute("id", QString());
        listValid = isValidList(xmlId);
        deletedListLevel = listLevel(xmlId);
    }
    /********************************************************************************************/

    // TODO: get level from the style, if it has a style:list-level attribute (new in ODF-1.2)
    if (numberedParagraph) {
        if (!d->currentList)
            d->currentList = d->list(cursor.block().document(), listStyle);
        d->currentListStyle = listStyle;
        level = element.attributeNS(KoXmlNS::text, "level", "1").toInt();
    }

    if (!numberedParagraph && (!isDeleteChange || listValid)) {
        if (!listStyle)
            listStyle = d->currentListStyle;
        if (!d->currentList)
            d->currentList = d->list(cursor.block().document(), listStyle);
        level = d->currentListLevel++;
        d->currentListStyle = listStyle;
    }

    /************************************ODF Bug Work-Around Code that uses RDF****************************************/
    if (!numberedParagraph && isDeleteChange && !d->currentList) {
        if (!listStyle)
            listStyle = d->currentListStyle;
        d->currentList = d->list(cursor.block().document(), listStyle);
        level = d->currentListLevel++;
        levelIncreased = true;
        d->currentListStyle = listStyle;
    }

    if (!numberedParagraph && isDeleteChange && deletedListLevel && (deletedListLevel == d->currentListLevel)) {
        level = d->currentListLevel++;
        levelIncreased = true;
    }
    /********************************************************************************************************************/

    if (element.hasAttributeNS(KoXmlNS::text, "continue-numbering")) {
        const QString continueNumbering = element.attributeNS(KoXmlNS::text, "continue-numbering", QString());
        d->currentList->setContinueNumbering(level, continueNumbering == "true");
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

    // Iterate over list items and add them to the textlist
    KoXmlElement e;
    bool firstTime = true;
    forEachElement(e, element) {
        if (e.isNull() || e.namespaceURI() != KoXmlNS::text)
            continue;

        const bool listHeader = e.tagName() == "list-header";

        if (!numberedParagraph && e.tagName() != "list-item" && !listHeader)
            continue;

        bool listItemValid = false;
        if (e.hasAttribute("id")) {
            QString xmlId = e.attribute("id", QString());
            listItemValid = isValidListItem(xmlId);
        }

        if (!firstTime && !numberedParagraph)
            cursor.insertBlock(defaultBlockFormat, defaultCharFormat);
        firstTime = false;

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
            loadBody(e, cursor, isDeleteChange);
        }

        if (!current.textList())
            d->currentList->add(current, level);

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
            d->currentList->add(c.block(), level);
        }
        kDebug(32500) << "text-style:" << KoTextDebug::textAttributes(cursor.blockCharFormat());
    }

    /*******************************ODF Bug Work-Around Code Changes***********************************/
    if (!isDeleteChange || (isDeleteChange && (listValid || levelIncreased)))
        d->currentListLevel--;

    if ((!isDeleteChange && (numberedParagraph || d->currentListLevel == 1)) ||
        (isDeleteChange && listValid && (numberedParagraph || d->currentListLevel == 1))) {
        d->currentListStyle = 0;
        d->currentList = 0;
    }
    /***************************************************************************************************/
}

/*************************************ODF Bug Work-Around Code*******************************************/
bool KoTextLoader::isValidList(const QString& xmlId) const
{
    #ifdef SHOULD_BUILD_RDF
    Soprano::Model *model = d->rdfData->model();
    Soprano::Node wildCardNode;

    // Find the Subject with this xmlId
    Soprano::Node xmlIdNode = Soprano::Node::createLiteralNode(Soprano::LiteralValue(xmlId));
    Soprano::StatementIterator stmtIt = model->listStatements(wildCardNode, wildCardNode, xmlIdNode, wildCardNode);

    //Store the subject Node
    QList<Soprano::Statement> allStatements = stmtIt.allElements();
    if (!allStatements.size())
        return true;
    Soprano::Node elementNode = allStatements.at(0).subject();

    //Find the Validity of the found subjectNode
    Soprano::Node listValidity = Soprano::Node::createResourceNode(QUrl(KoDeleteChangeMarker::RDFListValidity));
    stmtIt = model->listStatements(elementNode, listValidity, wildCardNode, wildCardNode);
    allStatements = stmtIt.allElements();

    if(!allStatements.size())
        return true;

    return allStatements.at(0).object().literal().toBool();
    #else
    return true;
    #endif
}

/*************************************ODF Bug Work-Around Code*******************************************/
bool KoTextLoader::isValidListItem(const QString& xmlId) const
{
    #ifdef SHOULD_BUILD_RDF
    Soprano::Model *model = d->rdfData->model();
    Soprano::Node wildCardNode;

    // Find the Subject with this xmlId
    Soprano::Node xmlIdNode = Soprano::Node::createLiteralNode(Soprano::LiteralValue(xmlId));
    Soprano::StatementIterator stmtIt = model->listStatements(wildCardNode, wildCardNode, xmlIdNode, wildCardNode);

    //Store the subject Node
    QList<Soprano::Statement> allStatements = stmtIt.allElements();
    if (!allStatements.size())
        return true;
    Soprano::Node elementNode = allStatements.at(0).subject();

    //Find the Validity of the found subjectNode
    Soprano::Node listValidity = Soprano::Node::createResourceNode(QUrl(KoDeleteChangeMarker::RDFListItemValidity));
    stmtIt = model->listStatements(elementNode, listValidity, wildCardNode, wildCardNode);
    allStatements = stmtIt.allElements();

    if(!allStatements.size())
        return true;

    return allStatements.at(0).object().literal().toBool();
    #else
    return true;
    #endif
}

/*************************************ODF Bug Work-Around Code*******************************************/
int KoTextLoader::listLevel(const QString& xmlId) const
{
    #ifdef SHOULD_BUILD_RDF
    Soprano::Model *model = d->rdfData->model();
    Soprano::Node wildCardNode;

    // Find the Subject with this xmlId
    Soprano::Node xmlIdNode = Soprano::Node::createLiteralNode(Soprano::LiteralValue(xmlId));
    Soprano::StatementIterator stmtIt = model->listStatements(wildCardNode, wildCardNode, xmlIdNode, wildCardNode);

    //Store the subject Node
    QList<Soprano::Statement> allStatements = stmtIt.allElements();
    if (!allStatements.size())
        return true;
    Soprano::Node elementNode = allStatements.at(0).subject();

    //Find the Validity of the found subjectNode
    Soprano::Node listLevel = Soprano::Node::createResourceNode(QUrl(KoDeleteChangeMarker::RDFListLevel));
    stmtIt = model->listStatements(elementNode, listLevel, wildCardNode, wildCardNode);
    allStatements = stmtIt.allElements();

    if(!allStatements.size())
        return true;

    return allStatements.at(0).object().literal().toInt();
    #else
    return 0;
    #endif
}

void KoTextLoader::loadSection(const KoXmlElement &sectionElem, QTextCursor &cursor)
{
    // Add a frame to the current layout
    QTextFrameFormat sectionFormat;
    QString sectionStyleName = sectionElem.attributeNS(KoXmlNS::text, "style-name", "");
    if (!sectionStyleName.isEmpty()) {
        KoSectionStyle *secStyle = d->textSharedData->sectionStyle(sectionStyleName, d->stylesDotXml);
        if (secStyle)
            secStyle->applyStyle(sectionFormat);
    }
    cursor.insertFrame(sectionFormat);
    // Get the cursor of the frame
    QTextCursor cursorFrame = cursor.currentFrame()->lastCursorPosition();

    loadBody(sectionElem, cursorFrame);

    // Get out of the frame
    cursor.movePosition(QTextCursor::End);
}

void KoTextLoader::loadNote(const KoXmlElement &noteElem, QTextCursor &cursor)
{
    kDebug(32500) << "Loading a text:note element.";
    KoTextDocumentLayout *layout = qobject_cast<KoTextDocumentLayout*>(cursor.block().document()->documentLayout());
    if (layout) {
        KoInlineNote *note = new KoInlineNote(KoInlineNote::Footnote);
        if (note->loadOdf(noteElem, d->context, d->styleManager, d->changeTracker)) {
            KoInlineTextObjectManager *textObjectManager = layout->inlineTextObjectManager();
            textObjectManager->insertInlineObject(cursor, note);
        } else {
            kDebug(32500) << "Error while loading the note !";
            delete note;
        }
    }
}

QString KoTextLoader::createUniqueBookmarkName(KoBookmarkManager* bmm, QString bookmarkName, bool isEndMarker)
{
    QString ret = bookmarkName;
    int uniqID = 0;

    while (true) {
        if (bmm->retrieveBookmark(ret)) {
            ret = QString("%1_%2").arg(bookmarkName).arg(++uniqID);
        } else {
            if (isEndMarker) {
                --uniqID;
                if (!uniqID)
                    ret = bookmarkName;
                else
                    ret = QString("%1_%2").arg(bookmarkName).arg(uniqID);
            }
            break;
        }
    }
    return ret;
}

void KoTextLoader::loadSpan(const KoXmlElement &element, QTextCursor &cursor, bool *stripLeadingSpace)
{
    kDebug(32500) << "text-style:" << KoTextDebug::textAttributes(cursor.blockCharFormat());
    Q_ASSERT(stripLeadingSpace);
    if (d->loadSpanLevel++ == 0)
        d->loadSpanInitialPos = cursor.position();

    for (KoXmlNode node = element.firstChild(); !node.isNull(); node = node.nextSibling()) {
        KoXmlElement ts = node.toElement();
        const QString localName(ts.localName());
        const bool isTextNS = ts.namespaceURI() == KoXmlNS::text;
        const bool isDrawNS = ts.namespaceURI() == KoXmlNS::draw;
        const bool isOfficeNS = ts.namespaceURI() == KoXmlNS::office;

        if (node.isText()) {
            QString text = node.toText().data();
#ifdef KOOPENDOCUMENTLOADER_DEBUG
            kDebug(32500) << "  <text> localName=" << localName << " parent.localName=" << element.localName() << " text=" << text
            << text.length();
#endif
            text = KoTextLoaderP::normalizeWhitespace(text, *stripLeadingSpace);

            if (!text.isEmpty()) {
                // if present text ends with a space,
                // we can remove the leading space in the next text
                *stripLeadingSpace = text[text.length() - 1].isSpace();

                if (d->changeTracker && d->changeStack.count()) {
                    QTextCharFormat format;
                    format.setProperty(KoCharacterStyle::ChangeTrackerId, d->changeStack.top());
                    cursor.mergeCharFormat(format);
                }
                cursor.insertText(text);

                if (d->loadSpanLevel == 1 && node.nextSibling().isNull()
                        && cursor.position() > d->loadSpanInitialPos) {
                    QTextCursor tempCursor(cursor);
                    tempCursor.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, 1); // select last char loaded
                    if (tempCursor.selectedText() == " " && *stripLeadingSpace) {            // if it's a collapsed blankspace
                        tempCursor.removeSelectedText();                                    // remove it
                    }
                }
            }
        } else if (isTextNS && localName == "change-start") { // text:change-start
            d->openChangeRegion(ts);
        } else if (isTextNS && localName == "change-end") {
            d->closeChangeRegion(ts);
        } else if (isTextNS && localName == "change") {
            QString id = ts.attributeNS(KoXmlNS::text, "change-id");
            int changeId = d->changeTracker->getLoadedChangeId(id);
            if (changeId) {
                if (d->changeStack.count())
                    d->changeTracker->setParent(changeId, d->changeStack.top());
                KoDeleteChangeMarker *deleteChangemarker = new KoDeleteChangeMarker(d->changeTracker);
                deleteChangemarker->setChangeId(changeId);
                KoChangeTrackerElement *changeElement = d->changeTracker->elementById(changeId);
                changeElement->setDeleteChangeMarker(deleteChangemarker);
                changeElement->setEnabled(true);
                KoTextDocumentLayout *layout = qobject_cast<KoTextDocumentLayout*>(cursor.block().document()->documentLayout());

                if (layout) {
                    KoInlineTextObjectManager *textObjectManager = layout->inlineTextObjectManager();
                    textObjectManager->insertInlineObject(cursor, deleteChangemarker);
                }

                loadDeleteChangeWithinPorH(id, cursor);
            }
        } else if (isTextNS && localName == "span") { // text:span
#ifdef KOOPENDOCUMENTLOADER_DEBUG
            kDebug(32500) << "  <span> localName=" << localName;
#endif
            QString styleName = ts.attributeNS(KoXmlNS::text, "style-name", QString());

            QTextCharFormat cf = cursor.charFormat(); // store the current cursor char format

            KoCharacterStyle *characterStyle = d->textSharedData->characterStyle(styleName, d->stylesDotXml);
            if (characterStyle) {
                characterStyle->applyStyle(&cursor);
            } else {
                kWarning(32500) << "character style " << styleName << " not found";
            }

            loadSpan(ts, cursor, stripLeadingSpace);   // recurse
            cursor.setCharFormat(cf); // restore the cursor char format
        } else if (isTextNS && localName == "s") { // text:s
            int howmany = 1;
            if (ts.hasAttributeNS(KoXmlNS::text, "c")) {
                howmany = ts.attributeNS(KoXmlNS::text, "c", QString()).toInt();
            }
            cursor.insertText(QString().fill(32, howmany));
        } else if ( (isTextNS && localName == "note")) { // text:note
            loadNote(ts, cursor);
        } else if (isTextNS && localName == "tab") { // text:tab
            cursor.insertText("\t");
        } else if (isTextNS && localName == "a") { // text:a
            QString target = ts.attributeNS(KoXmlNS::xlink, "href");
            QTextCharFormat cf = cursor.charFormat(); // store the current cursor char format
            if (!target.isEmpty()) {
                QTextCharFormat linkCf(cf);   // and copy it to alter it
                linkCf.setAnchor(true);
                linkCf.setAnchorHref(target);

                // TODO make configurable ? Ho, and it will interfere with saving :/
                QBrush foreground = linkCf.foreground();
                foreground.setColor(Qt::blue);
//                 foreground.setStyle(Qt::Dense1Pattern);
                linkCf.setForeground(foreground);
                linkCf.setProperty(KoCharacterStyle::UnderlineStyle, KoCharacterStyle::SolidLine);
                linkCf.setProperty(KoCharacterStyle::UnderlineType, KoCharacterStyle::SingleLine);

                cursor.setCharFormat(linkCf);
            }
            loadSpan(ts, cursor, stripLeadingSpace);   // recurse
            cursor.setCharFormat(cf);   // restore the cursor char format
        } else if (isTextNS && localName == "line-break") { // text:line-break
#ifdef KOOPENDOCUMENTLOADER_DEBUG
            kDebug(32500) << "  <line-break> Node localName=" << localName;
#endif
            cursor.insertText(QChar(0x2028));
        }
        else if (isTextNS && localName == "meta") {
            kDebug(30015) << "loading a text:meta";
            KoTextDocumentLayout *layout = qobject_cast<KoTextDocumentLayout*>(cursor.block().document()->documentLayout());
            if (layout) {
                const QTextDocument *document = cursor.block().document();
                KoInlineTextObjectManager *textObjectManager = layout->inlineTextObjectManager();
                KoTextMeta* startmark = new KoTextMeta(document);
                textObjectManager->insertInlineObject(cursor, startmark);

                // Add inline Rdf here.
                if (ts.hasAttributeNS(KoXmlNS::xhtml, "property")
                        || ts.hasAttribute("id")) {
                    KoTextInlineRdf* inlineRdf =
                        new KoTextInlineRdf((QTextDocument*)document, startmark);
                    inlineRdf->loadOdf(ts);
                    startmark->setInlineRdf(inlineRdf);
                }

                loadSpan(ts, cursor, stripLeadingSpace);   // recurse

                KoTextMeta* endmark = new KoTextMeta(document);
                textObjectManager->insertInlineObject(cursor, endmark);
                startmark->setEndBookmark(endmark);
            }
        }
        // text:bookmark, text:bookmark-start and text:bookmark-end
        else if (isTextNS && (localName == "bookmark" || localName == "bookmark-start" || localName == "bookmark-end")) {
            QString bookmarkName = ts.attribute("name");

            KoTextDocumentLayout *layout = qobject_cast<KoTextDocumentLayout*>(cursor.block().document()->documentLayout());
            if (layout) {
                const QTextDocument *document = cursor.block().document();
                KoInlineTextObjectManager *textObjectManager = layout->inlineTextObjectManager();
                // For cut and paste, make sure that the name is unique.
                QString uniqBookmarkName = createUniqueBookmarkName(textObjectManager->bookmarkManager(),
                                           bookmarkName,
                                           (localName == "bookmark-end"));
                KoBookmark *bookmark = new KoBookmark(uniqBookmarkName, document);

                if (localName == "bookmark")
                    bookmark->setType(KoBookmark::SinglePosition);
                else if (localName == "bookmark-start") {
                    bookmark->setType(KoBookmark::StartBookmark);

                    // Add inline Rdf to the bookmark.
                    if (ts.hasAttributeNS(KoXmlNS::xhtml, "property")
                            || ts.hasAttribute("id")) {
                        KoTextInlineRdf* inlineRdf =
                            new KoTextInlineRdf((QTextDocument*)document, bookmark);
                        inlineRdf->loadOdf(ts);
                        bookmark->setInlineRdf(inlineRdf);
                    }
                } else if (localName == "bookmark-end") {
                    bookmark->setType(KoBookmark::EndBookmark);
                    KoBookmark *startBookmark = textObjectManager->bookmarkManager()->retrieveBookmark(uniqBookmarkName);
                    startBookmark->setEndBookmark(bookmark);
                }
                textObjectManager->insertInlineObject(cursor, bookmark);
            }
        } else if (isTextNS && localName == "bookmark-ref") {
            QString bookmarkName = ts.attribute("ref-name");
            QTextCharFormat cf = cursor.charFormat(); // store the current cursor char format
            if (!bookmarkName.isEmpty()) {
                QTextCharFormat linkCf(cf); // and copy it to alter it
                linkCf.setAnchor(true);
                QStringList anchorName;
                anchorName << bookmarkName;
                linkCf.setAnchorNames(anchorName);
                cursor.setCharFormat(linkCf);
            }
            bool stripLeadingSpace = true;
            loadSpan(ts, cursor, &stripLeadingSpace);   // recurse
            cursor.setCharFormat(cf);   // restore the cursor char format
        } else if (isTextNS && localName == "number") { // text:number
            /*                ODF Spec, §4.1.1, Formatted Heading Numbering
            If a heading has a numbering applied, the text of the formatted number can be included in a
            <text:number> element. This text can be used by applications that do not support numbering of
            headings, but it will be ignored by applications that support numbering.                   */
        } else if (isDrawNS) {
            loadShape(ts, cursor);
        } else {
            KoInlineObject *obj = KoInlineObjectRegistry::instance()->createFromOdf(ts, d->context);

            if (obj) {
                KoTextDocumentLayout *layout = qobject_cast<KoTextDocumentLayout*>(cursor.block().document()->documentLayout());
                if (layout) {
                    KoInlineTextObjectManager *textObjectManager = layout->inlineTextObjectManager();
                    if (textObjectManager) {
                        KoVariableManager *varManager = textObjectManager->variableManager();
                        if (varManager) {
                            textObjectManager->insertInlineObject(cursor, obj);
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
                kDebug(32500) << "Node '" << localName << "' unhandled";
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
    int loadedTags = 0;

    QTextCharFormat charFormat = cursor.block().charFormat();
    QTextBlockFormat blockFormat = cursor.block().blockFormat();

    if (changeId) {
        KoChangeTrackerElement *changeElement = d->changeTracker->elementById(changeId);
        KoXmlElement element = d->deleteChangeTable.value(id);
        KoXmlElement tag;
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
                /********************** ODF Bug Work-around code that uses RDF ***************************/
                bool listValid = true;
                int deletedListLevel = 0;
                if (tag.hasAttribute("id")) {
                    QString xmlId = tag.attribute("id", QString());
                    listValid = isValidList(xmlId);
                    deletedListLevel = listLevel(xmlId);
                }

                if (listValid || (deletedListLevel && (deletedListLevel != (d->currentListLevel - 1))))
                    cursor.insertBlock(blockFormat, charFormat);
                /******************************************************************************************/
                loadList(tag, cursor, true);
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

void KoTextLoader::loadTable(const KoXmlElement &tableElem, QTextCursor &cursor)
{
    cursor.insertText("\n");
    cursor.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor, 1);
    QTextTableFormat tableFormat;
    QString tableStyleName = tableElem.attributeNS(KoXmlNS::table, "style-name", "");
    if (!tableStyleName.isEmpty()) {
        KoTableStyle *tblStyle = d->textSharedData->tableStyle(tableStyleName, d->stylesDotXml);
        if (tblStyle)
            tblStyle->applyStyle(tableFormat);
    }
    KoTableColumnAndRowStyleManager *tcarManager = new KoTableColumnAndRowStyleManager;
    tableFormat.setProperty(KoTableStyle::ColumnAndRowStyleManager, QVariant::fromValue(reinterpret_cast<void *>(tcarManager)));
    QTextTable *tbl = cursor.insertTable(1, 1, tableFormat);
    int rows = 0;
    int columns = 0;
    QList<QRect> spanStore; //temporary list to store spans until the entire table have been created
    KoXmlElement tblTag;
    forEachElement(tblTag, tableElem) {
        if (! tblTag.isNull()) {
            const QString tblLocalName = tblTag.localName();
            if (tblTag.namespaceURI() == KoXmlNS::table) {
                if (tblLocalName == "table-column") {
                    // Do some parsing with the column, see §8.2.1, ODF 1.1 spec
                    int repeatColumn = tblTag.attributeNS(KoXmlNS::table, "number-columns-repeated", "1").toInt();
                    QString columnStyleName = tblTag.attributeNS(KoXmlNS::table, "style-name", "");
                    if (!columnStyleName.isEmpty()) {
                        KoTableColumnStyle *columnStyle = d->textSharedData->tableColumnStyle(columnStyleName, d->stylesDotXml);
                        for (int c = columns; c < columns + repeatColumn; c++) {
                            tcarManager->setColumnStyle(c, columnStyle);
                        }
                    }
                    columns = columns + repeatColumn;
                    if (rows > 0)
                        tbl->resize(rows, columns);
                    else
                        tbl->resize(1, columns);
                } else if (tblLocalName == "table-row") {
                    QString rowStyleName = tblTag.attributeNS(KoXmlNS::table, "style-name", "");
                    if (!rowStyleName.isEmpty()) {
                        KoTableRowStyle *rowStyle = d->textSharedData->tableRowStyle(rowStyleName, d->stylesDotXml);
                        tcarManager->setRowStyle(rows, rowStyle);
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
                                    // Ok, it's a cell...
                                    const int currentRow = tbl->rows() - 1;
                                    QTextTableCell cell = tbl->cellAt(currentRow, currentCell);

                                    // store spans until entire table have been loaded
                                    int rowsSpanned = rowTag.attributeNS(KoXmlNS::table, "number-rows-spanned", "1").toInt();
                                    int columnsSpanned = rowTag.attributeNS(KoXmlNS::table, "number-columns-spanned", "1").toInt();
                                    spanStore.append(QRect(currentCell, currentRow, columnsSpanned, rowsSpanned));

                                    if (cell.isValid()) {
                                        QString cellStyleName = rowTag.attributeNS(KoXmlNS::table, "style-name", "");
                                        if (!cellStyleName.isEmpty()) {
                                            KoTableCellStyle *cellStyle = d->textSharedData->tableCellStyle(cellStyleName, d->stylesDotXml);
                                            QTextTableCellFormat cellFormat = cell.format().toTableCellFormat();
                                            if (cellStyle)
                                                cellStyle->applyStyle(cellFormat);
                                            cell.setFormat(cellFormat);
                                        }

                                        // handle inline Rdf
                                        // rowTag is the current table cell.
                                        if (rowTag.hasAttributeNS(KoXmlNS::xhtml, "property")
                                                || rowTag.hasAttribute("id")) {
                                            KoTextInlineRdf* inlineRdf =
                                                new KoTextInlineRdf((QTextDocument*)cursor.block().document(),
                                                        cell);
                                            inlineRdf->loadOdf(rowTag);
                                            QTextTableCellFormat cellFormat = cell.format().toTableCellFormat();
                                            cellFormat.setProperty(KoTableCellStyle::InlineRdf,
                                                    QVariant::fromValue(inlineRdf));
                                            cell.setFormat(cellFormat);
                                        }

                                        cursor = cell.firstCursorPosition();
                                        loadBody(rowTag, cursor);
                                    } else
                                        kDebug(32500) << "Invalid table-cell row=" << currentRow << " column=" << currentCell;
                                    currentCell++;
                                } else if (rowLocalName == "covered-table-cell") {
                                    currentCell++;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    // Finally create spans
    foreach(const QRect &span, spanStore) {
        tbl->mergeCells(span.y(), span.x(), span.height(), span.width()); // for some reason Qt takes row, column
    }
    cursor = tbl->lastCursorPosition();
    cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, 1);
}

void KoTextLoader::loadShape(const KoXmlElement &element, QTextCursor &cursor)
{
    KoShape *shape = KoShapeRegistry::instance()->createShapeFromOdf(element, d->context);
    if (!shape) {
        kDebug(32500) << "shape '" << element.localName() << "' unhandled";
        return;
    }

    QString anchorType;
    if (shape->hasAdditionalAttribute("text:anchor-type"))
        anchorType = shape->additionalAttribute("text:anchor-type");
    else if (element.hasAttributeNS(KoXmlNS::text, "anchor-type"))
        anchorType = element.attributeNS(KoXmlNS::text, "anchor-type");
    else
        anchorType = "as-char"; // default value

    // page anchored shapes are handled differently
    if (anchorType != "page") {
        KoTextAnchor *anchor = new KoTextAnchor(shape);
        anchor->loadOdf(element, d->context);
        d->textSharedData->shapeInserted(shape, element, d->context);

        KoTextDocumentLayout *layout = qobject_cast<KoTextDocumentLayout*>(cursor.block().document()->documentLayout());
        if (layout) {
            KoInlineTextObjectManager *textObjectManager = layout->inlineTextObjectManager();
            if (textObjectManager) {
                textObjectManager->insertInlineObject(cursor, anchor);
            }
        }
    } else {
        d->textSharedData->shapeInserted(shape, element, d->context);
    }
}

void KoTextLoader::loadTableOfContents(const KoXmlElement &element, QTextCursor &cursor)
{
    // Add a frame to the current layout
    QTextFrameFormat tocFormat;
    tocFormat.setProperty(KoText::TableOfContents, true);
    cursor.insertFrame(tocFormat);
    // Get the cursor of the frame
    QTextCursor cursorFrame = cursor.currentFrame()->lastCursorPosition();

    // We'll just try to find displayable elements and add them as paragraphs
    KoXmlElement e;
    forEachElement(e, element) {
        if (e.isNull() || e.namespaceURI() != KoXmlNS::text)
            continue;

        //TODO look at table-of-content-source

        // We look at the index body now
        if (e.localName() == "index-body") {
            KoXmlElement p;
            bool firstTime = true;
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
        }
    }
    // Get out of the frame
    cursor.movePosition(QTextCursor::End);
}

void KoTextLoader::startBody(int total)
{
    d->bodyProgressTotal += total;
}

void KoTextLoader::processBody()
{
    d->bodyProgressValue++;
    if (d->dt.elapsed() >= d->nextProgressReportMs) {  // update based on elapsed time, don't saturate the queue
        d->nextProgressReportMs = d->dt.elapsed() + 333; // report 3 times per second
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

#include <KoTextLoader.moc>
