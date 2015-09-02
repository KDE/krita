/* This file is part of the KDE project
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2011 Boudewijn Rempt <boud@valdyas.org>
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

#include "KoTextOdfSaveHelper.h"

#include <KoXmlWriter.h>
#include <KoOdf.h>
#include "KoTextWriter.h"
#include <KoShapeSavingContext.h>
#include <KoTextDocument.h>

#include <QTextDocument>

struct Q_DECL_HIDDEN KoTextOdfSaveHelper::Private {
    Private(const QTextDocument *document, int from, int to)
        : context(0)
        , document(document)
        , from(from)
        , to(to)
#ifdef SHOULD_BUILD_RDF
        , rdfModel(0)
#endif
    {
    }

    KoShapeSavingContext *context;
    const QTextDocument *document;

    int from;
    int to;

#ifdef SHOULD_BUILD_RDF
    QSharedPointer<Soprano::Model> rdfModel; //< This is so cut/paste can serialize the relevant RDF to the clipboard
#endif
};


KoTextOdfSaveHelper::KoTextOdfSaveHelper(const QTextDocument *document, int from, int to)
        : d(new Private(document, from, to))
{
}

KoTextOdfSaveHelper::~KoTextOdfSaveHelper()
{
    delete d;
}

bool KoTextOdfSaveHelper::writeBody()
{
    if (d->to < d->from) {
        qSwap(d->to, d->from);
    }
    Q_ASSERT(d->context);
    KoXmlWriter & bodyWriter = d->context->xmlWriter();
    bodyWriter.startElement("office:body");
    bodyWriter.startElement(KoOdf::bodyContentElement(KoOdf::Text, true));

    KoTextWriter writer(*d->context, 0);
    writer.write(d->document, d->from, d->to);

    bodyWriter.endElement(); // office:element
    bodyWriter.endElement(); // office:body
    return true;
}

KoShapeSavingContext * KoTextOdfSaveHelper::context(KoXmlWriter * bodyWriter,
                                                    KoGenStyles & mainStyles,
                                                    KoEmbeddedDocumentSaver & embeddedSaver)
{
    d->context = new KoShapeSavingContext(*bodyWriter, mainStyles, embeddedSaver);
    return d->context;
}

#ifdef SHOULD_BUILD_RDF
void KoTextOdfSaveHelper::setRdfModel(QSharedPointer<Soprano::Model> m)
{
    d->rdfModel = m;
}

QSharedPointer<Soprano::Model> KoTextOdfSaveHelper::rdfModel() const
{
    return d->rdfModel;
}
#endif

KoStyleManager *KoTextOdfSaveHelper::styleManager() const
{
    return KoTextDocument(d->document).styleManager();
}
