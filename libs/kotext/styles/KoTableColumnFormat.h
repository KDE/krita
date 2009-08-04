/* This file is part of the KDE project
 * Copyright (C) 2009 Elvis Stansvik <elvstone@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; version 2.
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
#ifndef KOTABLECOLUMNFORMAT_H
#define KOTABLECOLUMNFORMAT_H

#include "kotext_export.h"

#include "KoTableFormat.h"

#include <QtCore>

/**
 * Table column format class.
 *
 * This class holds formatting properties for a table column.
 *
 * References:
 *
 * <pre>
 * [ODF] OASIS Open Document Format for Office Applications v1.2 part 1
 *       http://www.oasis-open.org/committees/office/
 * [XSL] Extensible Stylesheet Language (XSL)
 *       http://www.w3.org/TR/2001/REC-xsl-20011015/
 * </pre>
 *
 * \sa KoTableFormat, KoTableRowFormat
 */
class KOTEXT_EXPORT KoTableColumnFormat : public KoTableFormat
{
    Q_GADGET
    Q_ENUMS(Property)
public:
    enum Property {
        BreakAfter,         /**< See 18.176 of [ODF] and §7.19.2 of [XSL]. */
        BreakBefore,        /**< See 18.177 of [ODF] and §7.19.2 of [XSL]. */
        ColumnWidth,        /**< See 18.245 of [ODF]. */
        RelativeColumnWidth /**< See 18.328 of [ODF]. */
    };

    /// Creates a column format with default column properties.
    KoTableColumnFormat();

    /**
     * Set break after property to \a breakAfter.
     *
     * @param breakAfter \c true to set break after, \c false to unset.
     *
     * \sa breakAfter()
     */
    void setBreakAfter(bool breakAfter);

    /**
     * Returns the break after property.
     *
     * @return \c true if break after is set, otherwise false.
     *
     * \sa setBreakAfter()
     */
    bool breakAfter() const;

    /**
     * Set break before property to \a breakAfter.
     *
     * @param breakBefore \c true to set break before, \c false to unset.
     *
     * \sa breakBefore()
     */
    void setBreakBefore(bool breakBefore);

    /**
     * Returns the break before property.
     *
     * @return \c true if break before is set, otherwise \c false.
     *
     * \sa setBreakBefore()
     */
    bool breakBefore() const;

    /**
     * Set column width property to \a columnWidth.
     *
     * @param columnWidth the column width.
     *
     * \sa columnWidth()
     */
    void setColumnWidth(qreal columnWidth);

    /**
     * Returns the column width property.
     *
     * @return the column width.
     *
     * \sa setColumnWidth()
     */
    qreal columnWidth() const;

    /**
     * Set relative column width property to \a relativeColumnWidth.
     *
     * @param relativeColumnWidth the relative column width.
     *
     * \sa relativeColumnWidth()
     */
    void setRelativeColumnWidth(qreal relativeColumnWidth);

    /**
     * Returns the relative column width property.
     *
     * @return the relative column width.
     *
     * \sa setRelativeColumnWidth()
     */
    qreal relativeColumnWidth() const;

    /// Returns \c true if this format is valid, otherwise \c false.
    bool isValid() const;
};

#endif // KOTABLECOLUMNFORMAT_H
