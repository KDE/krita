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
, m_leftPadding(0.0)
, m_topPadding(0.0)
, m_rightPadding(0.0)
, m_bottomPadding(0.0)
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

void KoCellStyle::prepareStyle( KoGenStyle& style ) const
{
    m_borders->saveOdf(style);
    if(m_backgroundColor.isValid()) {
        style.addProperty("fo:background-color", m_backgroundColor.name());
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
}
