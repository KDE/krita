/* This file is part of the KDE project
 * Copyright (C) 2011 Jan Hambrecht <jaham@gmx.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef SVGTEXTHELPER_H
#define SVGTEXTHELPER_H

#include <QVector>
#include <QString>
#include <QList>
#include <QPointF>

typedef QList<qreal> CharTransforms;

class KoXmlElement;
class SvgGraphicsContext;

class ArtisticTextLoadingContext
{
public:
    enum OffsetType {
        None,
        Absolute,
        Relative
    };

    ArtisticTextLoadingContext();

    static QString simplifyText(const QString &text, bool preserveWhiteSpace = false);

    /// Parses current character transforms (x,y,dx,dy,rotate)
    void parseCharacterTransforms(const KoXmlElement &element, SvgGraphicsContext *gc);

    /// Pushes the current character transforms to the stack
    void pushCharacterTransforms();

    /// Pops last character transforms from the stack
    void popCharacterTransforms();

    /// Checks current x-offset type
    OffsetType xOffsetType() const;

    /// Checks current y-offset type
    OffsetType yOffsetType() const;

    /// Returns x-offsets from stack
    CharTransforms xOffsets(int count);

    /// Returns y-offsets from stack
    CharTransforms yOffsets(int count);

    /// Returns rotations from stack
    CharTransforms rotations(int count);

    /// Returns the text position
    QPointF textPosition() const;

private:
    void printDebug();

    struct CharTransformState {
        CharTransformState()
            : hasData(false), lastTransform(0.0)
        {
        }

        CharTransformState(const CharTransforms &initialData)
            : data(initialData), hasData(!initialData.isEmpty())
            , lastTransform(initialData.isEmpty() ? 0.0 : initialData.last())
        {
        }

        CharTransforms extract(int count)
        {
            const int copyCount = qMin(data.count(), count);
            CharTransforms extracted = data.mid(0, copyCount);
            data = data.mid(copyCount);
            return extracted;
        }

        CharTransforms data;
        bool hasData;
        qreal lastTransform;
    };

    typedef QList<CharTransformState> CharTransformStack;

    enum ValueType {
        Number,
        XLength,
        YLength
    };

    /// Parses offset values from the given string
    CharTransforms parseList(const QString &listString, SvgGraphicsContext *gc, ValueType type);

    /// Collects number of specified transforms values from the stack
    CharTransforms collectValues(int count, CharTransformState &current, CharTransformStack &stack);

    CharTransformState m_currentAbsolutePosX; ///< current absolute character x-positions
    CharTransformState m_currentAbsolutePosY; ///< current absolute character y-positions
    CharTransformState m_currentRelativePosX; ///< current relative character x-positions
    CharTransformState m_currentRelativePosY; ///< current relative character y-positions
    CharTransformState m_currentRotations;    ///< current character rotations
    CharTransformStack m_absolutePosX; ///< stack of absolute character x-positions
    CharTransformStack m_absolutePosY; ///< stack of absolute character y-positions
    CharTransformStack m_relativePosX; ///< stack of relative character x-positions
    CharTransformStack m_relativePosY; ///< stack of relative character y-positions
    CharTransformStack m_rotations;    ///< stack of character rotations

    QPointF m_textPosition;
};

#endif // SVGTEXTHELPER_H
