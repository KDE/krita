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

#include "KoTextPaste.h"

#include <KoOdfReadStore.h>
#include <KoOdfLoadingContext.h>
#include <KoCanvasBase.h>
#include <KoShapeLoadingContext.h>
#include <KoShapeControllerBase.h>
#include <KoShapeController.h>
#include "KoTextShapeData.h"
#include "opendocument/KoTextLoader.h"

class KoTextPaste::Private {
public:
    Private(KoTextShapeData * shapeData, QTextCursor & cursor, KoCanvasBase * canvas)
            : shapeData(shapeData)
            , cursor(cursor)
            , canvas(canvas) {}

    KoTextShapeData * shapeData;
    QTextCursor & cursor;
    KoCanvasBase * canvas;
};

KoTextPaste::KoTextPaste(KoTextShapeData * shapeData, QTextCursor & cursor, KoCanvasBase * canvas)
        : d(new Private(shapeData, cursor, canvas))
{
}

KoTextPaste::~KoTextPaste()
{
    delete d;
}

bool KoTextPaste::process(const KoXmlElement & body, KoOdfReadStore & odfStore)
{
    KoOdfLoadingContext loadingContext(odfStore.styles(), odfStore.store());
    KoShapeLoadingContext context(loadingContext, d->canvas->shapeController()->resourceManager());

    KoTextLoader loader(context);

    loader.loadBody(body, d->cursor);   // now let's load the body from the ODF KoXmlElement.

    return true;
}
