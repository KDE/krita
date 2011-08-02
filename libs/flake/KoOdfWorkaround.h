/* This file is part of the KDE project
   Copyright (C) 2009 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2011 Jan Hambrecht <jaham@gmx.net>
   Copyright (C) 2011 Lukáš Tvrdý <lukas.tvrdy@ixonos.com>

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

#ifndef KOODFWORKAROUND_H
#define KOODFWORKAROUND_H

#include "flake_export.h"
#include "KoTextShapeDataBase.h"
#include <qnamespace.h>

class KoXmlElement;
class KoShape;
class KoShapeLoadingContext;
class QPen;
class QColor;
class QString;
class KoColorBackground;

/**
 * This class should contain all workarounds to correct problems with different ODF
 * implementations. If you need to access application specific things please create a
 * new namespace in the application you need it in
 * All calls to methods of this class should be wrapped into ifndefs like e.g.
 *
 * @code
 * #ifndef NWORKAROUND_ODF_BUGS
 *     KoOdfWorkaround::fixPenWidth(pen, context);
 * #endif
 * @endcode
 */
namespace KoOdfWorkaround
{
    /**
     * OpenOffice handles a line with the width of 0 as a cosmetic line but in svg it makes the line invisible.
     * To show it in calligra use a very small line width. However this is not a cosmetic line.
     */
    FLAKE_EXPORT void fixPenWidth(QPen &pen, KoShapeLoadingContext &context);

    /**
     * OpenOffice < 3.0 does not store the draw:enhanced-path for draw:type="ellipse"
     * Add the path needed for the ellipse
     */
    FLAKE_EXPORT void fixEnhancedPath(QString &path, const KoXmlElement &element, KoShapeLoadingContext &context);

    /**
     * OpenOffice interchanges the position coordinates for polar handles.
     * According to the specification the first coordinate is the angle, the
     * second coordinates is the radius. OpenOffice does it the other way around.
     */
    FLAKE_EXPORT void fixEnhancedPathPolarHandlePosition(QString &position, const KoXmlElement &element, KoShapeLoadingContext &context);

    FLAKE_EXPORT bool   fixMissingStroke(QPen &pen, const KoXmlElement &element, KoShapeLoadingContext &context, const KoShape *shape = 0);
    FLAKE_EXPORT QColor fixMissingFillColor(const KoXmlElement &element, KoShapeLoadingContext &context);
    FLAKE_EXPORT bool   fixMissingStyle_DisplayLabel(const KoXmlElement &element, KoShapeLoadingContext &context);

    FLAKE_EXPORT KoColorBackground *fixBackgroundColor(const KoShape *shape, KoShapeLoadingContext &context);

    /**
     * Old versions of ooimpress does not set the placeholder for shapes that should have it set
     * See open office issue http://www.openoffice.org/issues/show_bug.cgi?id=96406
     * And kde bug https://bugs.kde.org/show_bug.cgi?id=185354
     */
    FLAKE_EXPORT void setFixPresentationPlaceholder(bool fix, KoShapeLoadingContext &context);
    FLAKE_EXPORT bool fixPresentationPlaceholder();
    FLAKE_EXPORT void fixPresentationPlaceholder(KoShape *shape);

    /**
     * OpenOffice and LibreOffice save gluepoint positions wrong when no align is specified.
     * According to the specification for the above situation, the position should be saved
     * as percent values relative to the shapes center point. OpenOffice seems to write
     * these percent values converted to length units, where the millimeter value corresponds
     * to the correct percent value (i.e. -5cm = -50mm = -50%).
     */
    FLAKE_EXPORT void fixGluePointPosition(QString &positionString, KoShapeLoadingContext &context);

    /**
     * OpenOffice and LibreOffice does not conform to the specification about default value
     * of the svg:fill-rule. If this attribute is missing, according the spec, the initial
     * value is nonzero, but OOo uses evenodd. Because we are conform to the spec, we need
     * to set what OOo display.
     * See http://www.w3.org/TR/SVG/painting.html#FillRuleProperty
     */
    FLAKE_EXPORT void fixMissingFillRule(Qt::FillRule &fillRule, KoShapeLoadingContext &context);

    /**
     * OpenOffice resizes text shapes with autogrow in both directions. If the text box is saved to 
     * small the text will not fit and it needs to be adjusted during the first layout.
     * This methods returns true if we need to adjust the layout. The adjusting is handled at a different place.
     */
    FLAKE_EXPORT bool fixAutoGrow(KoTextShapeDataBase::ResizeMethod method, KoShapeLoadingContext &context);
}

#endif /* KOODFWORKAROUND_H */
