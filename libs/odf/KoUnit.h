/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
   Copyright (C) 2004, Nicolas GOUTTE <goutte@kde.org>
   Copyright (C) 2010 Thomas Zander <zander@kde.org>
   Copyright 2012 Friedrich W. H. Kossebau <kossebau@kde.org>

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

#ifndef KOUNIT_H
#define KOUNIT_H

// Calligra
#include "kritaodf_export.h"
// Qt
#include <QString>
#include <QDebug>
#include <QMetaType>
// std
#include <math.h> // for floor

class QStringList;

// 1 inch ^= 72 pt
// 1 inch ^= 25.399956 mm (-pedantic ;p)
// 1 pt = 1/12 pi
// 1 pt ^= 0.0077880997 cc
// 1 cc = 12 dd

constexpr qreal POINT_TO_MM(qreal px) {return (px)*0.352777167;}
constexpr qreal MM_TO_POINT(qreal mm) {return mm*2.83465058;}
constexpr qreal POINT_TO_CM(qreal px) {return (px)*0.0352777167;}
constexpr qreal CM_TO_POINT(qreal cm) {return (cm)*28.3465058;}
constexpr qreal POINT_TO_DM(qreal px) {return (px)*0.00352777167;}
constexpr qreal DM_TO_POINT(qreal dm) {return (dm)*283.465058;}
constexpr qreal POINT_TO_INCH(qreal px) {return (px)*0.01388888888889;}
constexpr qreal INCH_TO_POINT(qreal inch) {return (inch)*72.0;}
constexpr qreal MM_TO_INCH(qreal mm) {return (mm)*0.039370147;}
constexpr qreal INCH_TO_MM(qreal inch) {return (inch)*25.399956;}
constexpr qreal POINT_TO_PI(qreal px) {return (px)*0.083333333;}
constexpr qreal POINT_TO_CC(qreal px) {return (px)*0.077880997;}
constexpr qreal PI_TO_POINT(qreal pi) {return (pi)*12;}
constexpr qreal CC_TO_POINT(qreal cc) {return (cc)*12.840103;}


static const qreal PT_ROUNDING {1000.0};
//static const qreal PX_ROUNDING {1000.0}; // pixel value is not to be rounded

static const qreal CM_ROUNDING {10000.0};
static const qreal DM_ROUNDING {10000.0};
static const qreal MM_ROUNDING {10000.0};

static const qreal IN_ROUNDING {100000.0};

static const qreal PI_ROUNDING {100000.0}; // pico
static const qreal CC_ROUNDING {100000.0}; // cicero


/**
 * %Calligra stores everything in pt (using "qreal") internally.
 * When displaying a value to the user, the value is converted to the user's unit
 * of choice, and rounded to a reasonable precision to avoid 0.999999
 *
 * For implementing the selection of a unit type in the UI use the *ForUi() methods.
 * They ensure the same order of the unit types in all places, with the order not
 * bound to the order in the enum (so ABI-compatible extension is possible) and
 * with the order and scope of listed types controlled by the @c ListOptions parameter.
 */
class KRITAODF_EXPORT KoUnit
{
public:
    /** Length units supported by Calligra. */
    enum Type {
        Millimeter = 0,
        Point,  ///< Postscript point, 1/72th of an Inco
        Inch,
        Centimeter,
        Decimeter,
        Pica,
        Cicero,
        Pixel,
        TypeCount ///< @internal
    };

    /// Used to control the scope of the unit types listed in the UI
    enum ListOption {
        ListAll = 0,
        HidePixel = 1,
        HideMask = HidePixel
    };
     Q_DECLARE_FLAGS(ListOptions, ListOption)

    /** Returns a KoUnit instance with the type at the @p index of the UI list with the given @p listOptions. */
    static KoUnit fromListForUi(int index, ListOptions listOptions = ListAll, qreal factor = 1.0);

    /// Convert a unit symbol string into a KoUnit
    /// @param symbol symbol to convert
    /// @param ok if set, it will be true if the unit was known, false if unknown
    static KoUnit fromSymbol(const QString &symbol, bool *ok = 0);

