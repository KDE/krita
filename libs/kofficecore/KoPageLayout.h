/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
   Copyright 2002, 2003 David Faure <faure@kde.org>
   Copyright 2003 Nicolas GOUTTE <goutte@kde.org>

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

#include <KoGenStyles.h>
#include <QStringList>
#include <koffice_export.h>
#include <KoXmlReader.h>

class QDomElement;

/**
 * @brief Represents the paper format a document shall be printed on.
 *
 * For compatibility reasons, and because of screen and custom,
 * this enum doesn't map to QPrinter::PageSize but KoPageFormat::printerPageSize
 * does the conversion.
 *
 * @todo convert DIN to ISO in the names
 */
enum KoFormat {
    PG_DIN_A3 = 0,
    PG_DIN_A4 = 1,
    PG_DIN_A5 = 2,
    PG_US_LETTER = 3,
    PG_US_LEGAL = 4,
    PG_SCREEN = 5,
    PG_CUSTOM = 6,
    PG_DIN_B5 = 7,
    PG_US_EXECUTIVE = 8,
    PG_DIN_A0 = 9,
    PG_DIN_A1 = 10,
    PG_DIN_A2 = 11,
    PG_DIN_A6 = 12,
    PG_DIN_A7 = 13,
    PG_DIN_A8 = 14,
    PG_DIN_A9 = 15,
    PG_DIN_B0 = 16,
    PG_DIN_B1 = 17,
    PG_DIN_B10 = 18,
    PG_DIN_B2 = 19,
    PG_DIN_B3 = 20,
    PG_DIN_B4 = 21,
    PG_DIN_B6 = 22,
    PG_ISO_C5 = 23,
    PG_US_COMM10 = 24,
    PG_ISO_DL = 25,
    PG_US_FOLIO = 26,
    PG_US_LEDGER = 27,
    PG_US_TABLOID = 28,
    // update the number below and the static arrays if you add more values to the enum
    PG_LAST_FORMAT = PG_US_TABLOID // used by koPageLayout.cpp
};

/**
 *  Represents the orientation of a printed document.
 */
enum KoOrientation {
    PG_PORTRAIT = 0,
    PG_LANDSCAPE = 1
};

namespace KoPageFormat
{
    /**
     * @brief Convert a KoFormat into a KPrinter::PageSize.
     *
     * If format is 'screen' it will use A4 landscape.
     * If format is 'custom' it will use A4 portrait.
     * (you may want to take care of those cases separately).
     * Usually passed to KPrinter::setPageSize().
     *
     * @note We return int instead of the enum to avoid including kprinter.h
     */
    KOFFICECORE_EXPORT int /*KPrinter::PageSize*/ printerPageSize( KoFormat format );

    /**
     * Returns the width (in mm) for a given page format and orientation
     * 'Custom' isn't supported by this function, obviously.
     */
    KOFFICECORE_EXPORT double width( KoFormat format, KoOrientation orientation );

    /**
     * Returns the height (in mm) for a given page format and orientation
     * 'Custom' isn't supported by this function, obviously.
     */
    KOFFICECORE_EXPORT double height( KoFormat format, KoOrientation orientation );

    /**
     * Returns the internal name of the given page format.
     * Use for saving.
     */
    KOFFICECORE_EXPORT QString formatString( KoFormat format );

    /**
     * Convert a format string (internal name) to a page format value.
     * Use for loading.
     */
    KOFFICECORE_EXPORT KoFormat formatFromString( const QString & string );

    /**
     * Returns the default format (based on the KControl settings)
     */
    KOFFICECORE_EXPORT KoFormat defaultFormat();

    /**
     * Returns the translated name of the given page format.
     * Use for showing the user.
     */
    KOFFICECORE_EXPORT QString name( KoFormat format );

    /**
     * Lists the translated names of all the available formats
     */
    KOFFICECORE_EXPORT QStringList allFormats();

    /**
     * Try to find the paper format for the given width and height (in mm).
     * Useful to some import filters.
     */
    KOFFICECORE_EXPORT KoFormat guessFormat( double width, double height );
}


/**
 * @brief Header/Footer type.
 *
 * @note Yes, this should have been a bitfield, but there was only 0, 2, 3 in koffice-1.0. Don't ask why.
 * In the long run this should be replaced with a more flexible repetition/section concept.
 */
enum KoHFType {
    HF_SAME = 0,            ///< 0: Header/Footer is the same on all pages
    HF_FIRST_EO_DIFF = 1,   ///< 1: Header/Footer is different on first, even and odd pages (2&3)
    HF_FIRST_DIFF = 2,      ///< 2: Header/Footer for the first page differs
    HF_EO_DIFF = 3          ///< 3: Header/Footer for even - odd pages are different
};

/**
 * This structure defines the page layout, including
 * its size in pt, its format (e.g. A4), orientation, unit, margins etc.
 */
struct KoPageLayout
{
    /** Page format */
    KoFormat format;
    /** Page orientation */
    KoOrientation orientation;

    /** Page width in pt */
    double ptWidth;
    /** Page height in pt */
    double ptHeight;
    /** Left margin in pt */
    double ptLeft;
    /** Right margin in pt */
    double ptRight;
    /** Top margin in pt */
    double ptTop;
    /** Bottom margin in pt */
    double ptBottom;
    /// margin on page edge
    double ptPageEdge;
    /// margin on page-binding edge
    double ptBindingSide;

    bool operator==( const KoPageLayout& l ) const {
       return ( ptWidth == l.ptWidth &&
                ptHeight == l.ptHeight &&
                ptLeft == l.ptLeft &&
                ptRight == l.ptRight &&
                ptTop == l.ptTop &&
                ptBottom == l.ptBottom &&
                ptPageEdge == l.ptPageEdge &&
                ptBindingSide == l.ptBindingSide);
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
     * @since 1.4
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
    double ptColumnSpacing;
    bool operator==( const KoColumns& rhs ) const {
        return columns == rhs.columns &&
               qAbs(ptColumnSpacing - rhs.ptColumnSpacing) <= 1E-10;
    }
    bool operator!=( const KoColumns& rhs ) const {
        return columns != rhs.columns ||
               qAbs(ptColumnSpacing - rhs.ptColumnSpacing) > 1E-10;
    }
};

/** structure for KWord header-footer */
struct KoKWHeaderFooter
{
    KoHFType header;
    KoHFType footer;
    double ptHeaderBodySpacing;
    double ptFooterBodySpacing;
    double ptFootNoteBodySpacing;
    bool operator==( const KoKWHeaderFooter& rhs ) const {
        return header == rhs.header && footer == rhs.footer &&
               qAbs(ptHeaderBodySpacing - rhs.ptHeaderBodySpacing) <= 1E-10 &&
               qAbs(ptFooterBodySpacing - rhs.ptFooterBodySpacing) <= 1E-10 &&
               qAbs(ptFootNoteBodySpacing - rhs.ptFootNoteBodySpacing) <= 1E-10;
    }
    bool operator!=( const KoKWHeaderFooter& rhs ) const {
        return !( *this == rhs );
    }
};

#endif /* KOPAGELAYOUT_H */

