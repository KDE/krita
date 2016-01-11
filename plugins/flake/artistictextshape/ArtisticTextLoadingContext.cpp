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

#include "ArtisticTextLoadingContext.h"
#include "SvgUtil.h"
#include "SvgGraphicContext.h"

#include <KoXmlReader.h>
#include <QDebug>

#include <math.h>

ArtisticTextLoadingContext::ArtisticTextLoadingContext()
    : m_textPosition(HUGE_VAL, HUGE_VAL)
{
}

QString ArtisticTextLoadingContext::simplifyText(const QString &text, bool preserveWhiteSpace)
{
    // simplifies text according ot the svg specification
    QString simple = text;
    simple.remove('\n');
    simple.replace('\t', ' ');
    if (preserveWhiteSpace) {
        return simple;
    }

    QString stripped = simple.simplified();
    // preserve last whitespace character
    if (simple.endsWith(' ')) {
        stripped += QChar(' ');
    }

    return stripped;
}

ArtisticTextLoadingContext::OffsetType ArtisticTextLoadingContext::xOffsetType() const
{
    if (m_currentAbsolutePosX.data.count()) {
        return Absolute;
    } else if (m_currentRelativePosY.data.count()) {
        return Relative;
    } else {
        if (m_absolutePosX.count() &&  m_absolutePosX.last().data.count()) {
            return Absolute;
        } else if (m_relativePosX.count() && m_relativePosX.last().data.count()) {
            return Relative;
        }
    }
    return None;
}

ArtisticTextLoadingContext::OffsetType ArtisticTextLoadingContext::yOffsetType() const
{
    if (m_currentAbsolutePosY.data.count()) {
        return Absolute;
    } else if (m_currentRelativePosY.data.count()) {
        return Relative;
    } else {
        if (m_absolutePosY.count() &&  m_absolutePosY.last().data.count()) {
            return Absolute;
        } else if (m_relativePosY.count() && m_relativePosY.last().data.count()) {
            return Relative;
        }
    }
    return None;
}

CharTransforms ArtisticTextLoadingContext::xOffsets(int count)
{
    switch (xOffsetType()) {
    case Absolute: {
        const QPointF origin = textPosition();
        CharTransforms offsets = collectValues(count, m_currentAbsolutePosX, m_absolutePosX);
        const int offsetCount = offsets.count();
        for (int i = 0; i < offsetCount; ++i) {
            offsets[i] -= origin.x();
        }
        return offsets;
    }
    case Relative:
        return collectValues(count, m_currentRelativePosX, m_relativePosX);
    default:
        return CharTransforms();
    }
}

CharTransforms ArtisticTextLoadingContext::yOffsets(int count)
{
    switch (yOffsetType()) {
    case Absolute: {
        const QPointF origin = textPosition();
        CharTransforms offsets = collectValues(count, m_currentAbsolutePosY, m_absolutePosY);
        const int offsetCount = offsets.count();
        for (int i = 0; i < offsetCount; ++i) {
            offsets[i] -= origin.y();
        }
        return offsets;
    }
    case Relative:
        return collectValues(count, m_currentRelativePosY, m_relativePosY);
    default:
        return CharTransforms();
    }
}

CharTransforms ArtisticTextLoadingContext::rotations(int count)
{
    return collectValues(count, m_currentRotations, m_rotations);
}

QPointF ArtisticTextLoadingContext::textPosition() const
{
    qreal x = 0.0, y = 0.0;
    if (m_textPosition.x() != HUGE_VAL) {
        x = m_textPosition.x();
    }
    if (m_textPosition.y() != HUGE_VAL) {
        y = m_textPosition.y();
    }

    return QPointF(x, y);
}

/// Parses current character transforms (x,y,dx,dy,rotate)
void ArtisticTextLoadingContext::parseCharacterTransforms(const KoXmlElement &element, SvgGraphicsContext *gc)
{
    m_currentAbsolutePosX = parseList(element.attribute("x"), gc, XLength);
    m_currentAbsolutePosY = parseList(element.attribute("y"), gc, YLength);
    m_currentRelativePosX = parseList(element.attribute("dx"), gc, XLength);
    m_currentRelativePosY = parseList(element.attribute("dy"), gc, YLength);
    m_currentRotations = parseList(element.attribute("rotate"), gc, Number);

    if (m_textPosition.x() == HUGE_VAL && m_currentAbsolutePosX.data.count()) {
        m_textPosition.setX(m_currentAbsolutePosX.data.first());
    }
    if (m_textPosition.y() == HUGE_VAL && m_currentAbsolutePosY.data.count()) {
        m_textPosition.setY(m_currentAbsolutePosY.data.first());
    }
}

