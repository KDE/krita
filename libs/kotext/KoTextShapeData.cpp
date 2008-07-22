/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2008 Girish Ramakrishnan <girish@forwardbias.in>
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

#include "KoTextShapeData.h"
#include <KoXmlWriter.h>

#include <KDebug>
#include <QUrl>
#include <QTextDocument>
#include <QTextBlock>

#include <KoGenStyle.h>
#include <KoGenStyles.h>
#include <KoShapeLoadingContext.h>
#include <KoOdfLoadingContext.h>
#include <KoShapeSavingContext.h>

#include "KoInlineObject.h"
#include "KoInlineTextObjectManager.h"
#include "styles/KoStyleManager.h"
#include "styles/KoCharacterStyle.h"
#include "styles/KoParagraphStyle.h"
#include "styles/KoListStyle.h"
#include "styles/KoListLevelProperties.h"
#include "KoTextDocumentLayout.h"
#include "KoTextBlockData.h"

#include "opendocument/KoTextLoader.h"

#include "KoTextDebug.h"

class KoTextShapeData::Private {
public:
    Private()
        : document(0),
        ownsDocument(true),
        dirty(true),
        offset(0.0),
        position(-1),
        endPosition(-1),
        pageNumber(-1),
        direction(KoText::AutoDirection)
    {
    }

    QString saveParagraphStyle(KoShapeSavingContext &context, const KoStyleManager *manager, const QTextBlock &block);
    QString saveCharacterStyle(KoShapeSavingContext &context, const KoStyleManager *manager, 
                               const QTextCharFormat &charFormat, const QTextCharFormat &blockCharFormat);

    QTextDocument *document;
    bool ownsDocument, dirty;
    double offset;
    int position, endPosition, pageNumber;
    KoInsets margins;
    KoText::Direction direction;
};


KoTextShapeData::KoTextShapeData()
: d(new Private())
{
    setDocument(new QTextDocument, true);
}

KoTextShapeData::~KoTextShapeData() {
    if(d->ownsDocument)
        delete d->document;
    delete d;
}

void KoTextShapeData::setDocument(QTextDocument *document, bool transferOwnership) {
    Q_ASSERT(document);
    if(d->ownsDocument && document != d->document)
        delete d->document;
    d->document = document;
    // The following avoids the normal case where the glyph metrices are rounded to integers and
    // hinted to the screen by freetype, which you of course don't want for WYSIWYG
    if(! d->document->useDesignMetrics())
        d->document->setUseDesignMetrics(true);
    d->ownsDocument = transferOwnership;
    d->document->setDefaultFont(QFont("Sans Serif", 12, QFont::Normal, false));
}

QTextDocument *KoTextShapeData::document() {
    return d->document;
}

double KoTextShapeData::documentOffset() const {
    return d->offset;
}

void KoTextShapeData::setDocumentOffset(double offset) {
    d->offset = offset;
}

int KoTextShapeData::position() const {
    return d->position;
}

void KoTextShapeData::setPosition(int position) {
    d->position = position;
}

int KoTextShapeData::endPosition() const {
    return d->endPosition;
}

void KoTextShapeData::setEndPosition(int position) {
    d->endPosition = position;
}

void KoTextShapeData::foul() {
    d->dirty = true;
}

void KoTextShapeData::wipe() {
    d->dirty = false;
}

bool KoTextShapeData::isDirty() const {
    return d->dirty;
}

void KoTextShapeData::fireResizeEvent() {
    emit relayout();
}

void KoTextShapeData::setShapeMargins(const KoInsets &margins) {
    d->margins = margins;
}

KoInsets KoTextShapeData::shapeMargins() const {
    return d->margins;
}

void KoTextShapeData::setPageNumber(int page) {
    if (page == d->pageNumber)
        return;
    d->pageNumber = page;
    d->dirty = true;
}

int KoTextShapeData::pageNumber() const {
    return d->pageNumber;
}

void KoTextShapeData::setPageDirection(KoText::Direction direction) {
    d->direction = direction;
}

KoText::Direction KoTextShapeData::pageDirection() const {
    return d->direction;
}

bool KoTextShapeData::loadOdf(const KoXmlElement & element, KoShapeLoadingContext & context) {
    KoTextLoader loader( context );

    QTextCursor cursor( document() );
    document()->clear();
    loader.loadBody( element, cursor ); // now let's load the body from the ODF KoXmlElement.

    return true;
}

