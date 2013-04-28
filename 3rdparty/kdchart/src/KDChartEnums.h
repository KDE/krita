/****************************************************************************
** Copyright (C) 2001-2010 Klaralvdalens Datakonsult AB.  All rights reserved.
**
** This file is part of the KD Chart library.
**
** Licensees holding valid commercial KD Chart licenses may use this file in
** accordance with the KD Chart Commercial License Agreement provided with
** the Software.
**
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 and version 3 as published by the
** Free Software Foundation and appearing in the file LICENSE.GPL included.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** Contact info@kdab.com if any conditions of this licensing are not
** clear to you.
**
**********************************************************************/

#ifndef __KDCHARTENUMS_H__
#define __KDCHARTENUMS_H__

#include "KDChartGlobal.h"

#include <QRectF>
#include <QObject>
#include <QVector>

/** \file KDChartEnums.h
  \brief Definition of global enums.
  */

/**
  Project global class providing some enums needed both by KDChartParams
  and by KDChartCustomBox.
  */
class KDCHART_EXPORT KDChartEnums :public QObject
{
    Q_OBJECT
    Q_ENUMS( TextLayoutPolicy )
    Q_ENUMS( AreaName )
    Q_ENUMS( PositionFlag )

public:
    /**
      GranularitySequence specifies the values, that may be applied,
      to determine a step width within a given data range.

      \note Granularity with can be set for Linear axis calculation mode only,
      there is no way to specify a step width for Logarithmic axes.

      Value occurring in the GranularitySequence names only are showing
      their respective relation ship.  For real data they will most times not
      be used directly, but be multiplied by positive (or negative, resp.)
      powers of ten.

      A granularity sequence is a sequence of values from the following set:
      1, 1.25, 2, 2.5, 5.

      The reason for using one of the following three pre-defined granularity
      sequences (instead of just using the best matching step width) is to
      follow a simple rule: If scaling becomes finer (== smaller step width)
      no value, that has been on a grid line before, shall loose its line
      and be NOT on a grid line anymore!

      This means: Smaller step width may not remove any grid lines, but it
      may add additional lines in between.

      \li \c GranularitySequence_10_20 Step widths can be 1, or 2, but they never can be 2.5 nor 5, nor 1.25.
      \li \c GranularitySequence_10_50 Step widths can be 1, or 5, but they never can be 2, nor 2.5, nor 1.25.
      \li \c GranularitySequence_25_50 Step widths can be 2.5, or 5, but they never can be 1, nor 2, nor 1.25.
      \li \c GranularitySequence_125_25 Step widths can be 1.25 or 2.5 but they never can be 1, nor 2, nor 5.
      \li \c GranularitySequenceIrregular Step widths can be all of these values: 1, or 1.25, or 2, or 2.5, or 5.

      \note When ever possible, try to avoid using GranularitySequenceIrregular!
      Allowing all possible step values, using this granularity sequence involves a
      serious risk: Your users might be irritated due to 'jumping' grid lines, when step size
      is changed from 2.5 to 2 (or vice versa, resp.).
      In case you still want to use GranularitySequenceIrregular just make sure to NOT draw
      any sub-grid lines, because in most cases you will get not-matching step widths for
      the sub-grid.
      In short: GranularitySequenceIrregular can safely be used if your data range is not
      changing at all AND (b) you will not allow the coordinate plane to be zoomed
      AND (c) you are not displaying any sub-grid lines.

      Since you probably like having the value 1 as an allowed step width,
      the granularity sequence decision boils down to a boolean question:
      \li To get ten divided by five you use GranularitySequence_10_20, while
      \li for having it divided by two GranularitySequence_10_50 is your choice.

      */
    enum GranularitySequence {
        GranularitySequence_10_20,
        GranularitySequence_10_50,
        GranularitySequence_25_50,
        GranularitySequence_125_25,
        GranularitySequenceIrregular };

    /**
      Converts the specified granularity sequence enum to a
      string representation.

      \param sequence the granularity sequence enum to convert
      \return the string representation of the granularity sequence
      */
    static QString granularitySequenceToString( GranularitySequence sequence ) {
        switch( sequence ) {
            case GranularitySequence_10_20:
                return QString::fromLatin1("GranularitySequence_10_20");
            case GranularitySequence_10_50:
                return QString::fromLatin1("GranularitySequence_10_50");
            case GranularitySequence_25_50:
                return QString::fromLatin1("GranularitySequence_25_50");
            case GranularitySequence_125_25:
                return QString::fromLatin1("GranularitySequence_125_25");
            case GranularitySequenceIrregular:
                return QString::fromLatin1("GranularitySequenceIrregular");
            default: // should not happen
        qDebug( "Unknown granularity sequence" );
        return QString::fromLatin1("GranularitySequence_10_20");
        }
    }


