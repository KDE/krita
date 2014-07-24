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

#ifndef KOSHAPEPAINTINGCONTEXT_H
#define KOSHAPEPAINTINGCONTEXT_H

#include <QSet>
#include <QString>
#include <QVariant>

#include "flake_export.h"

class KoCanvasBase;

/**
 * Context passed to shapes during painting.
 */
class FLAKE_EXPORT KoShapePaintingContext
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