QString KoTextShapeData::Private::saveParagraphStyle(KoShapeSavingContext &context, const KoStyleManager *styleManager, const QTextBlock &block)
{
    Q_ASSERT(styleManager);
    QTextBlockFormat blockFormat = block.blockFormat();
    QTextCharFormat charFormat = QTextCursor(block).blockCharFormat(); // fix this after TT Task 219905
 
    KoParagraphStyle *originalParagraphStyle = styleManager->paragraphStyle(blockFormat.intProperty(KoParagraphStyle::StyleId));
    QString displayName, internalName, generatedName;
    if (originalParagraphStyle) {
        displayName = originalParagraphStyle->name();
        internalName = QString(QUrl::toPercentEncoding(displayName, "", " ")).replace("%", "_");
    }

    // we'll convert the blockFormat to a KoParagraphStyle to check for local changes.
    KoParagraphStyle paragStyle(blockFormat, charFormat);
    if (originalParagraphStyle && (paragStyle == (*originalParagraphStyle))) { // This is the real, unmodified character style.
        KoGenStyle style(KoGenStyle::StyleUser, "paragraph");
        originalParagraphStyle->saveOdf(style);
        generatedName = context.mainStyles().lookup(style, internalName, KoGenStyles::DontForceNumbering);
    } else { // There are manual changes... We'll have to store them then
        KoGenStyle style(KoGenStyle::StyleAuto, "paragraph", internalName);
        if (context.isSet(KoShapeSavingContext::AutoStyleInStyleXml))
            style.setAutoStyleInStylesDotXml( true );
        if (originalParagraphStyle)
            paragStyle.removeDuplicates(*originalParagraphStyle);
        paragStyle.saveOdf(style);
        generatedName = context.mainStyles().lookup(style, "P");
    }
    return generatedName;
}

QString KoTextShapeData::Private::saveCharacterStyle(KoShapeSavingContext &context, const KoStyleManager *styleManager, 
                                                     const QTextCharFormat &charFormat, const QTextCharFormat &blockCharFormat)
{
    Q_ASSERT(styleManager);
    KoCharacterStyle *defaultCharStyle = styleManager->defaultParagraphStyle()->characterStyle();

    KoCharacterStyle *originalCharStyle = styleManager->characterStyle(charFormat.intProperty(KoCharacterStyle::StyleId));
    if (!originalCharStyle)
        originalCharStyle = defaultCharStyle;

    QString generatedName;
    QString displayName = originalCharStyle->name();
    QString internalName = QString(QUrl::toPercentEncoding(displayName, "", " ")).replace("%", "_");

    KoCharacterStyle charStyle(charFormat);
    // we'll convert it to a KoCharacterStyle to check for local changes.
    if (charStyle == (*originalCharStyle)) { // This is the real, unmodified character style.
        if (originalCharStyle != defaultCharStyle) {
            charStyle.removeDuplicates(blockCharFormat);
            if (!charStyle.isEmpty()) {
                KoGenStyle style(KoGenStyle::StyleUser, "text");
                originalCharStyle->saveOdf(style);
                generatedName = context.mainStyles().lookup(style, internalName, KoGenStyles::DontForceNumbering);
            }
        }
    } else { // There are manual changes... We'll have to store them then
        KoGenStyle style(KoGenStyle::StyleAuto, "text", originalCharStyle != defaultCharStyle ? internalName : "" /*parent*/);
        if (context.isSet(KoShapeSavingContext::AutoStyleInStyleXml))
           style.setAutoStyleInStylesDotXml(true);
        charStyle.removeDuplicates(*originalCharStyle);
        charStyle.removeDuplicates(blockCharFormat);
        if (!charStyle.isEmpty()) {
           charStyle.saveOdf(style);
           generatedName = context.mainStyles().lookup(style, "T");
        }
    }

    return generatedName;
}

