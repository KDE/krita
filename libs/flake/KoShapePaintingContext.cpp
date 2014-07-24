/* This file is part of the KDE project
   Copyright (C) 2011 C. Boemann <cbo@boemann.dk>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "KoShapePaintingContext.h"

#include "KoCanvasBase.h"
#include "KoCanvasResourceManager.h"

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
    KoCanvasResourceManager *rm = canvas->resourceManager();

    showFormattingCharacters = rm->boolResource(KoCanvasResourceManager::ShowFormattingCharacters);
    if (forPrint) {
        showTextShapeOutlines = false;
        showFormattingCharacters = false;
        showTableBorders = false;
        showSectionBounds = false;
        showInlineObjectVisualization = false;
    } else {
        showTextShapeOutlines = rm->boolResource(KoCanvasResourceManager::ShowTextShapeOutlines);
        showInlineObjectVisualization = rm->boolResource(KoCanvasResourceManager::ShowInlineObjectVisualization);
        if (rm->hasResource(KoCanvasResourceManager::ShowTableBorders)) {
            showTableBorders = rm->boolResource(KoCanvasResourceManager::ShowTableBorders);
        } else {
            showTableBorders = true;
        }
        if (rm->hasResource(KoCanvasResourceManager::ShowSectionBounds)) {
            showSectionBounds = rm->boolResource(KoCanvasResourceManager::ShowSectionBounds);
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
