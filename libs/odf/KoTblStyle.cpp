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

#include "KoTblStyle.h"

KOSTYLE_DECLARE_SHARED_POINTER_IMPL(KoTblStyle)

namespace {
    class BreakStyleMap : public QMap<KoTblStyle::BreakType, QString>
    {
    public:
        BreakStyleMap()
        {
            insert(KoTblStyle::NoBreak, QString());
            insert(KoTblStyle::AutoBreak, "auto");
            insert(KoTblStyle::ColumnBreak, "column");
            insert(KoTblStyle::PageBreak, "page");
        }
    } breakStyleMap;

    class HorizontalAlignMap : public QMap<KoTblStyle::HorizontalAlign, QString>
    {
    public:
        HorizontalAlignMap()
        {
            insert(KoTblStyle::CenterAlign, "center");
            insert(KoTblStyle::LeftAlign, "left");
            insert(KoTblStyle::MarginsAlign, "margins");
            insert(KoTblStyle::RightAlign, "right");
        }
    } horizontalAlignMap;

    class BorderModelMap : public QMap<KoTblStyle::BorderModel, QString>
    {
    public:
        BorderModelMap()
        {
            insert(KoTblStyle::CollapsingModel, "collapsing");
            insert(KoTblStyle::SeparatingModel, "separating");
        }
    } borderModelMap;

    class KeepWithNextMap : public QMap<KoTblStyle::KeepWithNext, QString>
    {
    public:
        KeepWithNextMap()
        {
            insert(KoTblStyle::AutoKeepWithNext, "auto");
            insert(KoTblStyle::AlwaysKeepWithNext, "always");
        }
    } keepWithNextMap;

    class WritingModeMap : public QMap<KoTblStyle::WritingMode, QString>
    {
    public:
        WritingModeMap()
        {
            insert(KoTblStyle::LrTbWritingMode, "lr-tb");
            insert(KoTblStyle::RlTbWritingMode, "rl-tb");
            insert(KoTblStyle::TbRlWritingMode, "tb-rl");
            insert(KoTblStyle::TbLrWritingMode, "tb-lr");
            insert(KoTblStyle::LrWritingMode, "lr");
            insert(KoTblStyle::RlWritingMode, "rl");
            insert(KoTblStyle::TbWritingMode, "tb");
            insert(KoTblStyle::PageWritingMode, "page");
        }
    } writingModeMap;

    QString prefix = "table";
    const char* familyName = "table";
}

KoTblStyle::KoTblStyle()
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
  , m_writingMode(PageWritingMode)
  , m_display(true)
{
}

KoTblStyle::~KoTblStyle()
{
}

void KoTblStyle::setAllowBreakBetweenRows(bool allow)
{
    m_allowBreakBetweenRows = allow;
}

bool KoTblStyle::allowBreakBetweenRows() const
{
    return m_allowBreakBetweenRows;
}

void KoTblStyle::setMasterPageName(const QString& name)
{
    m_masterPageName = name;
}

void KoTblStyle::setMasterPageName(const char* name)
{
    m_masterPageName = QString::fromUtf8(name);
}

QString KoTblStyle::masterPageName() const
{
    return m_masterPageName;
}

void KoTblStyle::setBackgroundColor(const QColor& color)
{
    m_backgroundColor = color;
}

QColor KoTblStyle::backgroundColor() const
{
    return m_backgroundColor;
}

void KoTblStyle::setWidth(qreal width, KoTblStyle::WidthUnit unit)
{
    m_width = width;
    m_widthUnit = unit;
}

qreal KoTblStyle::width() const
{
    return m_width;
}

KoTblStyle::WidthUnit KoTblStyle::widthUnit() const
{
    return m_widthUnit;
}

void KoTblStyle::setLeftMargin(qreal left)
{
    m_leftMargin = left;
}

qreal KoTblStyle::leftMargin() const
{
    return m_leftMargin;
}

void KoTblStyle::setTopMargin(qreal top)
{
    m_topMargin = top;
}

qreal KoTblStyle::topMargin() const
{
    return m_topMargin;
}

void KoTblStyle::setRightMargin(qreal right)
{
    m_rightMargin = right;
}

