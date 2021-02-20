/*
 *  SPDX-FileCopyrightText: 2009 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_SHAPE_TOOL_HELPER_H
#define KIS_SHAPE_TOOL_HELPER_H

#include <kritaui_export.h>

#include <QRectF>

class KoShape;

/**
 * KisShapeToolHelper provides shapes and fallback shapes for shape based tools
 */
class KRITAUI_EXPORT KisShapeToolHelper
{
public:
    static KoShape* createRectangleShape(const QRectF& rect, qreal roundCornersX, qreal roundCornersY);

    static KoShape* createEllipseShape(const QRectF& rect);


};


#endif
