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

#include "KoRow.h"

#include <KoXmlWriter.h>

#include <QMap>
#include <QString>

namespace {
    class VisibilityMap : public QMap<KoRow::Visibility, QString>
    {
    public:
        VisibilityMap()
        : QMap<KoRow::Visibility, QString>()
        {
            insert(KoRow::Collapse, "colapse");
            insert(KoRow::Filter, "filter");
            insert(KoRow::Visible, "visible");
        }
    } visibilityMap;
}


KoRow::KoRow()
: m_defaultCellStyle(0)
, m_style(0)
, m_visibility(Visible)
{
}

KoRow::~KoRow()
{
}

void KoRow::setStyle(KoRowStyle::Ptr style)
{
    m_style = style;
}

KoRowStyle::Ptr KoRow::style()
{
    return m_style;
}

KoCellStyle::Ptr KoRow::defualtCellStyle() const
{
    return m_defaultCellStyle;
}

void KoRow::setDefaultCellStyle(KoCellStyle::Ptr defaultStyle)
{
    m_defaultCellStyle = defaultStyle;
}

void KoRow::setVisibility(KoRow::Visibility visibility)
{
    m_visibility = visibility;
}

KoRow::Visibility KoRow::visibility()
{
    return m_visibility;
}

void KoRow::saveOdf(KoXmlWriter& writer, KoGenStyles& styles)
{
    writer.startElement("table:table-row");
    if(m_style) {
        writer.addAttribute("table:style-name", m_style->saveOdf(styles));
    }
    if(m_defaultCellStyle) {
        writer.addAttribute("table:default-cell-style-name", m_defaultCellStyle->saveOdf(styles));
    }
    writer.addAttribute("table:visibility", visibilityMap.value(m_visibility));
}

void KoRow::finishSaveOdf(KoXmlWriter& writer, KoGenStyles& styles)
{
    Q_UNUSED(styles)
    writer.endElement();//table:row
}
