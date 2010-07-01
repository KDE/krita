/* This file is part of the KDE project
 * Copyright (C) 2006, 2009-2010 Thomas Zander <zander@kde.org>
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
#include "KoTextDocument.h"
#include "KoTextEditor.h"
#include "KoTextDocumentLayout.h"
#include "styles/KoStyleManager.h"
#include "styles/KoParagraphStyle.h"

#include <KDebug>
#include <QUrl>
#include <QTextDocument>
#include <QTextBlock>
#include <QMetaObject>
#include <QTextCursor>

#include <KoGenStyle.h>
#include <KoGenStyles.h>
#include <KoShapeLoadingContext.h>
#include <KoShapeSavingContext.h>
#include <rdf/KoDocumentRdfBase.h>

#include <KoXmlWriter.h>

#include "KoTextPage.h"

#include "opendocument/KoTextLoader.h"
#include "opendocument/KoTextWriter.h"

#include "KoTextDebug.h"

class KoTextShapeDataPrivate
{
public:
    KoTextShapeDataPrivate()
            : document(0),
            ownsDocument(true),
            dirty(true),
            offset(0.0),
            position(-1),
            endPosition(-1),
            direction(KoText::AutoDirection),
            textpage(0),
            textAlignment(Qt::AlignLeft | Qt::AlignTop)
    {
    }

    QTextDocument *document;
    bool ownsDocument, dirty;
    qreal offset;
    int position, endPosition;
    KoInsets margins;
    KoText::Direction direction;
    KoTextPage *textpage;
    Qt::Alignment textAlignment;
};


KoTextShapeData::KoTextShapeData()
        : d(new KoTextShapeDataPrivate())
{
    setDocument(new QTextDocument, true);
}

KoTextShapeData::~KoTextShapeData()
{
    delete d->textpage;
    if (d->ownsDocument)
        delete d->document;
    delete d;
}

void KoTextShapeData::setDocument(QTextDocument *document, bool transferOwnership)
{
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

    if (d->document->isEmpty()) { // apply app default style for first parag
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

QTextDocument *KoTextShapeData::document()
{
    return d->document;
}

qreal KoTextShapeData::documentOffset() const
{
    return d->offset;
}

void KoTextShapeData::setDocumentOffset(qreal offset)
{
    d->offset = offset;
}

int KoTextShapeData::position() const
{
    return d->position;
}

void KoTextShapeData::setPosition(int position)
{
    d->position = position;
}

int KoTextShapeData::endPosition() const
{
    return d->endPosition;
}

void KoTextShapeData::setEndPosition(int position)
{
    d->endPosition = position;
}

void KoTextShapeData::foul()
{
    d->dirty = true;
}

void KoTextShapeData::wipe()
{
    d->dirty = false;
}

bool KoTextShapeData::isDirty() const
{
    return d->dirty;
}

void KoTextShapeData::fireResizeEvent()
{
    emit relayout();
}

void KoTextShapeData::setShapeMargins(const KoInsets &margins)
{
    d->margins = margins;
}

KoInsets KoTextShapeData::shapeMargins() const
{
    return d->margins;
}

void KoTextShapeData::setPageDirection(KoText::Direction direction)
{
    d->direction = direction;
}

KoText::Direction KoTextShapeData::pageDirection() const
{
    return d->direction;
}

void KoTextShapeData::setPage(KoTextPage *textpage)
{
    delete d->textpage;
    d->textpage = textpage;
}

KoTextPage* KoTextShapeData::page() const
{
    return d->textpage;
}

bool KoTextShapeData::loadOdf(const KoXmlElement &element, KoShapeLoadingContext &context, KoDocumentRdfBase *rdfData, KoShape *shape)
{
    KoTextLoader loader(context, rdfData, shape);

    QTextCursor cursor(document());
    loader.loadBody(element, cursor);   // now let's load the body from the ODF KoXmlElement.
    KoTextEditor *editor = KoTextDocument(document()).textEditor();
    if (editor) // at one point we have to get the position from the odf doc instead.
        editor->setPosition(0);
    editor->finishedLoading();

    return true;
}

void KoTextShapeData::saveOdf(KoShapeSavingContext &context, KoDocumentRdfBase *rdfData, int from, int to) const
{
    KoTextWriter writer(context, rdfData);
    writer.write(d->document, from, to);
}

void KoTextShapeData::relayoutFor(KoTextPage &textPage)
{
    KoTextDocumentLayout *layout = qobject_cast<KoTextDocumentLayout*>(d->document->documentLayout());
    if (layout == 0)
        return;
    KoTextPage *oldPage = d->textpage;
    d->dirty = true;
    d->textpage = &textPage;
    layout->interruptLayout();
    layout->layout();
    d->textpage = oldPage;
}

void KoTextShapeData::setVerticalAlignment(Qt::Alignment alignment)
{
    d->textAlignment = (d->textAlignment & Qt::AlignHorizontal_Mask)
        | (alignment & Qt::AlignVertical_Mask);
}

Qt::Alignment KoTextShapeData::verticalAlignment() const
{
    return d->textAlignment & Qt::AlignVertical_Mask;
}

#include <KoTextShapeData.moc>