void ArtisticTextLoadingContext::pushCharacterTransforms()
{
    m_absolutePosX.append(m_currentAbsolutePosX);
    m_currentAbsolutePosX = CharTransformState();
    m_absolutePosY.append(m_currentAbsolutePosY);
    m_currentAbsolutePosY = CharTransformState();
    m_relativePosX.append(m_currentRelativePosX);
    m_currentRelativePosX = CharTransformState();
    m_relativePosY.append(m_currentRelativePosY);
    m_currentRelativePosY = CharTransformState();
    m_rotations.append(m_currentRotations);
    m_currentRotations = CharTransformState();
}

void ArtisticTextLoadingContext::popCharacterTransforms()
{
    m_currentAbsolutePosX = m_absolutePosX.last();
    m_absolutePosX.pop_back();
    m_currentAbsolutePosY = m_absolutePosY.last();
    m_absolutePosY.pop_back();
    m_currentRelativePosX = m_relativePosX.last();
    m_relativePosX.pop_back();
    m_currentRelativePosY = m_relativePosY.last();
    m_relativePosY.pop_back();
    m_currentRotations = m_rotations.last();
    m_rotations.pop_back();
}

CharTransforms ArtisticTextLoadingContext::parseList(const QString &listString, SvgGraphicsContext *gc, ValueType type)
{
    if (listString.isEmpty()) {
        return CharTransforms();
    } else {
        CharTransforms values;
        QStringList offsets = QString(listString).replace(',', ' ').simplified().split(' ');
        Q_FOREACH (const QString &offset, offsets) {
            switch (type) {
            case Number:
                values.append(offset.toDouble());
                break;
            case XLength:
                values.append(SvgUtil::parseUnitX(gc, offset));
                break;
            case YLength:
                values.append(SvgUtil::parseUnitY(gc, offset));
                break;
            }
        }
        return values;
    }
}

CharTransforms ArtisticTextLoadingContext::collectValues(int count, CharTransformState &current, CharTransformStack &stack)
{
    CharTransforms collected;

    if (current.hasData) {
        // use value from current data if that has initial values
        collected = current.extract(count);
    } else {
        collected = current.extract(count);
        // collect values from ancestors
        const int stackCount = stack.count();
        for (int i = stackCount - 1; i >= 0; --i) {
            CharTransformState &state = stack[i];
            // determine the number of values we need / can get from this ancestor
            const int copyCount = qMin(count - collected.count(), state.data.count());
            // extract values so they are not consumed more than once
            collected.append(state.extract(copyCount));
            // ok this ancestor had initial data, so we stop collecting values here
            if (state.hasData) {
                if (collected.isEmpty()) {
                    collected.append(state.lastTransform);
                }
                break;
            }
            if (copyCount == 0) {
                break;
            }
        }
    }
    return collected;
}

void ArtisticTextLoadingContext::printDebug()
{
    QString indent;
    Q_FOREACH (const CharTransformState &state, CharTransformStack(m_absolutePosX) << m_currentAbsolutePosX) {
        qDebug() << indent << state.data << state.hasData << state.lastTransform;
        indent.append("  ");
    }
    indent.clear();
    Q_FOREACH (const CharTransformState &state, CharTransformStack(m_absolutePosY) << m_currentAbsolutePosY) {
        qDebug() << indent << state.data << state.hasData << state.lastTransform;
        indent.append("  ");
    }
    indent.clear();
    Q_FOREACH (const CharTransformState &state, CharTransformStack(m_relativePosX) << m_currentRelativePosX) {
        qDebug() << indent << state.data << state.hasData << state.lastTransform;
        indent.append("  ");
    }
    indent.clear();
    Q_FOREACH (const CharTransformState &state, CharTransformStack(m_relativePosY) << m_currentRelativePosY) {
        qDebug() << indent << state.data << state.hasData << state.lastTransform;
        indent.append("  ");
    }
    indent.clear();
    Q_FOREACH (const CharTransformState &state, CharTransformStack(m_rotations) << m_currentRotations) {
        qDebug() << indent << state.data << state.hasData << state.lastTransform;
        indent.append("  ");
    }
    indent.clear();
}
