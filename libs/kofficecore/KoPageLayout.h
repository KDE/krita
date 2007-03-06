/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
   Copyright 2002, 2003 David Faure <faure@kde.org>
   Copyright 2003 Nicolas GOUTTE <goutte@kde.org>
   Copyright 2007 Thomas Zander <zander@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef KOPAGELAYOUT_H
#define KOPAGELAYOUT_H

#include "KoGenStyles.h"
#include "KoPageFormat.h"
#include "kofficecore_export.h"
#include "KoXmlReader.h"

/**
 * This structure defines the page layout, including
 * its size in points, its format (e.g. A4), orientation, unit, margins etc.
 */
struct KoPageLayout
{
    /** Page format */
    KoPageFormat::Format format;
    /** Page orientation */
    KoPageFormat::Orientation orientation;

    /** Page width in points */
    double width;
    /** Page height in points */
    double height;
    /** Left margin in points */
    double left;
    /** Right margin in points */
    double right;
    /** Top margin in points */
    double top;
    /** Bottom margin in points */
    double bottom;
    /// margin on page edge
    double pageEdge;
    /// margin on page-binding edge
    double bindingSide;

    bool operator==( const KoPageLayout& l ) const {
       return ( width == l.width &&
                height == l.height &&
                left == l.left &&
                right == l.right &&
                top == l.top &&
                bottom == l.bottom &&
                pageEdge == l.pageEdge &&
                bindingSide == l.bindingSide);
    }
    bool operator!=( const KoPageLayout& l ) const {
        return !( (*this) == l );
    }

    /**
     * Save this page layout to OASIS.
     */
    KOFFICECORE_EXPORT KoGenStyle saveOasis() const;

    /**
     * Load this page layout from OASIS
     */
    KOFFICECORE_EXPORT void loadOasis(const KoXmlElement &style);

    /**
     * @return a page layout with the default page size depending on the locale settings,
     * default margins (2 cm), and portrait orientation.
     */
    static KOFFICECORE_EXPORT KoPageLayout standardLayout();
};

/** structure for header-footer */
struct KoHeadFoot
{
    QString headLeft;
    QString headMid;
    QString headRight;
    QString footLeft;
    QString footMid;
    QString footRight;
};

/** structure for columns */
struct KoColumns
{
    int columns;
    double columnSpacing;
    bool operator==( const KoColumns& rhs ) const {
        return columns == rhs.columns &&
               qAbs(columnSpacing - rhs.columnSpacing) <= 1E-10;
    }
    bool operator!=( const KoColumns& rhs ) const {
        return columns != rhs.columns ||
               qAbs(columnSpacing - rhs.columnSpacing) > 1E-10;
    }
};

#endif /* KOPAGELAYOUT_H */

