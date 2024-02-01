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

#include <QScopedPointer>

/**
 * @brief The KoSvgTextContentElement struct
 *
 * Replacing KoSvgTextChunkShape, KoSvgTextContentElement represents
 * a formatted span of text. This can be a SVG <text />, <tspan /> or
 * <textPath /> node. This struct is used to represent the internal data
 * of KoSvgTextShape.
 */

struct KoSvgTextContentElement
{
public:
    KoSvgTextContentElement();
    KoSvgTextContentElement(const KoSvgTextContentElement &rhs)
        : properties(rhs.properties)
        , localTransformations(rhs.localTransformations)
        , textPathInfo(rhs.textPathInfo)
        , textLength(rhs.textLength)
        , lengthAdjust(rhs.lengthAdjust)
        , textDecorationOffsets(rhs.textDecorationOffsets)
        , textDecorationWidths(rhs.textDecorationWidths)
        , textDecorations(rhs.textDecorations)
        , text(rhs.text)
        , associatedOutline(rhs.associatedOutline)
    {
        if (rhs.textPath) {
        textPath.reset(rhs.textPath.data()->cloneShape());
        }
    }

    ~KoSvgTextContentElement() = default;

    KoSvgTextProperties properties;
    QVector<KoSvgText::CharTransformation> localTransformations;

    KoSvgText::TextOnPathInfo textPathInfo;
    QScopedPointer<KoShape> textPath{nullptr};

    KoSvgText::AutoValue textLength;
    KoSvgText::LengthAdjust lengthAdjust = KoSvgText::LengthAdjustSpacing;

    QMap<KoSvgText::TextDecoration, qreal> textDecorationOffsets;
    QMap<KoSvgText::TextDecoration, qreal> textDecorationWidths;
    QMap<KoSvgText::TextDecoration, QPainterPath> textDecorations;

    QString text;

    QPainterPath associatedOutline;

    bool loadSvg(const QDomElement &element, SvgLoadingContext &context);

    bool loadSvgTextNode(const QDomText &text, SvgLoadingContext &context);

    bool saveSvg(SvgSavingContext &context, KoSvgTextProperties parentProperties, bool rootText, bool saveText, QMap<QString, QString> shapeSpecificAttributes);


    /**
     * The number of characters contained in the currentChunk.
     * @param withControls this will enable the bidi controls to be
     * counted as well.
     */
    int numChars(bool withControls) const;

    QString getTransformedString(QVector<QPair<int, int>> &positions) const;

    void insertText(int start, QString insertText);
    void removeText(int &start, int length);
};

#endif // KOSVGTEXTCONTENTELEMENT_H
