/* This file is part of the KDE project
 * Copyright (C) 2006, 2009-2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2008 Girish Ramakrishnan <girish@forwardbias.in>
 * Copyright (C) 2011 C. Boemann <cbo@boemann.dk>
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

#include <KoTextShapeDataBase.h>
#include <KoTextShapeDataBase_p.h>
#include "KoTextDocument.h"
#include <KoTextEditor.h>
#include "styles/KoStyleManager.h"
#include "styles/KoParagraphStyle.h"

#include <KoTextLayoutRootArea.h>

#include <QTextDocument>
#include <QTextBlock>
#include <QTextCursor>

#include <KoGenStyle.h>
#include <KoOdfLoadingContext.h>
#include <KoShapeLoadingContext.h>
#include <KoShapeSavingContext.h>
#include <KoDocumentRdfBase.h>
#include <KoStyleStack.h>
#include <KoShape.h>
#include <KoUnit.h>

#include <KoXmlNS.h>

#include "KoTextPage.h"

#include "opendocument/KoTextLoader.h"
#include "opendocument/KoTextWriter.h"

#include <TextLayoutDebug.h>

class KoTextShapeDataPrivate : public KoTextShapeDataBasePrivate
{
public:
    KoTextShapeDataPrivate()
            : ownsDocument(true)
            , topPadding(0)
            , leftPadding(0)
            , rightPadding(0)
            , bottomPadding(0)
            , direction(KoText::AutoDirection)
            , textpage(0)
            , rootArea(0)
            , paragraphStyle(0)
    {
    }

    virtual ~KoTextShapeDataPrivate()
    {
        if (ownsDocument) {
            delete document;
        }
        delete textpage;
        delete paragraphStyle;
    }

    bool ownsDocument;
    qreal topPadding;
    qreal leftPadding;
    qreal rightPadding;
    qreal bottomPadding;
    KoText::Direction direction;
    KoTextPage *textpage;
    KoTextLayoutRootArea *rootArea;
    KoParagraphStyle *paragraphStyle; // the paragraph style of the shape (part of the graphic style)
};


KoTextShapeData::KoTextShapeData()
    : KoTextShapeDataBase(*(new KoTextShapeDataPrivate()))
{
    setDocument(new QTextDocument, true);
}

KoTextShapeData::~KoTextShapeData()
{
}

void KoTextShapeData::setDocument(QTextDocument *document, bool transferOwnership)
{
    Q_D(KoTextShapeData);
    Q_ASSERT(document);
    if (d->ownsDocument && document != d->document) {
        delete d->document;
    }
    d->ownsDocument = transferOwnership;

    // The following avoids the normal case where the glyph metrices are rounded to integers and
    // hinted to the screen by freetype, which you of course don't want for WYSIWYG
    if (! document->useDesignMetrics())
        document->setUseDesignMetrics(true);

    KoTextDocument kodoc(document);

    if (document->isEmpty() && !document->firstBlock().blockFormat().hasProperty(KoParagraphStyle::StyleId)) { // apply app default style for first parag
        KoStyleManager *sm = kodoc.styleManager();
        if (sm) {
            KoParagraphStyle *defaultStyle = sm->defaultParagraphStyle();
            if (defaultStyle) {
                QTextBlock block = document->begin();
                defaultStyle->applyStyle(block);
            }
        }
    }

    // After setting the document (even if not changing it) we need to explicitly set the root area
    // to 0. Otherwise crashes may occur when inserting textshape in words (or resetting document)
    d->rootArea = 0;

    if (d->document == document)
        return;
    d->document = document;

    if (kodoc.textEditor() == 0)
        kodoc.setTextEditor(new KoTextEditor(d->document));
}

qreal KoTextShapeData::documentOffset() const
{
    Q_D(const KoTextShapeData);
    if (d->rootArea) {
        KoBorder *border = d->rootArea->associatedShape()->border();
        if (border) {
            return d->rootArea->top() - topPadding() - border->borderWidth(KoBorder::TopBorder);
        } else {
            return d->rootArea->top() - topPadding();
        }
    } else {
        return 0.0;
    }
}

void KoTextShapeData::setDirty()
{
    Q_D(KoTextShapeData);
    if (d->rootArea) {
        d->rootArea->setDirty();
    }
}


bool KoTextShapeData::isDirty() const
{
    Q_D(const KoTextShapeData);
    if (d->rootArea) {
        return d->rootArea->isDirty();
    }
    return true;
}

void KoTextShapeData::setPageDirection(KoText::Direction direction)
{
    Q_D(KoTextShapeData);
    d->direction = direction;
}

KoText::Direction KoTextShapeData::pageDirection() const
{
    Q_D(const KoTextShapeData);
    return d->direction;
}

void KoTextShapeData::setRootArea(KoTextLayoutRootArea *rootArea)
{
    Q_D(KoTextShapeData);
    d->rootArea = rootArea;
}

KoTextLayoutRootArea *KoTextShapeData::rootArea()
{
    Q_D(const KoTextShapeData);
    return d->rootArea;
}

void KoTextShapeData::setLeftPadding(qreal padding)
{
    Q_D(KoTextShapeData);
    d->leftPadding = padding;
}

qreal KoTextShapeData::leftPadding() const
{
    Q_D(const KoTextShapeData);
    return d->leftPadding;
}

void KoTextShapeData::setTopPadding(qreal padding)
{
    Q_D(KoTextShapeData);
    d->topPadding = padding;
}

qreal KoTextShapeData::topPadding() const
{
    Q_D(const KoTextShapeData);
    return d->topPadding;
}

void KoTextShapeData::setRightPadding(qreal padding)
{
    Q_D(KoTextShapeData);
    d->rightPadding = padding;
}

qreal KoTextShapeData::rightPadding() const
{
    Q_D(const KoTextShapeData);
    return d->rightPadding;
}

void KoTextShapeData::setBottomPadding(qreal padding)
{
    Q_D(KoTextShapeData);
    d->bottomPadding = padding;
}

qreal KoTextShapeData::bottomPadding() const
{
    Q_D(const KoTextShapeData);
    return d->bottomPadding;
}

void KoTextShapeData::setPadding(qreal padding)
{
    setLeftPadding(padding);
    setTopPadding(padding);
    setRightPadding(padding);
    setBottomPadding(padding);
}

bool KoTextShapeData::loadOdf(const KoXmlElement &element, KoShapeLoadingContext &context, KoDocumentRdfBase *rdfData, KoShape *shape)
{
    Q_UNUSED(rdfData);
    KoTextLoader loader(context, shape);

    QTextCursor cursor(document());
    loader.loadBody(element, cursor);   // now let's load the body from the ODF KoXmlElement.
    KoTextEditor *editor = KoTextDocument(document()).textEditor();
    if (editor) { // at one point we have to get the position from the odf doc instead.
        editor->setPosition(0);
    }

    return true;
}

void KoTextShapeData::saveOdf(KoShapeSavingContext &context, KoDocumentRdfBase *rdfData, int from, int to) const
{
    Q_D(const KoTextShapeData);

    KoTextWriter::saveOdf(context, rdfData, d->document, from, to);
}

void KoTextShapeData::loadStyle(const KoXmlElement &element, KoShapeLoadingContext &context)
{
    Q_D(KoTextShapeData);
    // load the (text) style of the frame
    const KoXmlElement *style = 0;
    if (element.hasAttributeNS(KoXmlNS::draw, "style-name")) {
        style = context.odfLoadingContext().stylesReader().findStyle(
                    element.attributeNS(KoXmlNS::draw, "style-name"), "graphic",
                    context.odfLoadingContext().useStylesAutoStyles());
        if (!style) {
            warnTextLayout << "graphic style not found:" << element.attributeNS(KoXmlNS::draw, "style-name");
        }
    }
    if (element.hasAttributeNS(KoXmlNS::presentation, "style-name")) {
        style = context.odfLoadingContext().stylesReader().findStyle(
                    element.attributeNS(KoXmlNS::presentation, "style-name"), "presentation",
                    context.odfLoadingContext().useStylesAutoStyles());
        if (!style) {
            warnTextLayout << "presentation style not found:" << element.attributeNS(KoXmlNS::presentation, "style-name");
        }
    }


    if (style) {
        KoStyleStack &styleStack = context.odfLoadingContext().styleStack();
        styleStack.save();
        context.odfLoadingContext().addStyles(style, style->attributeNS(KoXmlNS::style, "family", "graphic").toLocal8Bit().constData());   // Load all parents
        styleStack.setTypeProperties("graphic");
        // Spacing (padding)
        const QString paddingLeft(styleStack.property(KoXmlNS::fo, "padding-left" ));
        if (!paddingLeft.isEmpty()) {
            setLeftPadding(KoUnit::parseValue(paddingLeft));
        }
        const QString paddingRight(styleStack.property(KoXmlNS::fo, "padding-right" ));
        if (!paddingRight.isEmpty()) {
            setRightPadding(KoUnit::parseValue(paddingRight));
        }
        const QString paddingTop(styleStack.property(KoXmlNS::fo, "padding-top" ));
        if (!paddingTop.isEmpty()) {
            setTopPadding(KoUnit::parseValue(paddingTop));
        }
        const QString paddingBottom(styleStack.property(KoXmlNS::fo, "padding-bottom" ));
        if (!paddingBottom.isEmpty()) {
            setBottomPadding(KoUnit::parseValue(paddingBottom));
        }
        const QString padding(styleStack.property(KoXmlNS::fo, "padding"));
        if (!padding.isEmpty()) {
            setPadding(KoUnit::parseValue(padding));
        }
        styleStack.restore();

        QString family = style->attributeNS(KoXmlNS::style, "family", "graphic");
        KoParagraphStyle *defaultStyle = 0;
        const KoXmlElement *dstyle = context.odfLoadingContext().stylesReader().defaultStyle(family);
        if (dstyle) {
            defaultStyle = new KoParagraphStyle();
            defaultStyle->loadOdf(dstyle, context);
        }
        // graphic styles don't support inheritance yet therefor some additional work is needed here.
        QList<KoParagraphStyle *> paragraphStyles;
        while (style) {
            KoParagraphStyle *pStyle = new KoParagraphStyle();
            pStyle->loadOdf(style, context);
            if (!paragraphStyles.isEmpty()) {
                paragraphStyles.last()->setParentStyle(pStyle);
            }
            paragraphStyles.append(pStyle);
            QString family = style->attributeNS(KoXmlNS::style, "family", "graphic");
            style = context.odfLoadingContext().stylesReader().findStyle(
                    style->attributeNS(KoXmlNS::style, "parent-style-name"), family.toLocal8Bit().constData(),
                    context.odfLoadingContext().useStylesAutoStyles());
        }
        // rather than setting default style and apply to block we just set a final parent
        paragraphStyles.last()->setParentStyle(defaultStyle);

        QTextDocument *document = this->document();
        QTextCursor cursor(document);
        QTextBlockFormat format;
        paragraphStyles.first()->applyStyle(format);
        cursor.setBlockFormat(format);
        QTextCharFormat cformat;
        paragraphStyles.first()->KoCharacterStyle::applyStyle(cformat);
        cursor.setCharFormat(cformat);
        cursor.setBlockCharFormat(cformat);

        d->paragraphStyle = new KoParagraphStyle(format, cformat);
        qDeleteAll(paragraphStyles);
        delete defaultStyle;
    }
}

void KoTextShapeData::saveStyle(KoGenStyle &style, KoShapeSavingContext &context) const
{
    if ((leftPadding() == rightPadding()) && (topPadding() == bottomPadding()) && (rightPadding() == topPadding())) {
        style.addPropertyPt("fo:padding", leftPadding(), KoGenStyle::GraphicType);
    } else {
        if (leftPadding()) {
            style.addPropertyPt("fo:padding-left", leftPadding(), KoGenStyle::GraphicType);
        }
        if (rightPadding()) {
            style.addPropertyPt("fo:padding-right", rightPadding(), KoGenStyle::GraphicType);
        }
        if (topPadding()) {
            style.addPropertyPt("fo:padding-top", topPadding(), KoGenStyle::GraphicType);
        }
        if (bottomPadding()) {
            style.addPropertyPt("fo:padding-bottom", bottomPadding(), KoGenStyle::GraphicType);
        }
    }

    Q_D(const KoTextShapeData);
    if (d->paragraphStyle) {
        d->paragraphStyle->saveOdf(style, context);
    }
}
