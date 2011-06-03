/*
 *  Copyright (c) 2010 Carlos Licea <carlos@kdab.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published
 *  by the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "KoCellStyle.h"
#include "KoGenStyle.h"
#include "KoGenStyles.h"

namespace {
    QString prefix = "cell";
    const char* familyName = "table-cell";
}

KOSTYLE_DECLARE_SHARED_POINTER_IMPL(KoCellStyle)

KoCellStyle::KoCellStyle()
: KoStyle()
, m_borders(new KoBorder)
, m_backgroundColor()
, m_backgroundOpacity(0.0)
, m_leftPadding(0.0)
, m_topPadding(0.0)
, m_rightPadding(0.0)
, m_bottomPadding(0.0)
, m_verticalAlign("")
, m_glyphOrientation(true)
{
}

KoCellStyle::~KoCellStyle()
{
    delete m_borders;
}

KoBorder* KoCellStyle::borders()
{
    return m_borders;
}

QString KoCellStyle::defaultPrefix() const
{
    return prefix;
}

KoGenStyle::Type KoCellStyle::styleType() const
{
    return KoGenStyle::TableCellStyle;
}

KoGenStyle::Type KoCellStyle::automaticstyleType() const
{
    return KoGenStyle::TableCellAutoStyle;
}

const char* KoCellStyle::styleFamilyName() const
{
    return familyName;
}

QColor KoCellStyle::backgroundColor() const
{
    return m_backgroundColor;
}

void KoCellStyle::setBackgroundColor(const QColor& color)
{
    m_backgroundColor = color;
}

void KoCellStyle::setBackgroundOpacity(qreal opacity)
{
    m_backgroundOpacity = opacity;
}

qreal KoCellStyle::backgroundOpacity() const
{
    return m_backgroundOpacity;
}

qreal KoCellStyle::topPadding() const
{
    return m_topPadding;
}

void KoCellStyle::setTopPadding(qreal padding)
{
    m_topPadding = padding;
}

qreal KoCellStyle::bottomPadding() const
{
    return m_bottomPadding;
}

void KoCellStyle::setBottomPadding(qreal padding)
{
    m_bottomPadding = padding;
}

qreal KoCellStyle::leftPadding() const
{
    return m_leftPadding;
}

void KoCellStyle::setLeftPadding(qreal padding)
{
    m_leftPadding = padding;
}

qreal KoCellStyle::rightPadding() const
{
    return m_rightPadding;
}

void KoCellStyle::setRightPadding(qreal padding)
{
    m_rightPadding = padding;
}

QString KoCellStyle::verticalAlign() const
{
    return m_verticalAlign;
}

void KoCellStyle::setVerticalAlign(const QString& align)
{
    m_verticalAlign = align;
}

bool KoCellStyle::glyphOrientation() const
{
    return m_glyphOrientation;
}

void KoCellStyle::setGlyphOrientation(bool orientation)
{
    m_glyphOrientation = orientation;
}

KoGenStyle KoCellStyle::styleProperties() const
{
    return m_styleProperties;
}

void KoCellStyle::setTextStyle(const KoGenStyle& style)
{
    KoGenStyle::copyPropertiesFromStyle(style, m_styleProperties, KoGenStyle::TextType);
}

void KoCellStyle::setParagraphStyle(const KoGenStyle& style)
{
    KoGenStyle::copyPropertiesFromStyle(style, m_styleProperties, KoGenStyle::ParagraphType);
}

void KoCellStyle::prepareStyle( KoGenStyle& style ) const
{
    m_borders->saveOdf(style);
    if (m_backgroundColor.isValid()) {
        style.addProperty("fo:background-color", m_backgroundColor.name());
    }
    if (m_backgroundOpacity != 0.0) {
        style.addProperty("draw:opacity", QString("%1%").arg(m_backgroundOpacity), KoGenStyle::GraphicType);
    }
    if(m_leftPadding != 0) {
        style.addPropertyPt("fo:padding-left", m_leftPadding);
    }
    if(m_topPadding != 0) {
        style.addPropertyPt("fo:padding-top", m_topPadding);
    }
    if(m_rightPadding != 0) {
        style.addPropertyPt("fo:padding-right", m_rightPadding);
    }
    if(m_bottomPadding != 0) {
        style.addPropertyPt("fo:padding-bottom", m_bottomPadding);
    }
    if (!m_verticalAlign.isEmpty()) {
        style.addProperty("style:vertical-align", m_verticalAlign);
    }
    if (!m_glyphOrientation) {
        style.addProperty("style:glyph-orientation-vertical", "0");
    }
    KoGenStyle::copyPropertiesFromStyle(m_styleProperties, style, KoGenStyle::ParagraphType);
    KoGenStyle::copyPropertiesFromStyle(m_styleProperties, style, KoGenStyle::TextType);
}
