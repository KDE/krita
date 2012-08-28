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

#include "KoTextPaste.h"

#include <KoOdfReadStore.h>
#include <KoOdfLoadingContext.h>
#include <KoShapeLoadingContext.h>
#include <KoShapeBasedDocumentBase.h>
#include <KoShapeController.h>

#include "KoTextDocument.h"
#include "KoDocumentRdfBase.h"
#include "opendocument/KoTextLoader.h"

#include <kdebug.h>
#ifdef SHOULD_BUILD_RDF
#include "KoTextRdfCore.h"
#endif

class KoTextPaste::Private
{
public:
    Private(KoTextEditor *editor, KoShapeController *shapeController, QSharedPointer<Soprano::Model> _rdfModel)
        : editor(editor)
        , resourceManager(shapeController->resourceManager())
        , rdfModel(_rdfModel)
    {
    }

    KoTextEditor *editor;
    KoDocumentResourceManager *resourceManager;
    QSharedPointer<Soprano::Model> rdfModel;
};

KoTextPaste::KoTextPaste(KoTextEditor *editor, KoShapeController *shapeController, QSharedPointer<Soprano::Model> rdfModel)
        : d(new Private(editor, shapeController, rdfModel))
{
}

KoTextPaste::~KoTextPaste()
{
    delete d;
}

bool KoTextPaste::process(const KoXmlElement &body, KoOdfReadStore &odfStore)
{
    bool ok = true;
    KoOdfLoadingContext loadingContext(odfStore.styles(), odfStore.store());
    KoShapeLoadingContext context(loadingContext, d->resourceManager);

    KoTextLoader loader(context);

    kDebug(30015) << "text paste";
    // load the paste directly into the editor's cursor -- which breaks encapsulation
    loader.loadBody(body, *d->editor->cursor());   // now let's load the body from the ODF KoXmlElement.

#ifdef SHOULD_BUILD_RDF
    kDebug(30015) << "text paste, rdf handling" << d->rdfModel;
    // RDF: Grab RDF metadata from ODF file if present & load it into rdfModel
    if (d->rdfModel)
    {
        QSharedPointer<Soprano::Model> tmpmodel(Soprano::createModel());
        ok = KoTextRdfCore::loadManifest(odfStore.store(), tmpmodel);
        kDebug(30015) << "ok:" << ok << " tmpmodel.sz:" << tmpmodel->statementCount();
        kDebug(30015) << "existing rdf model.sz:" << d->rdfModel->statementCount();
#ifndef NDEBUG
        KoTextRdfCore::dumpModel("RDF from C+P", tmpmodel);
#endif
        d->rdfModel->addStatements(tmpmodel->listStatements().allElements());
        kDebug(30015) << "done... existing rdf model.sz:" << d->rdfModel->statementCount();
#ifndef NDEBUG
        KoTextRdfCore::dumpModel("Imported RDF after C+P", d->rdfModel);
#endif
    }
#endif

    return ok;
}
