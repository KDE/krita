/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KoSvgTextChunkShape.h"

#include "KoPathShape.h"
#include "KoSvgText.h"
#include "KoSvgTextProperties.h"
#include <QSharedData>
#include <QTextCharFormat>

#include <KoShapeContainer_p.h>

class SvgGraphicsContext;

class KoSvgTextChunkShape::SharedData : public QSharedData
{
public:

    SharedData();
    SharedData(const SharedData &rhs);
    ~SharedData();

    KoSvgTextProperties properties;

    QVector<KoSvgText::CharTransformation> localTransformations;

    KoSvgText::TextOnPathInfo textPathInfo;
    KoShape *textPath{nullptr};

    KoSvgText::AutoValue textLength;
    KoSvgText::LengthAdjust lengthAdjust = KoSvgText::LengthAdjustSpacing;

    QMap<KoSvgText::TextDecoration, qreal> textDecorationOffsets;
    QMap<KoSvgText::TextDecoration, qreal> textDecorationWidths;
    QMap<KoSvgText::TextDecoration, QPainterPath> textDecorations;

    QString text;

    QPainterPath associatedOutline;

    void loadContextBasedProperties(SvgGraphicsContext *gc);
    bool isRichTextPreferred = true;
};

class KoSvgTextChunkShape::Private
{
public:
    struct LayoutInterface;
    QScopedPointer<LayoutInterface> layoutInterface;
};

