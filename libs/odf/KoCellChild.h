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

#ifndef CELLCHILD_H
#define CELLCHILD_H

class KoCell;
class KoXmlWriter;
class KoGenStyles;

/**
 * A \class KoCellChild represents all the elements that can be contained
 * inside a Cell.
 * 
 * \see ODF1.2 table:table-cell §9.1.4
 */

class KoCellChild
{
    friend class KoCell;

public:
    KoCellChild();
    virtual ~KoCellChild();

protected:
    virtual void saveOdf(KoXmlWriter& writer, KoGenStyles& styles) const =0;
};

#endif
