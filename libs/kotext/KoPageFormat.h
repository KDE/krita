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

#ifndef KOPAGEFORMAT_H
#define KOPAGEFORMAT_H

#include "kotext_export.h"

#include <QtGui/QPrinter>

/// The page formats koffice supports
namespace KoPageFormat
{
/**
 * @brief Represents the paper format a document shall be printed on.
 *
 * For compatibility reasons, and because of screen and custom,
 * this enum doesn't map to QPrinter::PageSize but KoPageFormat::printerPageSize
 * does the conversion.
 */
enum Format {
    IsoA3Size,
    IsoA4Size,
    IsoA5Size,
    UsLetterSize,
    UsLegalSize,
    ScreenSize,
    CustomSize,
    IsoB5Size,
    UsExecutiveSize,
    IsoA0Size,
    IsoA1Size,
    IsoA2Size,
    IsoA6Size,
    IsoA7Size,
    IsoA8Size,
    IsoA9Size,
    IsoB0Size,
    IsoB1Size,
    IsoB10Size,
    IsoB2Size,
    IsoB3Size,
    IsoB4Size,
    IsoB6Size,
    IsoC5Size,
    UsComm10Size,
    IsoDLSize,
    UsFolioSize,
    UsLedgerSize,
    UsTabloidSize
};

/**
 *  Represents the orientation of a printed document.
 */
enum Orientation {
    Portrait,
    Landscape
};

/**
 * @brief Convert a Format into a KPrinter::PageSize.
 *
 * If format is 'screen' it will use A4 landscape.
 * If format is 'custom' it will use A4 portrait.
 * (you may want to take care of those cases separately).
 * Usually passed to KPrinter::setPageSize().
 */
KOTEXT_EXPORT QPrinter::PageSize printerPageSize(Format format);

/**
 * Returns the width (in mm) for a given page format and orientation
 * 'Custom' isn't supported by this function, obviously.
 */
KOTEXT_EXPORT qreal width(Format format, Orientation orientation = Landscape);

/**
 * Returns the height (in mm) for a given page format and orientation
 * 'Custom' isn't supported by this function, obviously.
 */
KOTEXT_EXPORT qreal height(Format format, Orientation orientation  = Landscape);

/**
 * Returns the internal name of the given page format.
 * Use for saving.
 */
KOTEXT_EXPORT QString formatString(Format format);

/**
 * Convert a format string (internal name) to a page format value.
 * Use for loading.
 */
KOTEXT_EXPORT Format formatFromString(const QString &string);

/**
 * Returns the default format (based on the KControl settings)
 */
KOTEXT_EXPORT Format defaultFormat();

/**
 * Returns the translated name of the given page format.
 * Use for showing the user.
 */
KOTEXT_EXPORT QString name(Format format);

/**
 * Lists the translated names of all the available formats
 */
KOTEXT_EXPORT QStringList allFormats();

/**
 * Try to find the paper format for the given width and height (in mm).
 * Useful to some import filters.
 */
KOTEXT_EXPORT Format guessFormat(qreal width, qreal height);
}

#endif