qreal KoTblStyle::rightMargin() const
{
    return m_rightMargin;
}

void KoTblStyle::setBottomMargin(qreal bottom)
{
    m_bottomMargin = bottom;
}

qreal KoTblStyle::bottomMargin() const
{
    return m_bottomMargin;
}

void KoTblStyle::setHorizontalAlign(KoTblStyle::HorizontalAlign align)
{
    m_horizontalAlign = align;
}

KoTblStyle::HorizontalAlign KoTblStyle::horizontalAlign() const
{
    return m_horizontalAlign;
}

void KoTblStyle::setDisplay(bool display)
{
    m_display = display;
}

bool KoTblStyle::display() const
{
    return m_display;
}

void KoTblStyle::setBreakBefore(KoTblStyle::BreakType breakBefore)
{
    m_breakBefore = breakBefore;
}

KoTblStyle::BreakType KoTblStyle::breakBefore() const
{
    return m_breakBefore;
}

void KoTblStyle::setBreakAfter(KoTblStyle::BreakType breakAfter)
{
    m_breakAfter = breakAfter;
}

KoTblStyle::BreakType KoTblStyle::breakAfter() const
{
    return m_breakAfter;
}

void KoTblStyle::setBorderModel(KoTblStyle::BorderModel bordelModel)
{
    m_borderModel = bordelModel;
}

KoTblStyle::BorderModel KoTblStyle::borderModel() const
{
    return m_borderModel;
}

void KoTblStyle::setKeepWithNext(KoTblStyle::KeepWithNext keepWithNext)
{
    m_keepWithNext = keepWithNext;
}

KoTblStyle::KeepWithNext KoTblStyle::keepWithNext() const
{
    return m_keepWithNext;
}

void KoTblStyle::setWritingMode(KoTblStyle::WritingMode writingMode)
{
    m_writingMode = writingMode;
}

KoTblStyle::WritingMode KoTblStyle::writingMode() const
{
    return m_writingMode;
}

KoGenStyle::Type KoTblStyle::automaticstyleType() const
{
    return KoGenStyle::TableAutoStyle;
}

KoGenStyle::Type KoTblStyle::styleType() const
{
    return KoGenStyle::TableStyle;
}

const char* KoTblStyle::styleFamilyName() const
{
    return familyName;
}

QString KoTblStyle::defaultPrefix() const
{
    return prefix;
}

void KoTblStyle::prepareStyle(KoGenStyle& style) const
{
    if(m_backgroundColor.isValid()) {
        style.addProperty("fo:background-color", m_backgroundColor.name());
    }
    if (m_breakAfter != KoTblStyle::NoBreak) {
        style.addProperty("fo:break-after", breakStyleMap.value(m_breakAfter));
    }
    if (m_breakBefore != KoTblStyle::NoBreak) {
        style.addProperty("fo:break-before", breakStyleMap.value(m_breakBefore));
    }
    style.addProperty("fo:keep-with-next", keepWithNextMap.value(m_keepWithNext));

    style.addPropertyPt("fo:margin-top", m_topMargin);
    style.addPropertyPt("fo:margin-right", m_rightMargin);
    style.addPropertyPt("fo:margin-bottom", m_bottomMargin);
    style.addPropertyPt("fo:margin-left", m_leftMargin);

    // style:width may not be 0, use style:rel-width if width is 0
    if (m_widthUnit == PercentageUnit || m_width <= 0) {
        style.addProperty("style:rel-width", QString::number(m_width) + "%");
    } else {
        style.addPropertyPt("style:width", m_width);
    }

    style.addProperty("style:may-break-between-rows", m_allowBreakBetweenRows ? "true" : "false");
    style.addProperty("style:writing-mode", writingModeMap.value(m_writingMode));
    style.addProperty("table:align", horizontalAlignMap.value(m_horizontalAlign));
    style.addProperty("table:border-model", borderModelMap.value(m_borderModel));

    if(!m_display) {
        style.addProperty("table:display", "false");
    }

    if(!m_masterPageName.isEmpty()) {
        style.addAttribute("style:master-page-name", m_masterPageName);
    }
}
