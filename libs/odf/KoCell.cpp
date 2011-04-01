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

#include "KoCell.h"
#include "KoDummyCellValue.h"
#include "KoCellChild.h"
#include "KoCellStyle.h"

#include <KoXmlWriter.h>

KoCell::KoCell()
: m_value(new KoDummyCellValue)
, m_style(0)
, m_rowSpan(1)
, m_columnSpan(1)
, m_protected(false)
, m_covered(false)
{
}

KoCell::~KoCell()
{
    delete m_value;
    qDeleteAll(m_children);
}

void KoCell::saveOdf(KoXmlWriter& writer, KoGenStyles& styles)
{
    if (m_covered) {
        writer.startElement("table:covered-table-cell");
        writer.endElement(); // table:covered-table-cell
    }
    else {
        writer.startElement("table:table-cell");
        m_value->saveOdf(writer);
        if(m_style) {
            writer.addAttribute("table:style-name", m_style->saveOdf(styles));
        }
        if(m_columnSpan > 1) {
            writer.addAttribute("table:number-columns-spanned", m_columnSpan);
        }
        if(m_rowSpan > 1) {
            writer.addAttribute("table:number-rows-spanned", m_rowSpan);
        }
        writer.addAttribute("table:protected", m_protected? "true" : "false" );

        foreach(KoCellChild* child, m_children){
            child->saveOdf(writer, styles);
        }

        writer.endElement();//table:table-cell
    }
}

KoCellValue* KoCell::value() const
{
    return m_value;
}

void KoCell::setValue(KoCellValue* value)
{
    delete m_value;
    m_value = value;
}

KoCellStyle::Ptr KoCell::style() const
{
    return m_style;
}

void KoCell::setStyle(KoCellStyle::Ptr style)
{
    m_style = style;
}

QList< KoCellChild* > KoCell::children() const
{
    return m_children;
}

void KoCell::setChildren(QList< KoCellChild* > children)
{
    qDeleteAll(m_children);
    m_children = children;
}

void KoCell::appendChild(KoCellChild* child)
{
    m_children.append(child);
}

int KoCell::columnSpan() const
{
    return m_columnSpan;
}

void KoCell::setColumnSpan(int span)
{
    m_columnSpan = qMax(1, span);
}

int KoCell::rowSpan() const
{
    return m_rowSpan;
}

void KoCell::setRowSpan(int span)
{
    m_rowSpan = qMax(1, span);
}

bool KoCell::isProtected() const
{
    return m_protected;
}

bool KoCell::isCovered() const
{
    return m_covered;
}

void KoCell::setCovered(bool covered)
{
    m_covered = covered;
}

void KoCell::setProtected(bool protect)
{
    m_protected = protect;
}
