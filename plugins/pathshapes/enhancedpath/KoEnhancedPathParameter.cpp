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

#include "KoEnhancedPathParameter.h"
#include "KoEnhancedPathFormula.h"
#include "KoEnhancedPathShape.h"
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


KoEnhancedPathParameter::KoEnhancedPathParameter(KoEnhancedPathShape * parent)
    : m_parent(parent)
{
    Q_ASSERT(m_parent);
}

KoEnhancedPathParameter::~KoEnhancedPathParameter()
{
}

KoEnhancedPathShape * KoEnhancedPathParameter::parent()
{
    return m_parent;
}

qreal KoEnhancedPathParameter::evaluate()
{
    return 0.0;
}

void KoEnhancedPathParameter::modify(qreal value)
{
    Q_UNUSED(value);
}

KoEnhancedPathConstantParameter::KoEnhancedPathConstantParameter(qreal value, KoEnhancedPathShape * parent)
    : KoEnhancedPathParameter(parent), m_value(value)
{
}

qreal KoEnhancedPathConstantParameter::evaluate()
{
    return m_value;
}

QString KoEnhancedPathConstantParameter::toString() const
{
    return QString::number(m_value);
}

KoEnhancedPathNamedParameter::KoEnhancedPathNamedParameter(Identifier identifier, KoEnhancedPathShape * parent)
: KoEnhancedPathParameter(parent), m_identifier(identifier)
{
}

KoEnhancedPathNamedParameter::KoEnhancedPathNamedParameter(const QString &identifier, KoEnhancedPathShape * parent)
    : KoEnhancedPathParameter(parent)
{
    m_identifier = identifierFromString(identifier);
}

qreal KoEnhancedPathNamedParameter::evaluate()
{
    const QRectF &viewBox = parent()->viewBox();

    switch(m_identifier) {
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
        return parent()->border() ? 1.0 : 0.0;
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
        return KoUnit::toMillimeter(viewBox.width()) * 100;
        break;
    case IdentifierLogheight:
        return KoUnit::toMillimeter(viewBox.height()) * 100;
        break;
    default:
        break;
    }
    return 0.0;
}

Identifier KoEnhancedPathNamedParameter::identifierFromString(const QString &text)
{
    if (text.isEmpty())
        return IdentifierUnknown;
    else if (text == "pi")
        return IdentifierPi;
    else if (text == "left")
        return IdentifierLeft;
    else if (text == "top")
        return IdentifierTop;
    else if (text == "right")
        return IdentifierRight;
    else if (text == "bottom")
        return IdentifierBottom;
    else if (text == "xstretch")
        return IdentifierXstretch;
    else if (text == "ystretch")
        return IdentifierYstretch;
    else if (text == "hasstroke")
        return IdentifierHasStroke;
    else if (text == "hasfill")
        return IdentifierHasFill;
    else if (text == "width")
        return IdentifierWidth;
    else if (text == "height")
        return IdentifierHeight;
    else if (text == "logwidth")
        return IdentifierLogwidth;
    else if (text == "logheight")
        return IdentifierLogheight;
    else
        return IdentifierUnknown;
}

QString KoEnhancedPathNamedParameter::toString() const
{
    return identifierData[m_identifier];
}

KoEnhancedPathReferenceParameter::KoEnhancedPathReferenceParameter(const QString &reference, KoEnhancedPathShape * parent)
: KoEnhancedPathParameter(parent), m_reference(reference)
{
}

qreal KoEnhancedPathReferenceParameter::evaluate()
{
    return parent()->evaluateReference(m_reference);
}

void KoEnhancedPathReferenceParameter::modify(qreal value)
{
    parent()->modifyReference(m_reference, value);
}

QString KoEnhancedPathReferenceParameter::toString() const
{
    return m_reference;
}

