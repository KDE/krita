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

#include "KoTableStyle.h"

KOSTYLE_DECLARE_SHARED_POINTER_IMPL(KoTableStyle)

namespace {
    class BreakStyleMap : public QMap<KoTableStyle::BreakType, QString>
    {
    public:
        BreakStyleMap()
        {
            insert(KoTableStyle::NoBreak, QString());
            insert(KoTableStyle::AutoBreak, "auto");
            insert(KoTableStyle::ColumnBreak, "column");
            insert(KoTableStyle::PageBreak, "page");
        }
    } breakStyleMap;

    class HorizontalAlignMap : public QMap<KoTableStyle::HorizontalAlign, QString>
    {
    public:
        HorizontalAlignMap()
        {
            insert(KoTableStyle::CenterAlign, "center");
            insert(KoTableStyle::LeftAlign, "left");
            insert(KoTableStyle::MarginsAlign, "margins");
            insert(KoTableStyle::RightAlign, "right");
        }
    } horizontalAlignMap;

    class BorderModelMap : public QMap<KoTableStyle::BorderModel, QString>
    {
    public:
        BorderModelMap()
        {
            insert(KoTableStyle::CollapsingModel, "collapsing");
            insert(KoTableStyle::SeparatingModel, "separating");
        }
    } borderModelMap;

    class KeepWithNextMap : public QMap<KoTableStyle::KeepWithNext, QString>
    {
    public:
        KeepWithNextMap()
        {
            insert(KoTableStyle::AutoKeepWithNext, "auto");
            insert(KoTableStyle::AlwaysKeepWithNext, "always");
        }
    } keepWithNextMap;

    class WritingModeMap : public QMap<KoTableStyle::WritingMode, QString>
    {
    public:
        WritingModeMap()
        {
            insert(KoTableStyle::LrTbWrittingMode, "lr-tb");
            insert(KoTableStyle::RlTbWrittingMode, "rl-tb");
            insert(KoTableStyle::TbRlWrittingMode, "tb-rl");
            insert(KoTableStyle::TbLrWrittingMode, "tb-lr");
            insert(KoTableStyle::LrWrittingMode, "lr");
            insert(KoTableStyle::RlWrittingMode, "rl");
            insert(KoTableStyle::TbWrittingMode, "tb");
            insert(KoTableStyle::PageWrittingMode, "page");
        }
    } writingModeMap;

    QString prefix = "table";
    const char* familyName = "table";
}

KoTableStyle::KoTableStyle()
: KoStyle()
, m_backgroundColor()
, m_breakAfter(NoBreak)
, m_breakBefore(NoBreak)
, m_allowBreakBetweenRows(false)
, m_leftMargin(0.0)
, m_topMargin(0.0)
, m_rightMargin(0.0)
, m_bottomMargin(0.0)
, m_width(0.0)
, m_widthUnit(PointsUnit)
, m_horizontalAlign(LeftAlign)
, m_borderModel(CollapsingModel)
, m_keepWithNext(AutoKeepWithNext)
, m_writingMode(PageWrittingMode)
, m_display(true)
{
}

KoTableStyle::~KoTableStyle()
{
}

void KoTableStyle::setAllowBreakBetweenRows(bool allow)
{
    m_allowBreakBetweenRows = allow;
}

bool KoTableStyle::allowBreakBetweenRows() const
{
    return m_allowBreakBetweenRows;
}

void KoTableStyle::setBackgroundColor(const QColor& color)
{
    m_backgroundColor = color;
}

QColor KoTableStyle::backgroundColor() const
{
    return m_backgroundColor;
}

void KoTableStyle::setWidth(qreal width, KoTableStyle::WidthUnit unit)
{
    m_width = width;
    m_widthUnit = unit;
}

qreal KoTableStyle::width() const
{
    return m_width;
}

KoTableStyle::WidthUnit KoTableStyle::widthUnit() const
{
    return m_widthUnit;
}

void KoTableStyle::setLeftMargin(qreal left)
{
    m_leftMargin = left;
}

qreal KoTableStyle::leftMargin() const
{
    return m_leftMargin;
}

