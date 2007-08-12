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
#include <QTextDocument>
#include <QTextBlock>
#include <KoGenStyle.h>
#include <KoGenStyles.h>
#include <KoShapeSavingContext.h>
#include "styles/KoStyleManager.h"
#include "styles/KoCharacterStyle.h"
#include "styles/KoParagraphStyle.h"
#include "KoTextDocumentLayout.h"

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


void KoTextShapeData::saveOdf(KoShapeSavingContext & context, int from, int to) const {
    KoXmlWriter *writer = &context.xmlWriter();
    QTextBlock block = d->document->findBlock(from);
    kDebug() << "The document is " << d->document;
    
    KoStyleManager *styleManager = 0;
    KoTextDocumentLayout *lay = dynamic_cast<KoTextDocumentLayout*> (d->document->documentLayout());
    QHash<int, QString> styleNames; // Store an int index for a QTextFormat => ODF style name
    QVector<QTextFormat> allFormats = d->document->allFormats(); // Will be used to get the indexes.
    if (lay) {
        styleManager = lay->styleManager();
        if (styleManager) {
            kDebug() << "-------------------------------------------------------------------------------";
            kDebug() << "BEGINNING OF THE STYLE MESS";
            
            // First, get the style manager, we'll need it.
            KoParagraphStyle *defaultParagraphStyle = styleManager->defaultParagraphStyle();
            KoCharacterStyle *defaultCharacterStyle = defaultParagraphStyle->characterStyle();
                
            // Ok, now we will iterate over the QTextFormat contained in this textFrameSet.
            foreach (QTextFormat textFormat, allFormats) {
                kDebug() << "There is a textFormat :" << textFormat << "; type =" << textFormat.type();
                if (textFormat.type() == QTextFormat::BlockFormat) {
                    // This is a QTextBlockFormat, we'll convert it to a KoParagraphStyle
                    KoParagraphStyle paragStyle = KoParagraphStyle(textFormat);
                } else if (textFormat.type() == QTextFormat::CharFormat) {
                    // This is a QTextCharFormat.
                    KoCharacterStyle *originalCharStyle = styleManager->characterStyle(textFormat.intProperty(KoCharacterStyle::StyleId));
                    if (!originalCharStyle) {
                        continue;
                    }
                    // we'll convert it to a KoCharacterStyle to check for local changes.
                    KoCharacterStyle charStyle = KoCharacterStyle(textFormat);
                    kDebug() << "&charStyle :" << &charStyle << "  ; originalCharStyle:" << originalCharStyle;
                    KoGenStyle test(0, "text");
                    if (charStyle == (*originalCharStyle)) {
                        kDebug() << "This IS the real character style :" << originalCharStyle->name();
    //                         QString displayName = originalCharStyle->name();
    //                         QString internalName = QString(QUrl::toPercentEncoding(displayName, "", " ")).replace("%", "_");
                        originalCharStyle->saveOdf(&test);
                    } else {
                        kDebug() << "There are manual changes... We'll have to store them then";
                        charStyle.removeDuplicates(*originalCharStyle);
                        charStyle.saveOdf(&test);
                    }
                    QString generatedName = context.mainStyles().lookup(test, "T");
                    kDebug() << "Storing this style, result :" << generatedName;
                    styleNames[allFormats.indexOf(textFormat)] = generatedName;
                } else {
                    // It doesn't matter yet.
                }
            }
            kDebug() << "END OF THE STYLE MESS";
            kDebug() << "-------------------------------------------------------------------------------";
        }
    }
    while(block.isValid() && ((to == -1) || (block.position() < to))) {
        writer->startElement( "text:p", false );
        QTextBlock::iterator it;
        int firstFragmentFormat = -1;
        for (it = block.begin(); !(it.atEnd()); ++it) {
            QTextFragment currentFragment = it.fragment();
            if (currentFragment.isValid()) {
                if (firstFragmentFormat == -1)
                    firstFragmentFormat = currentFragment.charFormatIndex();
                kDebug() << "The fragment format is " << currentFragment.charFormatIndex();
                QTextFormat charFormat = currentFragment.charFormat();
                QMapIterator<int, QVariant> i(charFormat.properties());
                while (i.hasNext()) {
                    i.next();
                    kDebug() << "Key/Value : " << i.key() << "/" << i.value();
                }
                kDebug() << "StyleId in StyleManager :" << charFormat.intProperty(KoParagraphStyle::StyleId);
                if (styleManager) {
                    KoCharacterStyle *customCharStyle = styleManager->characterStyle(charFormat.intProperty(KoParagraphStyle::StyleId));
                    if (customCharStyle) {
                        kDebug() << "This format is in styleManager : " << customCharStyle->name();
                        if (customCharStyle == styleManager->defaultParagraphStyle()->characterStyle()) {
                            kDebug() << "This is the default style !";
                        }
                    }
                }
                if (currentFragment.charFormatIndex() != firstFragmentFormat) {
                    writer->startElement( "text:span", false );
                    if (styleNames.contains(allFormats.indexOf(charFormat)))
                        writer->addAttribute("text:style-name", styleNames[allFormats.indexOf(charFormat)]);
                        //kDebug() << "This charFormat has a name :" << styleNames[charFormat.objectIndex()];
                }
                writer->addTextSpan( currentFragment.text() );
                if (currentFragment.charFormatIndex() != firstFragmentFormat)
                    writer->endElement( );
            }
        }
        /*if(block.position() < from || block.position() + block.length() > to) {
            int start = qMax(0, from - block.position());
            int end = qMin(to, block.position() + block.length());
            writer->addTextSpan( block.text().mid(start, end - (start + block.position())) );
        }
        else
            writer->addTextSpan( block.text() );*/
        writer->endElement();
        block = block.next();
    }
    foreach (QTextFormat textFormat, d->document->allFormats()) {
        textFormat.clearProperty(-1);
    }
}

#include "KoTextShapeData.moc"