    /**
      Converts the specified string to a granularity sequence enum value.

      \param string the string to convert
      \return the granularity sequence enum value
      */
    static GranularitySequence stringToGranularitySequence( const QString& string ) {
      if( string == QString::fromLatin1("GranularitySequence_10_20") )
          return GranularitySequence_10_20;
      if( string == QString::fromLatin1("GranularitySequence_10_50") )
          return GranularitySequence_10_50;
      if( string == QString::fromLatin1("GranularitySequence_25_50") )
          return GranularitySequence_25_50;
      if( string == QString::fromLatin1("GranularitySequence_125") )
          return GranularitySequence_125_25;
      if( string == QString::fromLatin1("GranularitySequenceIrregular") )
          return GranularitySequenceIrregular;
      // default, should not happen
      return GranularitySequence_10_20;
    }


    /**
      Text layout policy: what to do if text that is to be drawn would
      cover neighboring text or neighboring areas.

      \li \c LayoutJustOverwrite Just ignore the layout collision and write the text nevertheless.
      \li \c LayoutPolicyRotate Try counter-clockwise rotation to make the text fit into the space.
      \li \c LayoutPolicyShiftVertically Shift the text baseline upwards (or downwards, resp.) and draw a connector line between the text and its anchor.
      \li \c LayoutPolicyShiftHorizontally Shift the text baseline to the left (or to the right, resp.) and draw a connector line between the text and its anchor.
      \li \c LayoutPolicyShrinkFontSize Reduce the text font size.

      \sa KDChartParams::setPrintDataValues
      */
    enum TextLayoutPolicy { LayoutJustOverwrite,
        LayoutPolicyRotate,
        LayoutPolicyShiftVertically,
        LayoutPolicyShiftHorizontally,
        LayoutPolicyShrinkFontSize };

    /**
      Converts the specified text layout policy enum to a
      string representation.

      \param type the text layout policy to convert
      \return the string representation of the text layout policy enum
      */
    static QString layoutPolicyToString( TextLayoutPolicy type );


    /**
      Converts the specified string to a text layout policy enum value.

      \param string the string to convert
      \return the text layout policy enum value
      */
    static TextLayoutPolicy stringToLayoutPolicy( const QString& string );


    /**
        Numerical values of the static KDChart::Position instances,
        for using a Position::value() with a switch() statement.

        \sa Position
    */
    enum PositionValue {
        PositionUnknown   = 0,
        PositionCenter    = 1,
        PositionNorthWest = 2,
        PositionNorth     = 3,
        PositionNorthEast = 4,
        PositionEast      = 5,
        PositionSouthEast = 6,
        PositionSouth     = 7,
        PositionSouthWest = 8,
        PositionWest      = 9,
        PositionFloating  =10
    };


    /**
      Measure calculation mode: the way how the absolute value of a KDChart::Measure is determined during KD Chart's internal geometry calculation time.

      KDChart::Measure values either are relative (calculated in relation to a given AbstractArea), or they are absolute (used as fixed values).

      Values stored in relative measure always are interpreted as per-mille of a reference area's height (or width, resp.) depending on the orientation set for the KDChart::Measure.

      \li \c MeasureCalculationModeAbsolute Value set by setValue() is absolute, to be used unchanged.
      \li \c MeasureCalculationModeRelative Value is relative, the reference area is specified by setReferenceArea(), and orientation specified by setOrientation().
      \li \c MeasureCalculationModeAuto Value is relative, KD Chart will automatically determine which reference area to use, and it will determine the orientation too.
      \li \c MeasureCalculationModeAutoArea Value is relative, Orientation is specified by setOrientation(), and KD Chart will automatically determine which reference area to use.
      \li \c MeasureCalculationModeAutoOrientation Value is relative, Area is specified by setReferenceArea(), and KD Chart will automatically determine which orientation to use.

      \sa KDChart::Measure::setCalculationMode
      */
    enum MeasureCalculationMode { MeasureCalculationModeAbsolute,
        MeasureCalculationModeRelative,
        MeasureCalculationModeAuto,
        MeasureCalculationModeAutoArea,
        MeasureCalculationModeAutoOrientation };

