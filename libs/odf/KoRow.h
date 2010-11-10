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

#ifndef KOROW_H
#define KOROW_H

class KoTable;
#include "KoCellStyle.h"
#include "KoRowStyle.h"

#include "koodf_export.h"

class KoGenStyles;
class KoXmlWriter;

/**
 * A \class KoRow represents a row inside a table.
 * its properties aren't shared unlike the KoStyle's it contains.
 */

class KOODF_EXPORT KoRow
{
    friend class KoTable;
    KoRow();

public:
    ~KoRow();

    KoRowStyle::Ptr style();
    void setStyle(KoRowStyle::Ptr style);

    KoCellStyle::Ptr defualtCellStyle() const;
    void setDefaultCellStyle(KoCellStyle::Ptr defaultStyle);

    enum Visibility {
        Collapse,
        Filter,
        Visible
    };
    Visibility visibility();
    void setVisibility(Visibility visibility);

private:
    void saveOdf(KoXmlWriter& writer, KoGenStyles& styles);
    void finishSaveOdf(KoXmlWriter& writer, KoGenStyles& styles);

    KoRow(const KoRow&);
    KoRow& operator=(const KoRow&);

    KoCellStyle::Ptr m_defaultCellStyle;
    KoRowStyle::Ptr m_style;
    Visibility m_visibility;
};

#endif