void KoTextShapeData::saveOdf(KoShapeSavingContext & context, int from, int to) const {
    KoXmlWriter *writer = &context.xmlWriter();
    QTextBlock block = d->document->findBlock(from);

    KoTextDocumentLayout *layout = dynamic_cast<KoTextDocumentLayout*>(d->document->documentLayout());
    Q_ASSERT(layout);
    Q_ASSERT(layout->inlineObjectTextManager());

    KoStyleManager *styleManager = layout->styleManager();
    if (styleManager)
        styleManager->saveOdfDefaultStyles(context.mainStyles());
  
    //TODO: The list formats are currently not stored in the KoStyleManager ??
    QMap<QTextList *, QString> listStyleNames;
    QTextBlock startBlock = block;
    while(block.isValid() && ((to == -1) || (block.position() < to))) {
        if ((block.textList()) && (!listStyleNames.contains(block.textList()))) {
            // Generate a style from that...
            KoGenStyle style(KoGenStyle::StyleList);
            KoListStyle *listStyle = KoListStyle::fromTextList(block.textList());
            listStyle->saveOdf(style);
            QString generatedName = context.mainStyles().lookup(style, listStyle->name());
            listStyleNames[block.textList()] = generatedName;
            delete(listStyle);
        }
        block = block.next();
    }
    block = startBlock;

    QList<QTextList*> textLists;	// Store the current lists being stored.
    // Ok, now that the styles are done, we can store the blocks themselves.
    while(block.isValid() && ((to == -1) || (block.position() < to))) {
        if ((block.begin().atEnd()) && (!block.next().isValid()))   // Do not add an extra empty line at the end...
            break;

        bool isHeading = false;
        KoTextBlockData *blockData = dynamic_cast<KoTextBlockData *>(block.userData());
        if (blockData)
            isHeading = (blockData->outlineLevel() > 0);
        if (block.textList() && !isHeading) {
            if ((textLists.isEmpty()) || (!textLists.contains(block.textList()))) {
                writer->startElement( "text:list", false );
                writer->addAttribute("text:style-name", listStyleNames[block.textList()]);
                textLists.append(block.textList());
            } else if (block.textList() != textLists.last()) {
                while ((!textLists.isEmpty()) && (block.textList() != textLists.last())) {
                    textLists.removeLast();
                    writer->endElement();
                }
            }
            writer->startElement( "text:list-item", false );
        } else {
            // Close any remaining list...
            while (!textLists.isEmpty()) {
                textLists.removeLast();
                writer->endElement();
            }
        }
        if (isHeading) {
            writer->startElement( "text:h", false );
            writer->addAttribute( "text:outline-level", blockData->outlineLevel() );
        } else {
            writer->startElement( "text:p", false );
        }

        // Write the block format
        QString styleName = d->saveParagraphStyle(context, styleManager, block);
        if (!styleName.isEmpty())
            writer->addAttribute("text:style-name", styleName);

        // Write the fragments and their formats
        QTextCursor cursor(block);
        QTextCharFormat blockCharFormat = cursor.blockCharFormat(); // Fix this after TT task 219905
 
        QTextBlock::iterator it;
        for (it = block.begin(); !(it.atEnd()); ++it) {
            QTextFragment currentFragment = it.fragment();
            const int fragmentStart = currentFragment.position();
            const int fragmentEnd = fragmentStart + currentFragment.length();
            if (to != -1 && fragmentStart >= to)
                break;
            if (currentFragment.isValid()) {
                QTextCharFormat charFormat = currentFragment.charFormat();
                KoInlineObject *inlineObject = layout->inlineObjectTextManager()->inlineTextObject((const QTextCharFormat &)charFormat);
                if (inlineObject) {
                    inlineObject->saveOdf(context);
                } else {
                    QString styleName = d->saveCharacterStyle(context, styleManager, charFormat, blockCharFormat);

                    if (charFormat.isAnchor()) {
                        writer->startElement("text:a", false);
                        writer->addAttribute("xlink:type", "simple");
                        writer->addAttribute("xlink:href", charFormat.anchorHref());
                    } else if (!styleName.isEmpty()) {
                        writer->startElement("text:span", false);
                        writer->addAttribute("text:style-name", styleName);
                    }
                    
                    QString text = currentFragment.text();
                    int spanFrom = fragmentStart >= from ? 0 : from;
                    int spanTo = to == -1 ? fragmentEnd : (fragmentEnd > to ? to : fragmentEnd);
                    if (spanFrom != fragmentStart || spanTo != fragmentEnd) { // avoid mid, if possible
                        writer->addTextSpan(text.mid(spanFrom - fragmentStart, spanTo - spanFrom));
                    } else {
                        writer->addTextSpan(text);
                    }

                    if (!styleName.isEmpty() || charFormat.isAnchor())
                        writer->endElement();
                } // if (inlineObject)
            } // if (fragment.valid())
        } // foeach(fragment)

        writer->endElement();

	    if (block.textList() && !isHeading)	// We must check if we need to close a previously-opened text:list node.
	        writer->endElement();

        block = block.next();
    } // while
}

#include "KoTextShapeData.moc"