void KoTableStyle::setTopMargin(qreal top)
{
    m_topMargin = top;
}

qreal KoTableStyle::topMargin() const
{
    return m_topMargin;
}

void KoTableStyle::setRightMargin(qreal right)
{
    m_rightMargin = right;
}

qreal KoTableStyle::rightMargin() const
{
    return m_rightMargin;
}

void KoTableStyle::setBottomMargin(qreal bottom)
{
    m_bottomMargin = bottom;
}

qreal KoTableStyle::bottomMargin() const
{
    return m_bottomMargin;
}

void KoTableStyle::setHorizontalAlign(KoTableStyle::HorizontalAlign align)
{
    m_horizontalAlign = align;
}

KoTableStyle::HorizontalAlign KoTableStyle::horizontalAlign() const
{
    return m_horizontalAlign;
}

void KoTableStyle::setDisplay(bool display)
{
    m_display = display;
}

bool KoTableStyle::display() const
{
    return m_display;
}

void KoTableStyle::setBreakBefore(KoTableStyle::BreakType breakBefore)
{
    m_breakBefore = breakBefore;
}

KoTableStyle::BreakType KoTableStyle::breakBefore() const
{
    return m_breakBefore;
}

void KoTableStyle::setBreakAfter(KoTableStyle::BreakType breakAfter)
{
    m_breakAfter = breakAfter;
}

KoTableStyle::BreakType KoTableStyle::breakAfter() const
{
    return m_breakAfter;
}

void KoTableStyle::setBorderModel(KoTableStyle::BorderModel bordelModel)
{
    m_borderModel = bordelModel;
}

KoTableStyle::BorderModel KoTableStyle::borderModel() const
{
    return m_borderModel;
}

void KoTableStyle::setKeepWithNext(KoTableStyle::KeepWithNext keepWithNext)
{
    m_keepWithNext = keepWithNext;
}

KoTableStyle::KeepWithNext KoTableStyle::keepWithNext() const
{
    return m_keepWithNext;
}

void KoTableStyle::setWritingMode(KoTableStyle::WritingMode writingMode)
{
    m_writingMode = writingMode;
}

KoTableStyle::WritingMode KoTableStyle::writingMode() const
{
    return m_writingMode;
}

KoGenStyle::Type KoTableStyle::automaticstyleType() const
{
    return KoGenStyle::TableAutoStyle;
}

KoGenStyle::Type KoTableStyle::styleType() const
{
    return KoGenStyle::TableStyle;
}

const char* KoTableStyle::styleFamilyName() const
{
    return familyName;
}

QString KoTableStyle::defaultPrefix() const
{
    return prefix;
}

void KoTableStyle::prepareStyle(KoGenStyle& style) const
{
    if(m_backgroundColor.isValid()) {
        style.addProperty("fo:background-color", m_backgroundColor.name());
    }
    style.addProperty("fo:break-after", breakStyleMap.value(m_breakAfter));
    style.addProperty("fo:break-before", breakStyleMap.value(m_breakBefore));
    style.addProperty("fo:keep-with-next", keepWithNextMap.value(m_keepWithNext));

    style.addPropertyPt("fo:margin-top", m_topMargin);
    style.addPropertyPt("fo:margin-right", m_rightMargin);
    style.addPropertyPt("fo:margin-bottom", m_bottomMargin);
    style.addPropertyPt("fo:margin-left", m_leftMargin);


    switch(m_widthUnit) {
        case PercentageUnit:
            style.addPropertyPt("style:rel-width", m_width);
            break;
        case PointsUnit:
            style.addPropertyPt("style:width", m_width);
            break;
    }

    style.addProperty("style:may-break-between-rows", m_allowBreakBetweenRows ? "true" : "false");
    style.addProperty("style:writing-mode", writingModeMap.value(m_writingMode));
    style.addProperty("table:align", horizontalAlignMap.value(m_horizontalAlign));
    style.addProperty("fo:border-model", borderModelMap.value(m_borderModel));

    if(!m_display) {
        style.addProperty("table:display", "false");
    }
}
