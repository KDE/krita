/* This file is part of the KDE project
 * Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
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

#include "EnhancedPathParameter.h"
#include "EnhancedPathFormula.h"
#include "EnhancedPathShape.h"
#include <KoShapeBackground.h>
#include <KoUnit.h>
#include <math.h>

QString identifierData[] = {
    "",          // IdentifierUnknown
    "pi",        // IdentifierPi
    "left",      // IdentifierLeft
    "top",       // IdentifierTop
    "right",     // IdentifierRight
    "bottom",    // IdentifierBottom
    "xstretch",  // IdentifierXstretch
    "ystretch",  // IdentifierYstretch
    "hasstroke", // IdentifierHasStroke
    "hasfill",   // IdentifierHasFill
    "width",     // IdentifierWidth
    "height",    // IdentifierHeight
    "logwidth",  // IdentifierLogwidth
    "logheight"  // IdentifierLogheight
};

EnhancedPathParameter::EnhancedPathParameter(EnhancedPathShape *parent)
    : m_parent(parent)
{
    Q_ASSERT(m_parent);
}

EnhancedPathParameter::~EnhancedPathParameter()
{
}

EnhancedPathShape *EnhancedPathParameter::parent()
{
    return m_parent;
}

qreal EnhancedPathParameter::evaluate()
{
    return 0.0;
}

void EnhancedPathParameter::modify(qreal value)
{
    Q_UNUSED(value);
}

EnhancedPathConstantParameter::EnhancedPathConstantParameter(qreal value, EnhancedPathShape *parent)
    : EnhancedPathParameter(parent)
    , m_value(value)
{
}

qreal EnhancedPathConstantParameter::evaluate()
{
    return m_value;
}

QString EnhancedPathConstantParameter::toString() const
{
    return QString::number(m_value);
}

EnhancedPathNamedParameter::EnhancedPathNamedParameter(Identifier identifier, EnhancedPathShape *parent)
    : EnhancedPathParameter(parent)
    , m_identifier(identifier)
{
}

EnhancedPathNamedParameter::EnhancedPathNamedParameter(const QString &identifier, EnhancedPathShape *parent)
    : EnhancedPathParameter(parent)
{
    m_identifier = identifierFromString(identifier);
}

qreal EnhancedPathNamedParameter::evaluate()
{
    const QRect &viewBox = parent()->viewBox();

    switch (m_identifier) {
    case IdentifierPi:
        return M_PI;
        break;
    case IdentifierLeft:
        return viewBox.left();
        break;
    case IdentifierTop:
        return viewBox.top();
        break;
    case IdentifierRight:
        return viewBox.right();
        break;
    case IdentifierBottom:
        return viewBox.bottom();
        break;
    case IdentifierXstretch:
        break;
    case IdentifierYstretch:
        break;
    case IdentifierHasStroke:
        return parent()->stroke() ? 1.0 : 0.0;
        break;
    case IdentifierHasFill:
        return parent()->background() ? 0.0 : 1.0;
        break;
    case IdentifierWidth:
        return viewBox.width();
        break;
    case IdentifierHeight:
        return viewBox.height();
        break;
    case IdentifierLogwidth:
        // TODO: ? viewBox does not have any unit or const relation to mm
        return KoUnit(KoUnit::Millimeter).toUserValue(viewBox.width()) * 100;
        break;
    case IdentifierLogheight:
        // TODO: ? viewBox does not have any unit or const relation to mm
        return KoUnit(KoUnit::Millimeter).toUserValue(viewBox.height()) * 100;
        break;
    default:
        break;
    }
    return 0.0;
}

Identifier EnhancedPathNamedParameter::identifierFromString(const QString &text)
{
    if (text.isEmpty()) {
        return IdentifierUnknown;
    } else if (text == "pi") {
        return IdentifierPi;
    } else if (text == "left") {
        return IdentifierLeft;
    } else if (text == "top") {
        return IdentifierTop;
    } else if (text == "right") {
        return IdentifierRight;
    } else if (text == "bottom") {
        return IdentifierBottom;
    } else if (text == "xstretch") {
        return IdentifierXstretch;
    } else if (text == "ystretch") {
        return IdentifierYstretch;
    } else if (text == "hasstroke") {
        return IdentifierHasStroke;
    } else if (text == "hasfill") {
        return IdentifierHasFill;
    } else if (text == "width") {
        return IdentifierWidth;
    } else if (text == "height") {
        return IdentifierHeight;
    } else if (text == "logwidth") {
        return IdentifierLogwidth;
    } else if (text == "logheight") {
        return IdentifierLogheight;
    } else {
        return IdentifierUnknown;
    }
}

QString EnhancedPathNamedParameter::toString() const
{
    return identifierData[m_identifier];
}

EnhancedPathReferenceParameter::EnhancedPathReferenceParameter(const QString &reference, EnhancedPathShape *parent)
    : EnhancedPathParameter(parent)
    , m_reference(reference)
{
}

qreal EnhancedPathReferenceParameter::evaluate()
{
    return parent()->evaluateReference(m_reference);
}

void EnhancedPathReferenceParameter::modify(qreal value)
{
    parent()->modifyReference(m_reference, value);
}

QString EnhancedPathReferenceParameter::toString() const
{
    return m_reference;
}

