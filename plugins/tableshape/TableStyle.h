/* This file is part of the KDE project
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2008
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
#ifndef TABLESTYLE_H
#define TABLESTYLE_H

#include <KoParagraphStyle.h>
/**
 * The table:style-name attribute references a table style, i.e., an <style:style>
 * element of type“table. The table style describes the formatting properties of
 * the table, such as width and background color. The table style can be either an
 * automatic or common style.
 *
 * A table style consists of a paragraph style (which contains a frame description)
 * and a width in points.
 *
 * Styles can be nested: the style associated with the table contains the width of a
 * whole table, the style associated with a column the width of that column.
 */
class TableStyle : public KoParagraphStyle
{
public:
    TableStyle();

    ~TableStyle();

    float width() const  { return m_width; }
    void setWidth( float width ) { m_width = width; }

private:

    float m_width; // The width in points
};

#endif
