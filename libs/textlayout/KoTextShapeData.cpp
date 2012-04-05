/* This file is part of the KDE project
 * Copyright (C) 2006, 2009-2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2008 Girish Ramakrishnan <girish@forwardbias.in>
 * Copyright (C) 2011 Casper Boemann <cbo@boemann.dk>
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

#include <KDebug>
#include <QUrl>
#include <QTextDocument>
#include <QTextBlock>
#include <QMetaObject>
#include <QTextCursor>

#include <KoGenStyle.h>
#include <KoGenStyles.h>
#include <KoOdfLoadingContext.h>
#include <KoShapeLoadingContext.h>
#include <KoShapeSavingContext.h>
#include <KoDocumentRdfBase.h>

#include <KoXmlWriter.h>
#include <KoXmlNS.h>

#include "KoTextPage.h"

#include "opendocument/KoTextLoader.h"
#include "opendocument/KoTextWriter.h"

#include <KoChangeTracker.h>
#include <KoChangeTrackerElement.h>
#include <KoTextAnchor.h>
#include <KoInlineTextObjectManager.h>
#include <KoCanvasBase.h>
#include <KoShapeController.h>
#include <KoShapeContainer.h>
#include <kundo2stack.h>
#include <kundo2command.h>

class KoTextShapeDataPrivate : public KoTextShapeDataBasePrivate
{
public:
    KoTextShapeDataPrivate()
            : ownsDocument(true)
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
    if (d->ownsDocument && document != d->document)
        delete d->document;
    d->ownsDocument = transferOwnership;
    if (d->document == document)
        return;
    d->document = document;
    // The following avoids the normal case where the glyph metrices are rounded to integers and
    // hinted to the screen by freetype, which you of course don't want for WYSIWYG
    if (! d->document->useDesignMetrics())
        d->document->setUseDesignMetrics(true);

    if (d->document->isEmpty() && !d->document->firstBlock().blockFormat().hasProperty(KoParagraphStyle::StyleId)) { // apply app default style for first parag
        KoTextDocument doc(d->document);
        KoStyleManager *sm = doc.styleManager();
        if (sm) {
            KoParagraphStyle *defaultStyle = sm->defaultParagraphStyle();
            if (defaultStyle) {
                QTextBlock block = d->document->begin();
                defaultStyle->applyStyle(block);
            }
        }
    }

    KoTextDocument kodoc(d->document);
    if (kodoc.textEditor() == 0)
        kodoc.setTextEditor(new KoTextEditor(d->document));
}

qreal KoTextShapeData::documentOffset() const
{
    Q_D(const KoTextShapeData);
    return d->rootArea ? d->rootArea->top() : 0.0;
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

bool KoTextShapeData::isCursorVisible(QTextCursor *cursor) const
{
    Q_UNUSED(cursor);
#ifdef __GNUC__
    #warning FIXME: KoTextShapeData::isCursorVisible returns always true, why do we still need it?
#endif
    return true;
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
            kDebug(32500) << "graphic style not found:" << element.attributeNS(KoXmlNS::draw, "style-name");
        }
    }
    if (element.hasAttributeNS(KoXmlNS::presentation, "style-name")) {
        style = context.odfLoadingContext().stylesReader().findStyle(
                    element.attributeNS(KoXmlNS::presentation, "style-name"), "presentation",
                    context.odfLoadingContext().useStylesAutoStyles());
        if (!style) {
            kDebug(32500) << "presentation style not found:" << element.attributeNS(KoXmlNS::presentation, "style-name");
        }
    }

    if (style) {
        // graphic styles don't support inheritance yet therefor some additional work is needed here.
        QList<KoParagraphStyle *> paragraphStyles;
        while (style) {
            KoParagraphStyle *pStyle = new KoParagraphStyle();
            pStyle->loadOdf(style, context);
            if (!paragraphStyles.isEmpty()) {
                paragraphStyles.last()->setParentStyle(pStyle);
            }
            paragraphStyles.append(pStyle);
            QString family = style->attributeNS(KoXmlNS::style, "family", "paragraph");
            style = context.odfLoadingContext().stylesReader().findStyle(
                    style->attributeNS(KoXmlNS::style, "parent-style-name"), family.toLocal8Bit().constData(),
                    context.odfLoadingContext().useStylesAutoStyles());
            if (!style && paragraphStyles.size() == 1) {
                style = context.odfLoadingContext().stylesReader().findStyle(
                        "standard", family.toLocal8Bit().constData(),
                        context.odfLoadingContext().useStylesAutoStyles());
            }
        }

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
    }
}

void KoTextShapeData::saveStyle(KoGenStyle &style, KoShapeSavingContext &context) const
{
    Q_D(const KoTextShapeData);
    if (d->paragraphStyle) {
        d->paragraphStyle->saveOdf(style, context);
    }
}

#include <KoTextShapeData.moc>
