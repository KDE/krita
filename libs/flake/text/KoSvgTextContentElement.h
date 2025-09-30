/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KOSVGTEXTCONTENTELEMENT_H
#define KOSVGTEXTCONTENTELEMENT_H

#include "KoSvgText.h"
#include "KoSvgTextProperties.h"
#include "SvgLoadingContext.h"
#include "SvgSavingContext.h"

#include <kritaflake_export.h>
#include <QScopedPointer>

/**
 * @brief The KoSvgTextContentElement struct
 *
 * Replacing KoSvgTextChunkShape, KoSvgTextContentElement represents
 * a formatted span of text. This can be a SVG <text />, <tspan /> or
 * <textPath /> node. This struct is used to represent the internal data
 * of KoSvgTextShape.
 */

struct KRITAFLAKE_EXPORT KoSvgTextContentElement
{
public:
    KoSvgTextContentElement();
    KoSvgTextContentElement(const KoSvgTextContentElement &rhs)
        : properties(rhs.properties)
        , localTransformations(rhs.localTransformations)
        , textPathInfo(rhs.textPathInfo)
        , textPathId(rhs.textPathId)
        , textLength(rhs.textLength)
        , lengthAdjust(rhs.lengthAdjust)
        , textDecorations(rhs.textDecorations)
        , text(rhs.text)
        , finalResultIndex(rhs.finalResultIndex)
        , associatedOutline(rhs.associatedOutline)
    {

    }

    ~KoSvgTextContentElement() = default;

    /// The textProperties. This includes
    KoSvgTextProperties properties;

    /// Local SVG char transforms.
    QVector<KoSvgText::CharTransformation> localTransformations;

    /// Text path info for the text-on-path algorithm
    KoSvgText::TextOnPathInfo textPathInfo;

    /// The textpath's name, if any.
    QString textPathId;

    /// the value 'textLength' attribute of the associated dom element
    KoSvgText::AutoValue textLength;
    /// the value 'lengthAdjust' attribute of the associated dom element
    KoSvgText::LengthAdjust lengthAdjust = KoSvgText::LengthAdjustSpacing;

    /// Cached text decorations to be used by the painting function.
    QMap<KoSvgText::TextDecoration, QPainterPath> textDecorations;

    /// Plain text of the current node. Use insertText and removeText to manipulate it.
    QString text;

    /// Set during layout, finalResultIndex determines the size of the iterator
    /// on result after going over this contentElement. Once layout has been set
    /// this is less fiddly than using numChars which requires resolving properties.
    int finalResultIndex = -1;

    /// The associated outline. Currently only a bounding box.
    QPainterPath associatedOutline;

    /**
     * @brief loadSvg
     * load SVG style data into the current content element.
     * @param element -- xml element to load the data from.
     * @param context -- loading context.
     * @param rootNode -- whether this content element is a <text /> node. During text layout,
     * text properties are inherited dynamically. However, we can only start at the root text node,
     * while SVG can have properties that are set on the document root and inherit. Therefore, when
     * loading we inherit and resolve those properties dynamically for the root node only.
     * @return whether successful.
     */
    bool loadSvg(const QDomElement &element, SvgLoadingContext &context, bool rootNode = false);

    bool loadSvgTextNode(const QDomText &text, SvgLoadingContext &context);

    bool saveSvg(SvgSavingContext &context, bool rootText, bool saveText, QMap<QString, QString> shapeSpecificAttributes, KoShape *textPath = nullptr);

    /**
     * The number of characters contained in the currentChunk.
     * @param withControls this will enable the bidi controls to be
     * counted as well.
     */
    int numChars(bool withControls = false, KoSvgTextProperties resolvedProps = KoSvgTextProperties()) const;

    /**
     * Get the text with transformations applied.
     * @param positions the text positions which may have changed due the uppercase transform.
     */
    QString getTransformedString(QVector<QPair<int, int>> &positions, KoSvgTextProperties resolvedProps = KoSvgTextProperties()) const;

    /**
     * @brief insertText
     * @param start -- start index.
     * @param insertText -- text to insert.
     */
    void insertText(int start, QString insertText);
    /**
     * @brief removeText
     * removes text, @see KoSvgTextShape::removeText and CssUtils::removeText
     * @param start -- start index, may be modified.
     * @param length -- length of text to remove.
     */
    void removeText(int &start, int length);
};

#endif // KOSVGTEXTCONTENTELEMENT_H
