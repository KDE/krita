/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KOSVGTEXTSHAPE_H
#define KOSVGTEXTSHAPE_H

#include "kritaflake_export.h"

#include <KoShapeFactoryBase.h>
#include <KoSvgTextChunkShape.h>
#include <SvgShape.h>

class KoSvgTextProperties;

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

    enum TextRendering {
        Auto,
        OptimizeSpeed,
        OptimizeLegibility,
        GeometricPrecision
    };

    KoShape* cloneShape() const override;

    void paintComponent(QPainter &painter) const override;
    void paintStroke(QPainter &painter) const override;

    void paintDebug(QPainter &painter) const;

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
    void relayout() const;

    QList<KoShape *> textOutline() const;

    /**
     * @brief setTextRenderingFromString
     * Set the quality of the text rendering.
     *
     * Based on: https://www.w3.org/TR/SVG11/painting.html#TextRenderingProperty
     *
     * "auto" is the default.
     * "optimizeSpeed" will turn off anti-aliasing.
     * "optimizeLegibility" will have us load the hinting metrics.
     * "geometricPrecision" will not load the hinting metrics.
     *
     * @param textRendering the textRendering to use.
     */
    void setTextRenderingFromString(const QString &textRendering);

    /**
     * @brief textRenderingString
     * @see setTextRenderingFromString
     * @return the Text Rendering type as a string.
     */

    QString textRenderingString() const override;

    /**
     * @brief setShapesInside
     * @param shapesInside the list of shapes to make up the content area.
     */
    void setShapesInside(QList<KoShape*> shapesInside);

    /**
     * @brief shapesInside
     * @return the list of shapes that make up the content area.
     */
    QList<KoShape*> shapesInside() const;

    /**
     * @brief setShapesSubtract
     * @param shapesSubtract the list of shapes that subtract from the wrapping area.
     */
    void setShapesSubtract(QList<KoShape*> shapesSubtract);

    /**
     * @brief shapesSubtract
     * @return list of subtract shapes.
     */
    QList<KoShape*> shapesSubtract() const;

    QMap<QString, QString> shapeTypeSpecificStyles(SvgSavingContext &context) const override;

    void setResolution(qreal xRes, qreal yRes) override;

protected:
    /**
     * Show if the shape is a root of the text hierarchy. Always true for
     * KoSvgTextShape and always false for KoSvgTextChunkShape
     */
    bool isRootTextNode() const override;

    void shapeChanged(ChangeType type, KoShape *shape) override;

private:
    class Private;
    QScopedPointer<Private> d;
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
    bool supports(const QDomElement &e, KoShapeLoadingContext &context) const override;
};



#endif // KOSVGTEXTSHAPE_H
