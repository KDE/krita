/* This file is part of the KDE project
 * Copyright (C) 2011 Jan Hambrecht <jaham@gmx.net>
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

#ifndef SVGSHAPEFACTORY_H
#define SVGSHAPEFACTORY_H

#include "kritaflake_export.h"
#include "KoShapeFactoryBase.h"

/// Use this shape factory to load embedded svg files from odf
class KRITAFLAKE_EXPORT SvgShapeFactory : public KoShapeFactoryBase
{
public:
    SvgShapeFactory();
    ~SvgShapeFactory() override;

    // reimplemented from KoShapeFactoryBase
    bool supports(const KoXmlElement &element, KoShapeLoadingContext &context) const override;
    // reimplemented from KoShapeFactoryBase
    KoShape *createShapeFromOdf(const KoXmlElement &element, KoShapeLoadingContext &context) override;

    static int calculateZIndex(const KoXmlElement &element, KoShapeLoadingContext &context);
    static KoShape *createShapeFromSvgDirect(const KoXmlElement &root, const QRectF &boundsInPixels, const qreal pixelsPerInch, const qreal forcedFontSizeResolution, int zIndex, KoShapeLoadingContext &context, QSizeF *fragmentSize = 0);
};

#endif // SVGSHAPEFACTORY_H
