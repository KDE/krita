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

#include <QFlags>

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

    enum class DebugElement {
        CharBbox = 1 << 0,
        LineBox = 1 << 1,
    };
    Q_DECLARE_FLAGS(DebugElements, DebugElement);
    void paintDebug(QPainter &painter, DebugElements elements) const;

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

    /**
     * @brief nextPos
     * get the next position.
     * @param pos -- cursor position.
     * @return the next pos;
     */
    int nextPos(int pos);

    /**
     * @brief previousPos
     * get the previous position.
     * @param pos -- cursor position.
     * @return the previous pos;
     */
    int previousPos(int pos);

    /**
     * @brief cursorForPos
     * returns the QPainterPath associated with this cursorPosition.
     * @param pos the cursor Position
     * @return a path to draw a cursor with.
     */
    QPainterPath cursorForPos(int pos);

    /**
     * @brief selectionBoxes
     * returns all selection boxes for a given range.
     * Range will be normalized internally.
     * @param pos -- the main cursor pos.
     * @param anchor -- the anchor from which the selection is calculated.
     * @return a winding-fill style QPainterPath will all boxes added as subpaths.
     */
    QPainterPath selectionBoxes(int pos, int anchor);

    /**
     * @brief posForPoint
     * Finds the closest cursor position for the given point in shape coordinates.
     * @param point
     * @return the closest cursor position.
     */
    int posForPoint(QPointF point);
    /**
     * @brief posForIndex
     * Get the cursor position for a given index in a string.
     * @param index -- index in the string.
     * @param firstIndex -- whether to return for the first instance of the index.
     * @param skipSynthetic -- whether to skip over synthetic cursorPositions.
     * @return the cursor -- position for an index.
     */
    int posForIndex(int index, bool firstIndex = false, bool skipSynthetic = false);
    /**
     * @brief indexForPos
     * get the string index for a given cursor position.
     * @param pos the cursor position.
     * @return the index in the string.
     */
    int indexForPos(int pos);

    /**
     * @brief insertText
     * Insert a text somewhere in the KoTextShape.
     * @param pos the cursor position.
     * @param text the text to insert.
     * @return whether it was succesful in inserting text.
     */
    bool insertText(int pos, QString text);

    /**
     * @brief removeText
     * @param pos
     * @param length
     * @return
     */
    bool removeText(int pos, int length);

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

Q_DECLARE_OPERATORS_FOR_FLAGS(KoSvgTextShape::DebugElements)

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
