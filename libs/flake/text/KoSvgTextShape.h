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

    static const QString &defaultPlaceholderText();

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



    /// return position to the left;
    /// visual indicates whether to use logical ('previous') or visual left (depending on direction).
    int posLeft(int pos, bool visual = false);
    /// return position to the right;
    /// visual indicates whether to use logical ('next') or visual right (depending on direction).
    int posRight(int pos, bool visual = false);
    /// return position above.
    int posUp(int pos, bool visual = false);
    /// return position below.
    int posDown(int pos, bool visual = false);

    /**
     * @brief lineStart
     * return the 'line start' for this pos. This uses anchored chunks,
     * so each absolute x, y posititioning will be considered a line-start.
     * @param pos -- the cursor pos for which to find the line start.
     * @return the line start.
     */
    int lineStart(int pos);
    /**
     * @brief lineEnd
     * return the 'line end' for this pos. This uses anchored chunks,
     * so each absolute x, y posititioning will be considered a line-start.
     * @param pos -- the cursor pos for which to find the line end.
     * @return the line end.
     */
    int lineEnd(int pos);

    /**
     * @brief wordLeft
     * return the cursorpos for the word left or the extreme of the line.
     * @param pos the position.
     * @param visual whether to flip for rtl.
     * @return the first pos before a wordbreak at the left.
     */
    int wordLeft(int pos, bool visual = false);
    /**
     * @brief wordRight
     * return the cursorpos for the word right or the extreme of the line.
     * @param pos the position.
     * @param visual whether to flip for rtl.
     * @return the first word break at the right.
     */
    int wordRight(int pos, bool visual = false);

    /**
     * @brief nextIndex
     * Return the first cursor position with a
     * higher string index.
     * @param pos the cursor position.
     * @return the first pos with a higher cluster
     * or same cluster and higher offset.
     */
    int nextIndex(int pos);

    /**
     * @brief previousIndex
     * Return the first pos which has a lower string index.
     * @param pos the cursor position.
     * @return the first cursor position with a lower index.
     */
    int previousIndex(int pos);

    /**
     * @brief nextLine
     * get a position on the next line for this position.
     * @param pos -- cursor position.
     * @return the pos on the next line;
     */
    int nextLine(int pos);
    /**
     * @brief previousLine
     * get a position on the previous line for this position.
     * @param pos -- cursor position.
     * @return the pos on the previous line;
     */
    int previousLine(int pos);

    /**
     * @brief wordEnd
     * return the pos of the first wordbreak.
     * @param pos -- cursor position.
     * @return the first wordbreak or line end.
     */
    int wordEnd(int pos);
    /**
     * @brief wordStart
     * return the first pos before a wordbreak in the start direction.
     * @param pos -- cursor position
     * @return the first position before a wordbreak or the line start.
     */
    int wordStart(int pos);

    /**
     * @brief cursorForPos
     * returns the QPainterPath associated with this cursorPosition.
     * @param pos the cursor Position
     * @param bidiFlagSize -- size of the bidirectional indicator.
     * @return a path to draw a cursor with.
     */
    QPainterPath cursorForPos(int pos, QLineF &caret, QColor &color, double bidiFlagSize = 1.0);

    /**
     * @brief selectionBoxes
     * returns all selection boxes for a given range.
     * Range will be normalized internally.
     * @param pos -- the main cursor pos.
     * @param anchor -- the anchor from which the selection is calculated.
     * @return a winding-fill style QPainterPath will all boxes added as subpaths.
     */
    QPainterPath selectionBoxes(int pos, int anchor);

    QPainterPath underlines(int pos, int anchor,
                            KoSvgText::TextDecorations decor,
                            KoSvgText::TextDecorationStyle style,
                            qreal minimum, bool thick);

    /**
     * @brief posForPoint
     * Finds the closest cursor position for the given point in shape coordinates.
     * @param point
     * @param start -- optional start position;
     * @param end -- optional end position;
     * @param overlaps -- optional bool that is set if the point overlaps any glyph box.
     * @return the closest cursor position.
     */
    int posForPoint(QPointF point, int start = -1, int end = -1, bool *overlaps = nullptr);

    /**
     * @brief posForPointLineSensitive
     * When clicking on an empty space in a wrapped text, it is preferrable to have the
     * caret be at the end of the line visually associated with that empty space than
     * the positions above or below that might be closer.
     *
     * Because SVG has several situations where there's no real lines, we first test
     * for the closest cursorpos and also whether there's an actual overlap with a glyph.
     * If there's no such overlap, we test against whether there's an anchored chunk in
     * the inline direction, and if so search the line resulting from that. If not, we return
     * the closest pos.
     * @param point the point in shape coordinates.
     * @return the closest pos, taking into account any line starts or ends.
     */
    int posForPointLineSensitive(QPointF point);
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
     * @brief initialTextPosition
     * Returns the initial text position as per SVG algorithm.
     * The eventual result of this can include transforms or repositioning due to text shapes.
     * @return the initial text position in shape coordinates.
     */
    QPointF initialTextPosition() const;

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
     * Where insert text explicitely uses a cursorposition,
     * remove text uses a string index. It will modify these values
     * so that...
     * - Whole code points are deleted at any time, avoiding
     *   no dangling surrogates.
     * - Graphemes don't end with Zero-width-joiners, as that can lead
     *   to the grapheme merging with the next.
     * - Variation selectors are deleted along their base.
     * - regional sequences are deleted in pairs.
     * @param index - index (not cursorpos!) of the start position.
     * @param length - total length of text to remove.
     * @return whether it was successful.
     */
    bool removeText(int &index, int &length);

    KoSvgTextProperties propertiesForPos(int pos);
    void setPropertiesAtPos(int pos, KoSvgTextProperties properties);

    /**
     * @brief plainText
     * plain text of all text inside this text shape,
     * without the bidi controls or any transforms.
     * @return a string of plain text.
     */
    QString plainText();

    /**
     * @brief writingMode
     * There's a number of places we need to check the writing mode to provide proper controls.
     * @return the writing mode of this text.
     */
    KoSvgText::WritingMode writingMode() const;

    /// ShapeChangeListener so we can inform any text cursors that the cursor needs updating.
    struct KRITAFLAKE_EXPORT TextCursorChangeListener : public ShapeChangeListener {
        void notifyShapeChanged(ChangeType type, KoShape *shape) override;
        virtual void notifyCursorPosChanged(int pos, int anchor) = 0;
    };
    /// Notify that the cursor position has changed.
    void notifyCursorPosChanged(int pos, int anchor);

protected:
    /**
     * Show if the shape is a root of the text hierarchy. Always true for
     * KoSvgTextShape and always false for KoSvgTextChunkShape
     */
    bool isRootTextNode() const override;

    void shapeChanged(ChangeType type, KoShape *shape) override;

private:
    /**
     * @brief nextPos
     * get the next position.
     * @param pos -- cursor position.
     * @return the next pos;
     */
    int nextPos(int pos, bool visual);

    /**
     * @brief previousPos
     * get the previous position.
     * @param pos -- cursor position.
     * @return the previous pos;
     */
    int previousPos(int pos, bool visual);

    /**
     * @brief defaultCursorShape
     * This returns a default cursor shape for when there's no text inside the text shape.
     * @return a QPainterPath for a cursor.
     */
    QPainterPath defaultCursorShape();

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