    /** Construction requires initialization. The factor is for variable factor units like pixel */
    explicit KoUnit(Type unit = Point, qreal factor = 1.0) {
        m_type = unit;
        m_pixelConversion = factor;
    }

    KoUnit& operator=(Type unit) {
        m_type = unit; m_pixelConversion = 1.0; return *this;
    }

    bool operator!=(const KoUnit &other) const {
        return !operator==(other);
    }

    bool operator==(const KoUnit &other) const {
        return m_type == other.m_type &&
            (m_type != Pixel ||
             qFuzzyCompare(m_pixelConversion, other.m_pixelConversion));
    }

    KoUnit::Type type() const {
        return m_type;
    }

    void setFactor(qreal factor) {
        m_pixelConversion = factor;
    }


    /**
     * convert the given value directly from one unit to another
     */
    static qreal convertFromUnitToUnit(const qreal value, const KoUnit &fromUnit, const KoUnit &toUnit, qreal factor = 1.0);

    /**
     * Convert the value @p ptValue to the unit and round it.
     * This method is meant to be used to display a value in a dialog.
     * \return the converted and rounded value
     */

    qreal toUserValueRounded(const qreal value) const;
    /**
     * Convert the value @p ptValue to the unit (without rounding)
     * This method is meant to be used in complex calculations.
     * \return the converted value
     */
    qreal toUserValuePrecise(const qreal ptValue) const;

    /**
     * Convert the value @p ptValue with or with rounding as indicated by @p rounding.
     * This method is a proxy to toUserValuePrecise and toUserValueRounded.
     * \return the value @p ptValue converted to unit
     */
    qreal toUserValue(qreal ptValue, bool rounding = true) const;



    /// This method is the one to use to display a value in a dialog
    /// @return the value @p ptValue converted the unit and rounded, ready to be displayed
    QString toUserStringValue(qreal ptValue) const;

    /// This method is the one to use to read a value from a dialog
    /// @return the value converted to points for internal use
    qreal fromUserValue(qreal value) const;

    /// This method is the one to use to read a value from a dialog
    /// @param value value entered by the user
    /// @param ok if set, the pointed bool is set to true if the value could be
    /// converted to a qreal, and to false otherwise.
    /// @return the value converted to points for internal use
    qreal fromUserValue(const QString &value, bool *ok = 0) const;

    /// Get the description string of the given unit
    static QString unitDescription(KoUnit::Type type);

    /// Get the symbol string of the unit
    QString symbol() const;

    /// Returns the list of unit types for the UI, controlled with the given @p listOptions.
    static QStringList listOfUnitNameForUi(ListOptions listOptions = ListAll);

    /// Get the index of this unit in the list of unit types for the UI,
    /// if it is controlled with the given @p listOptions.
    int indexInListForUi(ListOptions listOptions = ListAll) const;

    /// parse common %Calligra and Odf values, like "10cm", "5mm" to pt
    static qreal parseValue(const QString &value, qreal defaultVal = 0.0);

    /// parse an angle to its value in degrees
    static qreal parseAngle(const QString &value, qreal defaultVal = 0.0);

    QString toString() const {
        return symbol();
    }

    /**
     * Get an approximate scale of a unit vector that was converted by
     * the transformation.
     *
     * Please note that exact values are guaranteed only for
     * combinations of Translate, Rotation and Uniform Scale
     * matrices. For combinations having shears and perspective the
     * value will be average for the point near CS origin.
     */
    static qreal approxTransformScale(const QTransform &t);

    /**
     * Adjust the unit by pixel transformation applied to the
     * describing object. It multiplies the pixel coefficient by the
     * average scale of the matrix.
     */
    void adjustByPixelTransform(const QTransform &t);

private:
    Type m_type;
    qreal m_pixelConversion;
};

#ifndef QT_NO_DEBUG_STREAM
KRITAODF_EXPORT QDebug operator<<(QDebug, const KoUnit &);
#endif

Q_DECLARE_METATYPE(KoUnit)
Q_DECLARE_OPERATORS_FOR_FLAGS(KoUnit::ListOptions)

#endif
