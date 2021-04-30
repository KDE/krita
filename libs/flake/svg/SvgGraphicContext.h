/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2003, 2005 Rob Buis <buis@kde.org>
 * SPDX-FileCopyrightText: 2007, 2009 Jan Hambrecht <jaham@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef SVGGRAPHICCONTEXT_H
#define SVGGRAPHICCONTEXT_H

#include "kritaflake_export.h"
#include <KoShapeStroke.h>
#include <QFont>
#include <QTransform>
#include <text/KoSvgTextProperties.h>

class KRITAFLAKE_EXPORT SvgGraphicsContext
{
public:
    // Fill/stroke styles
    enum StyleType {
        None,     ///< no style
        Solid,    ///< solid style
        Complex   ///< gradient or pattern style
    };

    SvgGraphicsContext();
    SvgGraphicsContext(const SvgGraphicsContext &gc);

    void workaroundClearInheritedFillProperties();

    StyleType     fillType  {Solid};  ///< the current fill type
    Qt::FillRule  fillRule  {Qt::WindingFill};  ///< the current fill rule
    QColor        fillColor {QColor(Qt::black)}; ///< the current fill color. Default is black fill as per svg spec
    QString       fillId;    ///< the current fill id (used for gradient/pattern fills)

    StyleType     strokeType {None};///< the current stroke type
    QString       strokeId;  ///< the current stroke id (used for gradient strokes)
    KoShapeStrokeSP stroke;    ///< the current stroke

    QString filterId;       ///< the current filter id
    QString clipPathId;     ///< the current clip path id
    QString clipMaskId;     ///< the current clip mask id
    Qt::FillRule clipRule {Qt::WindingFill};  ///< the current clip rule
    qreal opacity {1.0};    ///< the shapes opacity

    QTransform matrix;      ///< the current transformation matrix
    QFont   font;           ///< the current font
    QStringList fontFamiliesList; ///< the full list of all the families to search glyphs in
    QColor  currentColor {Qt::black};   ///< the current color
    QString xmlBaseDir;     ///< the current base directory (used for loading external content)
    bool preserveWhitespace {false}; ///< preserve whitespace in element text

    QRectF currentBoundingBox; ///< the current bound box used for bounding box units
    bool   forcePercentage {false}; ///< force parsing coordinates/length as percentages of currentBoundbox
    QTransform viewboxTransform; ///< view box transformation

    bool display {true};           ///< controls display of shape
    bool visible {true};           ///< controls visibility of the shape (inherited)
    bool isResolutionFrame {false};
    qreal pixelsPerInch {72.0};    ///< controls the resolution of the image raster

    QString markerStartId;
    QString markerMidId;
    QString markerEndId;

    bool autoFillMarkers {false};

    KoSvgTextProperties textProperties;
private:
    SvgGraphicsContext& operator=(const SvgGraphicsContext &gc) = default; ///< used by copy constructor, shouldn't be public
};

#endif // SVGGRAPHICCONTEXT_H
