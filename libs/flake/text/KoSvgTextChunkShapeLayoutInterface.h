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

#ifndef KOSVGTEXTCHUNKSHAPELAYOUTINTERFACE_H
#define KOSVGTEXTCHUNKSHAPELAYOUTINTERFACE_H

#include <kritaflake_export.h>

#include <QTextCharFormat>

#include "KoSvgText.h"
#include <boost/optional.hpp>

class KoSvgTextChunkShape;


class KRITAFLAKE_EXPORT KoSvgTextChunkShapeLayoutInterface
{
public:
    KoSvgTextChunkShapeLayoutInterface() {}
    virtual ~KoSvgTextChunkShapeLayoutInterface() {}

    virtual KoSvgText::AutoValue textLength() const = 0;
    virtual KoSvgText::LengthAdjust lengthAdjust() const = 0;

    virtual int numChars() const = 0;
    virtual int relativeCharPos(KoSvgTextChunkShape *child, int pos) const = 0;

    virtual bool isTextNode() const = 0;
    virtual QString nodeText() const = 0;

    virtual QVector<KoSvgText::CharTransformation> localCharTransformations() const = 0;

    virtual void addAssociatedOutline(const QRectF &rect) = 0;
    virtual void clearAssociatedOutline() = 0;

    struct SubChunk
    {
        SubChunk()
        {
        }

        SubChunk(const QString &_text, const KoSvgText::KoSvgCharChunkFormat &_format)
            : text(_text), format(_format)
        {
        }

        SubChunk(const QString &_text, const KoSvgText::KoSvgCharChunkFormat &_format,
                  const KoSvgText::CharTransformation &t)
            : text(_text), format(_format), transformation(t)
        {
        }

        QString text;
        KoSvgText::KoSvgCharChunkFormat format;
        KoSvgText::CharTransformation transformation;
    };

    virtual QVector<SubChunk> collectSubChunks() const = 0;
};

#endif // KOSVGTEXTCHUNKSHAPELAYOUTINTERFACE_H
