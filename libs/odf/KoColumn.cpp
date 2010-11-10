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

#include "KoColumn.h"
#include "KoColumnStyle.h"
#include "KoCellStyle.h"

#include <KoXmlWriter.h>

#include <QString>

namespace {
    class VisibilityMap : public QMap<KoColumn::Visibility, QString> {
    public:
        VisibilityMap()
        : QMap<KoColumn::Visibility, QString>()
        {
            insert(KoColumn::Collapse, "colapse");
            insert(KoColumn::Filter, "filter");
            insert(KoColumn::Visible, "visible");
        }
    } visibilityMap;
}

KoColumn::KoColumn()
: m_defaultCellStyle(0)
, m_style(0)
, m_visibility(Visible)
{
}

KoColumn::~KoColumn()
{
}

void KoColumn::setStyle(KoColumnStyle::Ptr style)
{
    m_style = style;
}

KoColumnStyle::Ptr KoColumn::style()
{
    return m_style;
}

KoCellStyle::Ptr KoColumn::defualtCellStyle() const
{
    return m_defaultCellStyle;
}

void KoColumn::setDefaultCellStyle(KoCellStyle::Ptr defaultStyle)
{
    m_defaultCellStyle = defaultStyle;
}

void KoColumn::setVisibility(KoColumn::Visibility visibility)
{
    m_visibility = visibility;
}

KoColumn::Visibility KoColumn::visibility()
{
    return m_visibility;
}

void KoColumn::saveOdf(KoXmlWriter& writer, KoGenStyles& styles)
{
    writer.startElement("table:table-column");
    if(m_style) {
        writer.addAttribute("table:style-name", m_style->saveOdf(styles));
    }
    if(m_defaultCellStyle) {
        writer.addAttribute("table:default-cell-style-name", m_defaultCellStyle->saveOdf(styles));
    }
    writer.addAttribute("table:visibility", visibilityMap.value(m_visibility));
    writer.endElement();//table:column
}
