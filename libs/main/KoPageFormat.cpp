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
#include "KoPageFormat.h"

#include <klocale.h>

// paper formats ( mm )
#define PG_A3_WIDTH             297.0
#define PG_A3_HEIGHT            420.0
#define PG_A4_WIDTH             210.0
#define PG_A4_HEIGHT            297.0
#define PG_A5_WIDTH             148.0
#define PG_A5_HEIGHT            210.0
#define PG_B5_WIDTH             182.0
#define PG_B5_HEIGHT            257.0
#define PG_US_LETTER_WIDTH      216.0
#define PG_US_LETTER_HEIGHT     279.0
#define PG_US_LEGAL_WIDTH       216.0
#define PG_US_LEGAL_HEIGHT      356.0
#define PG_US_EXECUTIVE_WIDTH   191.0
#define PG_US_EXECUTIVE_HEIGHT  254.0

struct PageFormatInfo
{
    KoPageFormat::Format format;
    KPrinter::PageSize kprinter;
    const char* shortName; // Short name
    const char* descriptiveName; // Full name, which will be translated
    double width; // in mm
    double height; // in mm
};

// NOTES:
// - the width and height of non-ISO formats are rounded
// http://en.wikipedia.org/wiki/Paper_size can help
// - the comments "should be..." indicates the exact values if the inch sizes would be multiplied by 25.4 mm/inch

const PageFormatInfo pageFormatInfo[]=
{
    { KoPageFormat::IsoA3Size,       KPrinter::A3,        "A3",        I18N_NOOP("ISO A3"),       297.0,  420.0 },
    { KoPageFormat::IsoA4Size,       KPrinter::A4,        "A4",        I18N_NOOP("ISO A4"),       210.0,  297.0 },
    { KoPageFormat::IsoA5Size,       KPrinter::A5,        "A5",        I18N_NOOP("ISO A5"),       148.0,  210.0 },
    { KoPageFormat::UsLetterSize,    KPrinter::Letter,    "Letter",    I18N_NOOP("US Letter"),    215.9,  279.4 },
    { KoPageFormat::UsLegalSize,     KPrinter::Legal,     "Legal",     I18N_NOOP("US Legal"),     215.9,  355.6 },
    { KoPageFormat::ScreenSize,      KPrinter::A4,        "Screen",    I18N_NOOP("Screen"), PG_A4_HEIGHT, PG_A4_WIDTH }, // Custom, so fall back to A4
    { KoPageFormat::CustomSize,      KPrinter::A4,        "Custom",    I18N_NOOP2("Custom size", "Custom"), PG_A4_WIDTH, PG_A4_HEIGHT }, // Custom, so fall back to A4
    { KoPageFormat::IsoB5Size,       KPrinter::B5,        "B5",        I18N_NOOP("ISO B5"),       182.0,  257.0 },
    { KoPageFormat::UsExecutiveSize, KPrinter::Executive, "Executive", I18N_NOOP("US Executive"), 191.0,  254.0 }, // should be 190.5 mm x 254.0 mm
    { KoPageFormat::IsoA0Size,       KPrinter::A0,        "A0",        I18N_NOOP("ISO A0"),       841.0, 1189.0 },
    { KoPageFormat::IsoA1Size,       KPrinter::A1,        "A1",        I18N_NOOP("ISO A1"),       594.0,  841.0 },
    { KoPageFormat::IsoA2Size,       KPrinter::A2,        "A2",        I18N_NOOP("ISO A2"),       420.0,  594.0 },
    { KoPageFormat::IsoA6Size,       KPrinter::A6,        "A6",        I18N_NOOP("ISO A6"),       105.0,  148.0 },
    { KoPageFormat::IsoA7Size,       KPrinter::A7,        "A7",        I18N_NOOP("ISO A7"),        74.0,  105.0 },
    { KoPageFormat::IsoA8Size,       KPrinter::A8,        "A8",        I18N_NOOP("ISO A8"),        52.0,   74.0 },
    { KoPageFormat::IsoA9Size,       KPrinter::A9,        "A9",        I18N_NOOP("ISO A9"),        37.0,   52.0 },
    { KoPageFormat::IsoB0Size,       KPrinter::B0,        "B0",        I18N_NOOP("ISO B0"),      1030.0, 1456.0 },
    { KoPageFormat::IsoB1Size,       KPrinter::B1,        "B1",        I18N_NOOP("ISO B1"),       728.0, 1030.0 },
    { KoPageFormat::IsoB10Size,      KPrinter::B10,       "B10",       I18N_NOOP("ISO B10"),       32.0,   45.0 },
    { KoPageFormat::IsoB2Size,       KPrinter::B2,        "B2",        I18N_NOOP("ISO B2"),       515.0,  728.0 },
    { KoPageFormat::IsoB3Size,       KPrinter::B3,        "B3",        I18N_NOOP("ISO B3"),       364.0,  515.0 },
    { KoPageFormat::IsoB4Size,       KPrinter::B4,        "B4",        I18N_NOOP("ISO B4"),       257.0,  364.0 },
    { KoPageFormat::IsoB6Size,       KPrinter::B6,        "B6",        I18N_NOOP("ISO B6"),       128.0,  182.0 },
    { KoPageFormat::IsoC5Size,       KPrinter::C5E,       "C5",        I18N_NOOP("ISO C5"),       163.0,  229.0 }, // Some sources tells: 162 mm x 228 mm
    { KoPageFormat::UsComm10Size,    KPrinter::Comm10E,   "Comm10",    I18N_NOOP("US Common 10"), 105.0,  241.0 }, // should be 104.775 mm x 241.3 mm
    { KoPageFormat::IsoDLSize,       KPrinter::DLE,       "DL",        I18N_NOOP("ISO DL"),       110.0,  220.0 },
    { KoPageFormat::UsFolioSize,     KPrinter::Folio,     "Folio",     I18N_NOOP("US Folio"),     210.0,  330.0 }, // should be 209.54 mm x 330.2 mm
    { KoPageFormat::UsLedgerSize,    KPrinter::Ledger,    "Ledger",    I18N_NOOP("US Ledger"),    432.0,  279.0 }, // should be 431.8 mm x 297.4 mm
    { KoPageFormat::UsTabloidSize,   KPrinter::Tabloid,   "Tabloid",   I18N_NOOP("US Tabloid"),   279.0,  432.0 },  // should be 297.4 mm x 431.8 mm
    { (KoPageFormat::Format) -1,  (KPrinter::PageSize) -1,   "",   "",   -1,  -1 }
};

