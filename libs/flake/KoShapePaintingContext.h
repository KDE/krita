/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2011 C. Boemann <cbo@boemann.dk>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KOSHAPEPAINTINGCONTEXT_H
#define KOSHAPEPAINTINGCONTEXT_H

#include "kritaflake_export.h"

class KoCanvasBase;

/**
 * Context passed to shapes during painting.
 */
class KRITAFLAKE_EXPORT KoShapePaintingContext
{
public:
    KoShapePaintingContext();
    KoShapePaintingContext(KoCanvasBase *canvas, bool forPrint);

    ~KoShapePaintingContext();

    bool showFormattingCharacters;
    bool showTextShapeOutlines;
    bool showTableBorders;
    bool showSectionBounds;
    bool showSpellChecking;
    bool showSelections;
    bool showInlineObjectVisualization;
    bool showAnnotations;
};

#endif /* KOSHAPEPAINTINGCONTEXT_H */
