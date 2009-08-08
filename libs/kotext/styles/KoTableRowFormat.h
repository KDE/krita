/* This file is part of the KDE project
 * Copyright (C) 2009 Elvis Stansvik <elvstone@gmail.com>
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
#ifndef KOTABLEROWFORMAT_H
#define KOTABLEROWFORMAT_H

#include "kotext_export.h"

#include "KoTableFormat.h"

#include <QtCore>

class QBrush;

/**
 * Table row format class.
 *
 * This class holds formatting properties for a table row.
 *
 * References:
 *
 * <pre>
 * [ODF] OASIS Open Document Format for Office Applications v1.2 part 1
 *       http://www.oasis-open.org/committees/office/
 * </pre>
 *
 * \sa KoTableFormat, KoTableColumnFormat
 */
class KOTEXT_EXPORT KoTableRowFormat : public KoTableFormat
{
    Q_GADGET
    Q_ENUMS(Property)
public:
    enum Property {
        Background,    /**< Row background. See fo:background-color in [ODF]. */
        MinimumHeight, /**< Minumum row height. See style:min-row-height in [ODF]. */
        KeepTogether,  /**< Keep rows together. See fo:keep-together in [ODF]. */
        BreakAfter,    /**< Break after row. See fo:break-after in [ODF]. */
        BreakBefore   /**< Break before row. See fo:break-before in [ODF]. */
    };

    /// Creates a row format with default row properties.
    KoTableRowFormat();

    /**
     * Set the background property to \a background.
     *
     * @param background the background brush.
     *
     * \sa background()
     */
    void setBackground(const QBrush &background);

    /**
     * Returns the background property.
     *
     * @return the background brush.
     *
     * \sa setBackground()
     */
    QBrush background() const;

    /**
     * Set the minimum height property to \a minimumHeight.
     *
     * @param minimumHeight
     *
     * \sa minimumHeight()
     */
    void setMinimumHeight(qreal minimumHeight);

    /**
     * Returns the minimum height property.
     *
     * @return the minimum height.
     *
     * \sa setMinimumHeight()
     */
    qreal minimumHeight() const;

    /**
     * Set keep together property to \a breakAfter.
     *
     * @param keepTogether \c true to set keep together, \c false to unset.
     *
     * \sa keepTogether()
     */
    void setKeepTogether(bool keepTogether);

    /**
     * Returns the keep together property.
     *
     * @return \c true if keep together is set, otherwise false.
     *
     * \sa setKeepTogether()
     */
    bool keepTogether() const;

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
};

#endif // KOTABLEROWFORMAT_H
