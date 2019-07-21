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

#include "KoSvgTextChunkShape.h"

#include "KoSvgText.h"
#include "KoSvgTextProperties.h"
#include <QSharedData>
#include <QTextCharFormat>

class SvgGraphicsContext;

class KoSvgTextChunkShape::Private : public QSharedData
{
public:

    Private();
    Private(const Private &rhs);
    ~Private();

    KoSvgTextProperties properties;
    QFont font;
    QStringList fontFamiliesList;

    QVector<KoSvgText::CharTransformation> localTransformations;

    KoSvgText::AutoValue textLength;
    KoSvgText::LengthAdjust lengthAdjust = KoSvgText::LengthAdjustSpacing;

    QString text;

    struct LayoutInterface;
    QScopedPointer<LayoutInterface> layoutInterface;

    QPainterPath associatedOutline;

    void loadContextBasedProperties(SvgGraphicsContext *gc);
    bool isRichTextPreferred = true;
};

