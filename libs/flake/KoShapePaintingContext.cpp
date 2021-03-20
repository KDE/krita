/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2011 C. Boemann <cbo@boemann.dk>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "KoShapePaintingContext.h"

#include "KoCanvasBase.h"
#include "KoCanvasResourceProvider.h"

KoShapePaintingContext::KoShapePaintingContext()
    : showFormattingCharacters(false)
    , showTextShapeOutlines(false)
    , showTableBorders(true)
    , showSectionBounds(false)
    , showSpellChecking(false)
    , showSelections(true)
    , showInlineObjectVisualization(false)
    , showAnnotations(false)
{
}

KoShapePaintingContext::KoShapePaintingContext(KoCanvasBase *canvas, bool forPrint)
{
    KoCanvasResourceProvider *rm = canvas->resourceManager();

    showFormattingCharacters = rm->boolResource(KoCanvasResource::ShowFormattingCharacters);
    if (forPrint) {
        showTextShapeOutlines = false;
        showFormattingCharacters = false;
        showTableBorders = false;
        showSectionBounds = false;
        showInlineObjectVisualization = false;
    } else {
        showTextShapeOutlines = rm->boolResource(KoCanvasResource::ShowTextShapeOutlines);
        showInlineObjectVisualization = rm->boolResource(KoCanvasResource::ShowInlineObjectVisualization);
        if (rm->hasResource(KoCanvasResource::ShowTableBorders)) {
            showTableBorders = rm->boolResource(KoCanvasResource::ShowTableBorders);
        } else {
            showTableBorders = true;
        }
        if (rm->hasResource(KoCanvasResource::ShowSectionBounds)) {
            showSectionBounds = rm->boolResource(KoCanvasResource::ShowSectionBounds);
        } else {
            showSectionBounds = true;
        }
    }
    showSpellChecking = !forPrint;
    showSelections = !forPrint;
    showAnnotations = !forPrint;
}

KoShapePaintingContext::~KoShapePaintingContext()
{
}
