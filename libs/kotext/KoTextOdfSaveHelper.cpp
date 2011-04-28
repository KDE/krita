/* This file is part of the KDE project
 * Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
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
#include <KoTextShapeDataBase.h>
#include "KoTextWriter.h"
#include <KoGenChanges.h>
#include <KoShapeSavingContext.h>

#include <opendocument/KoTextSharedSavingData.h>
#include "KoTextSopranoRdfModel_p.h"

struct KoTextOdfSaveHelper::Private {
    Private(KoTextShapeDataBase *shapeData, int from, int to)
        : shapeData(shapeData),
        from(from),
        to(to),
        rdfModel(0)
    {
    }

    KoShapeSavingContext *context;
    KoTextShapeDataBase *shapeData;

    int from;
    int to;

    const Soprano::Model *rdfModel; //< This is so cut/paste can serialize the relevant RDF to the clipboard
};


KoTextOdfSaveHelper::KoTextOdfSaveHelper(KoTextShapeDataBase * shapeData, int from, int to)
        : d(new Private(shapeData, from, to))
{
}

KoTextOdfSaveHelper::~KoTextOdfSaveHelper()
{
    delete d;
}

bool KoTextOdfSaveHelper::writeBody()
{
    if (d->to < d->from)
        qSwap(d->to, d->from);

    KoXmlWriter & bodyWriter = d->context->xmlWriter();
    bodyWriter.startElement("office:body");
    bodyWriter.startElement(KoOdf::bodyContentElement(KoOdf::Text, true));

//    d->shapeData->saveOdf(*d->context, 0, d->from, d->to);
    KoTextWriter writer(*d->context, 0);
    writer.write(d->shapeData->document(), d->from, d->to);

    bodyWriter.endElement(); // office:element
    bodyWriter.endElement(); // office:body
    return true;
}

KoShapeSavingContext * KoTextOdfSaveHelper::context(KoXmlWriter * bodyWriter, KoGenStyles & mainStyles, KoEmbeddedDocumentSaver & embeddedSaver)
{
//    Q_ASSERT(d->context == 0);

    d->context = new KoShapeSavingContext(*bodyWriter, mainStyles, embeddedSaver);
    return d->context;
}

void KoTextOdfSaveHelper::setRdfModel(const Soprano::Model *m)
{
    d->rdfModel = m;
}

const Soprano::Model *KoTextOdfSaveHelper::rdfModel() const
{
    return d->rdfModel;
}

