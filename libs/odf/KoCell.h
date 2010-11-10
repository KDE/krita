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

#ifndef kOCELL_H
#define KOCELL_H

#include "KoCellStyle.h"
class KoCellChild;
class KoCellValue;
class KoTable;

class KoXmlWriter;
class KoGenStyles;

#include "koodf_export.h"

#include <QList>

/**
 * A \class KoCell represents a cell inside a table.
 *
 **/
class KOODF_EXPORT KoCell
{
    friend class KoTable;

    KoCell();
    KoCell& operator=(KoCell&);

public:
    ~KoCell();

<<<<<<< HEAD
    void setStyle(KoCellStyle* style);
    KoCellStyle* style() const;
=======
    /**
     * Set the this Cell style to the given style.
     */
    void setStyle(KoCellStyle::Ptr style);

    /**
     * Returns the style assigned to this KoCell.
     * It's null if no style has been set.
     */
    KoCellStyle::Ptr style() const;
>>>>>>> c56441c... Fix KoCell.

    void setValue(KoCellValue* value);
    KoCellValue* value() const;

    void appendChild(KoCellChild* child);

    /**
     *  Deletes the old list of children in favor of this one.
     */
    void setChildren(QList<KoCellChild*> children);

    /**
     * Returns all the children appended to the cell.
     * \note the cell is still the owner of those objects
     */
    QList<KoCellChild*> children() const;

    /**
     * \brief Sets the row span of the cell.
     * By default the span is 1.
     * Negative spans are not allowed, they are converted back to 1.
     *
     * \note the span is not checked to ensure not overlaping.
     * Also the covered cells can also be populated with data.
     */
    void setRowSpan(int span);

    /**
     * Gets the row span of the cell.
     * \see ODF1.2 table:number-rows-spanned §19.680
     */
    int rowSpan() const;

    /**
     * \brief Sets the column span of the cell.
     * By default the span is 1.
     * Negative spans are not allowed, they are converted back to 1.
     *
     * \note the span is not checked to ensure not overlaping.
     * Also the covered cells can also be populated with data.
     */
    void setColumnSpan(int span);

    /**
     * Gets the column span of the cell.
     * \see ODF1.2 table:number-columns-spanned §19.681
     */
    int columnSpan() const;

    /**
     * Sets if the cell is protected. Protected cells cannot be edited by the user.
     */
    void setProtected(bool protect);

    /**
     * Returns if the cell is protected or not.
     * \see ODF1.2 table:protected §19.698.5
     */
    bool isProtected() const;

private:
    void saveOdf(KoXmlWriter& writer, KoGenStyles& styles);

    QList<KoCellChild*> m_children;
    KoCellValue* m_value;
    KoCellStyle::Ptr m_style;

    int m_rowSpan;
    int m_columnSpan;
    bool m_protected;
};

#endif