    /**
      Converts the specified measure calculation mode enum to a
      string representation.

      \param mode the measure calculation mode to convert
      \return the string representation of the Measure calculation mode enum
      */
    static QString measureCalculationModeToString( MeasureCalculationMode mode ) {
        switch( mode ) {
            case MeasureCalculationModeAbsolute:
                return QString::fromLatin1("MeasureCalculationModeAbsolute");
            case MeasureCalculationModeAuto:
                return QString::fromLatin1("MeasureCalculationModeAuto");
            case MeasureCalculationModeAutoArea:
                return QString::fromLatin1("MeasureCalculationModeAutoArea");
            case MeasureCalculationModeAutoOrientation:
                return QString::fromLatin1("MeasureCalculationModeAutoOrientation");
            case MeasureCalculationModeRelative:
                return QString::fromLatin1("MeasureCalculationModeRelative");
            default: // should not happen
        qDebug( "Unknown measure calculation mode" );
        return QString::fromLatin1("MeasureCalculationModeAuto");
        }
    }


    /**
      Converts the specified string to a measure calculation mode enum value.

      \param string the string to convert
      \return the measure calculation mode enum value
      */
    static MeasureCalculationMode stringToMeasureCalculationMode( const QString& string ) {
      if( string == QString::fromLatin1("MeasureCalculationModeAbsolute") )
          return MeasureCalculationModeAbsolute;
      if( string == QString::fromLatin1("MeasureCalculationModeAuto") )
          return MeasureCalculationModeAuto;
      if( string == QString::fromLatin1("MeasureCalculationModeAutoArea") )
          return MeasureCalculationModeAutoArea;
      if( string == QString::fromLatin1("MeasureCalculationModeAutoOrientation") )
          return MeasureCalculationModeAutoOrientation;
      if( string == QString::fromLatin1("MeasureCalculationModeRelative") )
          return MeasureCalculationModeRelative;
      // default, should not happen
      return MeasureCalculationModeAuto;
    }

    /**
      Measure orientation mode: the way how the absolute value of a KDChart::Measure is determined during KD Chart's internal geometry calculation time.

      KDChart::Measure values either are relative (calculated in relation to a given AbstractArea), or they are absolute (used as fixed values).

      Values stored in relative measure take into account the width (and/or the height, resp.) of a so-called reference area,
      that is either specified by KDChart::Measure::setReferenceArea, or determined by KD Chart automatically, respectively.

      \li \c MeasureOrientationAuto Value is calculated, based upon the width (or on the height, resp.) of the reference area: KD Chart will automatically determie an appropriate way.
      \li \c MeasureOrientationHorizontal Value is calculated, based upon the width of the reference area.
      \li \c MeasureOrientationVertical Value is calculated, based upon the height of the reference area.
      \li \c MeasureOrientationMinimum Value is calculated, based upon the width (or on the height, resp.) of the reference area - which ever is smaller.
      \li \c MeasureOrientationMaximum Value is calculated, based upon the width (or on the height, resp.) of the reference area - which ever is smaller.

      \sa KDChart::Measure::setOrientationMode
      */
    enum MeasureOrientation { MeasureOrientationAuto,
        MeasureOrientationHorizontal,
        MeasureOrientationVertical,
        MeasureOrientationMinimum,
        MeasureOrientationMaximum };

    /**
      Converts the specified measure orientation enum to a
      string representation.

      \param mode the measure orientation to convert
      \return the string representation of the measure orientation enum
      */
    static QString measureOrientationToString( MeasureOrientation mode ) {
        switch( mode ) {
            case MeasureOrientationAuto:
                return QString::fromLatin1("MeasureOrientationAuto");
            case MeasureOrientationHorizontal:
                return QString::fromLatin1("MeasureOrientationHorizontal");
            case MeasureOrientationVertical:
                return QString::fromLatin1("MeasureOrientationVertical");
            case MeasureOrientationMinimum:
                return QString::fromLatin1("MeasureOrientationMinimum");
            case MeasureOrientationMaximum:
                return QString::fromLatin1("MeasureOrientationMaximum");
            default: // should not happen
        qDebug( "Unknown measure orientation mode" );
        return QString::fromLatin1("MeasureOrientationAuto");
        }
    }


    /**
      Converts the specified string to a measure orientation enum value.

      \param string the string to convert
      \return the measure orientation enum value
      */
    static MeasureOrientation stringToMeasureOrientation( const QString& string ) {
      if( string == QString::fromLatin1("MeasureOrientationAuto") )
          return MeasureOrientationAuto;
      if( string == QString::fromLatin1("MeasureOrientationHorizontal") )
          return MeasureOrientationHorizontal;
      if( string == QString::fromLatin1("MeasureOrientationVertical") )
          return MeasureOrientationVertical;
      if( string == QString::fromLatin1("MeasureOrientationMinimum") )
          return MeasureOrientationMinimum;
      if( string == QString::fromLatin1("MeasureOrientationMaximum") )
          return MeasureOrientationMaximum;
      // default, should not happen
      return MeasureOrientationAuto;
    }


};


#endif