KPrinter::PageSize KoPageFormat::printerPageSize( KoPageFormat::Format format )
{
    if ( format == ScreenSize )
    {
        kWarning() << "You use the page layout SCREEN. Printing in ISO A4 Landscape." << endl;
        return KPrinter::A4;
    }
    if ( format == CustomSize )
    {
        kWarning() << "The used page layout (Custom) is not supported by KPrinter. Printing in A4." << endl;
        return KPrinter::A4;
    }
    return pageFormatInfo[ format ].kprinter;
}

double KoPageFormat::width( Format format, Orientation orientation )
{
    if ( orientation == Landscape )
        return height( format, Portrait );
    return pageFormatInfo[ format ].width;
}

double KoPageFormat::height( Format format, Orientation orientation )
{
    if ( orientation == Landscape )
        return width( format, Portrait );
    return pageFormatInfo[ format ].height;
}

KoPageFormat::Format KoPageFormat::guessFormat( double width, double height )
{
    for(int i=0; pageFormatInfo[i].format != -1 ;i++)
    {
        // We have some tolerance. 1pt is a third of a mm, this is
        // barely noticeable for a page size.
        if ( qAbs( width - pageFormatInfo[i].width ) < 1.0 && qAbs( height - pageFormatInfo[i].height ) < 1.0 )
            return static_cast<Format>(i);
    }
    return CustomSize;
}

QString KoPageFormat::formatString( Format format )
{
    return QString::fromLatin1( pageFormatInfo[ format ].shortName );
}

KoPageFormat::Format KoPageFormat::formatFromString( const QString & string )
{
    for(int i=0; pageFormatInfo[i].format != -1 ;i++)
    {
        if (string == QString::fromLatin1( pageFormatInfo[ i ].shortName ))
            return pageFormatInfo[ i ].format;
    }
    // We do not know the format name, so we have a custom format
    return CustomSize;
}

KoPageFormat::Format KoPageFormat::defaultFormat()
{
    int kprinter = KGlobal::locale()->pageSize();
    for(int i=0; pageFormatInfo[i].format != -1 ;i++)
    {
        if ( pageFormatInfo[ i ].kprinter == kprinter )
            return static_cast<Format>(i);
    }
    return IsoA4Size;
}

QString KoPageFormat::name( Format format )
{
    return i18n( pageFormatInfo[ format ].descriptiveName );
}

QStringList KoPageFormat::allFormats()
{
    QStringList lst;
    for(int i=0; pageFormatInfo[i].format != -1 ;i++)
    {
        lst << i18n( pageFormatInfo[ i ].descriptiveName );
    }
    return lst;
}
