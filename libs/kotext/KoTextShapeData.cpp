/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
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
#include <KoOasisLoadingContext.h>
#include <KoShapeSavingContext.h>

#include "KoInlineObject.h"
#include "KoInlineTextObjectManager.h"
#include "styles/KoStyleManager.h"
#include "styles/KoCharacterStyle.h"
#include "styles/KoParagraphStyle.h"
#include "KoTextDocumentLayout.h"

#include "opendocument/KoTextLoadingContext.h"
#include "opendocument/KoTextLoader.h"

class KoTextShapeData::Private {
public:
    Private()
        : ownsDocument(true),
        dirty(true),
        offset(0.0),
        position(-1),
        endPosition(-1),
        direction(KoText::AutoDirection)
    {
    }
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
    d->document = new QTextDocument();
    d->document->setUseDesignMetrics(true);
    d->document->setDefaultFont(QFont("Sans Serif", 12, QFont::Normal, false));
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

void KoTextShapeData::faul() {
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
    d->pageNumber = page;
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
    KoOasisLoadingContext &oasisLoadingContext = context.koLoadingContext();

    KoTextLoadingContext *textLoadingContext = dynamic_cast<KoTextLoadingContext*>( &oasisLoadingContext );
    Q_ASSERT(textLoadingContext);
    if( ! textLoadingContext )
        return false;

    KoTextLoader* loader = textLoadingContext->loader();
    Q_ASSERT(loader);
    if( ! loader )
        return false;

    QTextCursor cursor( document() );
    loader->loadBody(*textLoadingContext, element, cursor);
    return true;
}

void KoTextShapeData::saveOdf(KoShapeSavingContext & context, int from, int to) const {
    KoXmlWriter *writer = &context.xmlWriter();
    QTextBlock block = d->document->findBlock(from);
    kDebug() << "The document is " << d->document;

    KoStyleManager *styleManager = 0;
    KoTextDocumentLayout *lay = dynamic_cast<KoTextDocumentLayout*> (d->document->documentLayout());
    QHash<int, QString> styleNames; // Store an int index for a QTextFormat => ODF style name
    QVector<QTextFormat> allFormats = d->document->allFormats(); // Will be used to get the indexes.
    QTextFormat firstFragmentFormat;
    if (lay) {
        styleManager = lay->styleManager();
        if (styleManager) {
            // Ok, now we will iterate over the QTextFormat contained in this textFrameSet.
            foreach (QTextFormat textFormat, allFormats) {
                if (textFormat.type() == QTextFormat::BlockFormat) {
                    // This is a QTextBlockFormat.
                    KoParagraphStyle *originalParagraphStyle = styleManager->paragraphStyle(textFormat.intProperty(KoParagraphStyle::StyleId));
                    if (!originalParagraphStyle) {
                        continue;
                    }
                    // we'll convert it to a KoParagraphStyle to check for local changes.
                    KoParagraphStyle paragStyle = KoParagraphStyle(textFormat);
                    QString displayName = originalParagraphStyle->name();
                    QString internalName = QString(QUrl::toPercentEncoding(displayName, "", " ")).replace("%", "_");
                    QString generatedName;
                    if (paragStyle == (*originalParagraphStyle)) {
                        // This is the real, unmodified character style.
                        KoGenStyle test(KoGenStyle::StyleUser, "paragraph");
                        test.setAutoStyleInStylesDotXml(true);
                        originalParagraphStyle->saveOdf(&test);
                        generatedName = context.mainStyles().lookup(test, internalName);
                    } else {
                        // There are manual changes... We'll have to store them then
                        KoGenStyle test(KoGenStyle::StyleAuto, "paragraph", internalName);
                        test.setAutoStyleInStylesDotXml(false);
                        paragStyle.removeDuplicates(*originalParagraphStyle);
                        paragStyle.saveOdf(&test);
                        generatedName = context.mainStyles().lookup(test, "P");
                    }
                    styleNames[allFormats.indexOf(textFormat)] = generatedName;
                } else if (textFormat.type() == QTextFormat::CharFormat) {
                    if (textFormat.objectType() == 1001)
                        continue;
                    // This is a QTextCharFormat.
                    KoCharacterStyle *originalCharStyle = styleManager->characterStyle(textFormat.intProperty(KoCharacterStyle::StyleId));
                    if (!originalCharStyle) {
                        continue;
                    }
                    // we'll convert it to a KoCharacterStyle to check for local changes.
                    KoCharacterStyle charStyle = KoCharacterStyle(textFormat);
                    QString generatedName;
                    QString displayName = originalCharStyle->name();
                    QString internalName = QString(QUrl::toPercentEncoding(displayName, "", " ")).replace("%", "_");
                    if (charStyle == (*originalCharStyle)) {
                        // This is the real, unmodified character style.
                        KoGenStyle test(KoGenStyle::StyleUser, "text");
                        test.setAutoStyleInStylesDotXml(true);
                        originalCharStyle->saveOdf(&test);
                        generatedName = context.mainStyles().lookup(test, internalName);
                        // Check whether it is the default char format...
                        if (styleManager->defaultParagraphStyle()) {
                            KoParagraphStyle *defaultParagraph = styleManager->defaultParagraphStyle();
                            if ((*(defaultParagraph->characterStyle())) == charStyle)
                                firstFragmentFormat = textFormat;
                        }
                    } else {
                        // There are manual changes... We'll have to store them then
                        KoGenStyle test(KoGenStyle::StyleAuto, "text", internalName);
                        test.setAutoStyleInStylesDotXml(false);
                        charStyle.removeDuplicates(*originalCharStyle);
                        charStyle.saveOdf(&test);
                        generatedName = context.mainStyles().lookup(test, "T");
                    }
                    styleNames[allFormats.indexOf(textFormat)] = generatedName;
                } else {
                    // It doesn't matter yet.
                }
            }
        }
    }
    while(block.isValid() && ((to == -1) || (block.position() < to))) {
        writer->startElement( "text:p", false );
        if (styleNames.contains(allFormats.indexOf(block.blockFormat())))
            writer->addAttribute("text:style-name", styleNames[allFormats.indexOf(block.blockFormat())]);
        QTextBlock::iterator it;
        for (it = block.begin(); !(it.atEnd()); ++it) {
            QTextFragment currentFragment = it.fragment();
            if (currentFragment.isValid()) {
                QTextFormat charFormat = currentFragment.charFormat();
                KoTextDocumentLayout *layout = dynamic_cast<KoTextDocumentLayout*> ( d->document->documentLayout() );
                Q_ASSERT(layout && layout->inlineObjectTextManager());
                KoInlineObject *inlineObject = layout->inlineObjectTextManager()->inlineTextObject((const QTextCharFormat &)charFormat);
                if (inlineObject) {
                    // Hey, there is a KoInlineTextObject in this fragment, stop here !
                    //writer->addTextSpan("Here will come an inlineTextObject");
                    inlineObject->saveOdf(context);
                } else {
                    if (currentFragment.charFormat() != firstFragmentFormat) {
                        writer->startElement( "text:span", false );
                        if (styleNames.contains(allFormats.indexOf(charFormat)))
                            writer->addAttribute("text:style-name", styleNames[allFormats.indexOf(charFormat)]);
                    }
                    writer->addTextSpan( currentFragment.text() );
                    if (currentFragment.charFormat() != firstFragmentFormat)
                        writer->endElement( );
                }
            }
        }
        writer->endElement();
        block = block.next();
    }
}

#include "KoTextShapeData.moc"
