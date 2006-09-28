/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>, Torben Weis <weis@kde.org>
   Copyright (C) 2004, Nicolas GOUTTE <goutte@kde.org>

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

#ifndef kounit_h
#define kounit_h
#include <QString>
#include <QStringList>
#include <math.h> // for floor
#include <koffice_export.h>

class KoXmlWriter;
class QDomElement;

// 1 inch ^= 72 pt
// 1 inch ^= 25.399956 mm (-pedantic ;p)
// 1 pt = 1/12 pi
// 1 pt ^= 0.0077880997 cc
// 1 cc = 12 dd
// Note: I don't use division but multiplication with the inverse value
// because it's faster ;p (Werner)
#define POINT_TO_MM(px) ((px)*0.352777167)
#define MM_TO_POINT(mm) ((mm)*2.83465058)
#define POINT_TO_CM(px) ((px)*0.0352777167)
#define CM_TO_POINT(cm) ((cm)*28.3465058)
#define POINT_TO_DM(px) ((px)*0.00352777167)
#define DM_TO_POINT(dm) ((dm)*283.465058)
#define POINT_TO_INCH(px) ((px)*0.01388888888889)
#define INCH_TO_POINT(inch) ((inch)*72.0)
#define MM_TO_INCH(mm) ((mm)*0.039370147)
#define INCH_TO_MM(inch) ((inch)*25.399956)
#define POINT_TO_PI(px)((px)*0.083333333)
#define POINT_TO_DD(px)((px)*0.006490083)
#define POINT_TO_CC(px)((px)*0.077880997)
#define PI_TO_POINT(pi)((pi)*12)
#define DD_TO_POINT(dd)((dd)*154.08124)
#define CC_TO_POINT(cc)((cc)*12.840103)
/**
 * %KOffice stores everything in pt (using "double") internally.
 * When displaying a value to the user, the value is converted to the user's unit
 * of choice, and rounded to a reasonable precision to avoid 0.999999
 */
class KOFFICECORE_EXPORT KoUnit
{
public:
    /** Length units supported by KOffice. */
    enum Unit {
        U_MM = 0,
        U_PT = 1,
        U_INCH = 2,
        U_CM = 3,
        U_DM = 4,
        U_PI = 5, // pica
        U_DD = 6, // didot
        U_CC = 7, // cicero
// XXX update U_LASTUNIT and rulers to allow for the new U_PIXEL!
	U_PIXEL = 8,
        U_LASTUNIT = U_CC// update when adding a new unit
        // when adding a new unit, make sure to implement support for it in koRuler too
    };

    /// Prepare ptValue to be displayed in pt
    static double toPoint( double ptValue ) {
        // No conversion, only rounding (to 0.001 precision)
        return floor( ptValue * 1000.0 ) / 1000.0;
    }

    /// Prepare ptValue to be displayed in mm
    static double toMM( double ptValue ) {
        // "mm" values are rounded to 0.0001 millimeters
        return floor( POINT_TO_MM( ptValue ) * 10000.0 ) / 10000.0;
    }

    /// Prepare ptValue to be displayed in cm
    static double toCM( double ptValue ) {
        return floor( POINT_TO_CM( ptValue ) * 10000.0 ) / 10000.0;
    }

    /// Prepare ptValue to be displayed in dm
    static double toDM( double ptValue ) {
        return floor( POINT_TO_DM( ptValue ) * 10000.0 ) / 10000.0;
    }

    /// Prepare ptValue to be displayed in inch
    static double toInch( double ptValue ) {
        // "in" values are rounded to 0.00001 inches
        return floor( POINT_TO_INCH( ptValue ) * 100000.0 ) / 100000.0;
    }

    /// Prepare ptValue to be displayed in pica
    static double toPI( double ptValue ) {
        // "pi" values are rounded to 0.00001 inches
        return floor( POINT_TO_PI( ptValue ) * 100000.0 ) / 100000.0;
    }

    /// Prepare ptValue to be displayed in didot
    static double toDD( double ptValue ) {
        // "dd" values are rounded to 0.00001 inches
        return floor( POINT_TO_DD( ptValue ) * 100000.0 ) / 100000.0;
    }

    /// Prepare ptValue to be displayed in cicero
    static double toCC( double ptValue ) {
        // "cc" values are rounded to 0.00001 inches
        return floor( POINT_TO_CC( ptValue ) * 100000.0 ) / 100000.0;
    }

    /**
     * This method is the one to use to display a value in a dialog
     * \return the value @p ptValue converted to @p unit and rounded, ready to be displayed
     * Old name: ptToUnit
     */
    static double toUserValue( double ptValue, Unit unit );

    /**
     * Convert the value @p ptValue to a given unit @p unit
     * Unlike KoUnit::ptToUnit the return value remains unrounded, so that it can be used in complex calculation
     * \return the converted value
     * Old name: ptToUnitUnrounded
     */
    static double ptToUnit( const double ptValue, const Unit unit );

    /// This method is the one to use to display a value in a dialog
    /// @return the value @p ptValue converted to @p unit and rounded, ready to be displayed
    /// Old name: userValue
    static QString toUserStringValue( double ptValue, Unit unit );

    /// This method is the one to use to read a value from a dialog
    /// @return the value in @p unit, converted to points for internal use
    /// Old name: ptFromUnit
    static double fromUserValue( double value, Unit unit );

    /// This method is the one to use to read a value from a dialog
    /// @param value value entered by the user
    /// @param unit unit type selected by the user
    /// @param ok if set, the pointed bool is set to true if the value could be
    /// converted to a double, and to false otherwise.
    /// @return the value in @p unit, converted to points for internal use
    static double fromUserValue( const QString& value, Unit unit, bool* ok = 0 );

    /// Convert a unit name into a Unit enum
    /// @param _unitName name to convert
    /// @param ok if set, it will be true if the unit was known, false if unknown
    static Unit unit( const QString &_unitName, bool* ok = 0 );
    /// Get the name of a unit
    static QString unitName( Unit _unit );
    /// Get the full (translated) description of a unit
    static QString unitDescription( Unit _unit );
    static QStringList listOfUnitName();

    /// parse common %KOffice and OO values, like "10cm", "5mm" to pt
    static double parseValue( QString value, double defaultVal = 0.0 );
    // Note: the above method doesn't take a const ref, since it modifies the arg.

    /// Save a unit in OASIS format
    static void saveOasis(KoXmlWriter* settingsWriter, Unit _unit);

};


#endif
