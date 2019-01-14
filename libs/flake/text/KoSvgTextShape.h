/*
 *  Copyright (c) 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KOSVGTEXTSHAPE_H
#define KOSVGTEXTSHAPE_H

#include "kritaflake_export.h"

#include <KoShapeFactoryBase.h>
#include <KoSvgTextChunkShape.h>
#include <SvgShape.h>

class KoSvgTextProperties;
class KoSvgTextShapePrivate;

#define KoSvgTextShape_SHAPEID "KoSvgTextShapeID"
/**
 * KoSvgTextShape is a root chunk of the \<text\> element subtree.
 */
class KRITAFLAKE_EXPORT KoSvgTextShape : public KoSvgTextChunkShape
{
public:
    KoSvgTextShape();
    KoSvgTextShape(const KoSvgTextShape &rhs);
    ~KoSvgTextShape() override;

    KoShape* cloneShape() const override;

    void paintComponent(QPainter &painter, const KoViewConverter &converter, KoShapePaintingContext &paintContext) override;
    void paintStroke(QPainter &painter, const KoViewConverter &converter, KoShapePaintingContext &paintContext) override;

    /**
     * Reset the text shape into initial shape, removing all the child shapes
     * and precalculated layouts. This method is used by text-updating code to
     * upload the updated text into shape. The upload code first calls
     * resetTextShape() and then adds new children.
     */
    void resetTextShape() override;

    /**
     * Create a new text layout for the current content of the text shape
     * chunks tree. The user should always call relayout() after every change
     * in the text shapes hierarchy.
     */
    void relayout();

    QPainterPath textOutline();

protected:
    /**
     * Show if the shape is a root of the text hierarchy. Always true for
     * KoSvgTextShape and always false for KoSvgTextChunkShape
     */
    bool isRootTextNode() const override;

    void shapeChanged(ChangeType type, KoShape *shape) override;

private:
    Q_DECLARE_PRIVATE(KoSvgTextShape)
};

class KoSvgTextShapeFactory : public KoShapeFactoryBase
{

public:
    /// constructor
    KoSvgTextShapeFactory();
    ~KoSvgTextShapeFactory() {}

    KoShape *createDefaultShape(KoDocumentResourceManager *documentResources = 0) const override;

    KoShape *createShape(const KoProperties *params, KoDocumentResourceManager *documentResources = 0) const override;
    /// Reimplemented
    bool supports(const KoXmlElement &e, KoShapeLoadingContext &context) const override;
};



#endif // KOSVGTEXTSHAPE_H
