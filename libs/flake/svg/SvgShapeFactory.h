/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2011 Jan Hambrecht <jaham@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
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
    KoShape *createShapeFromXML(const KoXmlElement &element, KoShapeLoadingContext &context) override;

    static int calculateZIndex(const KoXmlElement &element, KoShapeLoadingContext &context);
    static KoShape *createShapeFromSvgDirect(const KoXmlElement &root, const QRectF &boundsInPixels, const qreal pixelsPerInch, const qreal forcedFontSizeResolution, int zIndex, KoShapeLoadingContext &context, QSizeF *fragmentSize = 0);
};

#endif // SVGSHAPEFACTORY_H
