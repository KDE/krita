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

#ifndef KDCHARTDATAVALUEATTRIBUTES_H
#define KDCHARTDATAVALUEATTRIBUTES_H

#include <Qt>
#include <QMetaType>
#include "KDChartGlobal.h"
#include "KDChartEnums.h"
#include "KDChartRelativePosition.h"

/** \file KDChartDataValueAttributes.h
 *  \brief Declaring the class KDChart::DataValueAttributes.
 *
 *
 */


namespace KDChart {

  class TextAttributes;
  class BackgroundAttributes;
  class FrameAttributes;
  class MarkerAttributes;

  /**
   * \class DataValueAttributes KDChartDataValueAttributes.h KDChartDataValueAttributes
   * \brief Diagram attributes dealing with data value labels.
   *
   * The DataValueAttributes group all properties that can be set
   * wrt data value labels and if and how they are displayed. This
   * includes things like the text attributes (font, color), what
   * markers are used, howmany decimal digits are displayed, etc.
   */
class KDCHART_EXPORT DataValueAttributes
{
public:
  DataValueAttributes();
  DataValueAttributes( const DataValueAttributes& );
  DataValueAttributes &operator= ( const DataValueAttributes& );
  bool operator==( const DataValueAttributes& ) const;
  inline bool operator!=( const DataValueAttributes& other ) const { return !operator==(other); }

  ~DataValueAttributes();

  static const DataValueAttributes& defaultAttributes();
  static const QVariant& defaultAttributesAsVariant();

  /** Set whether data value labels should be displayed.
   * \param visible Whether data value labels should be displayed.
   */
  void setVisible( bool visible );

  /**
   * @return Whether data value labels should be displayed.
   */
  bool isVisible() const;

  /**
   * Set the text attributes to use for the data value labels.
   * \param a The text attributes to set.
   * \see TextAttributes
   */
  void setTextAttributes( const TextAttributes &a );

  /**
   * \return The text attributes used for painting data value labels.
   */
  TextAttributes textAttributes() const;

  /**
   * Set the frame attributes to use for the data value labels area.
   * \param a The frame attributes to set.
   * \see FrameAttributes
   */
  void setFrameAttributes( const FrameAttributes &a );

  /**
   * \return The frame attributes used for painting the data
   * value labels area.
   * \see FrameAttributes
   */
  FrameAttributes frameAttributes() const;

  /**
   * Set the background attributes to use for the data value labels area.
   * \param a The background attributes to set.
   * \see BackgroundAttributes
   */
  void setBackgroundAttributes( const BackgroundAttributes &a );

  /**
   * \return The background attributes used for painting the data
   * value labels area.
   * \see BackgroundAttributes
   */
  BackgroundAttributes backgroundAttributes() const;

  /**
   * Set the marker attributes to use for the data values. This includes
   * the marker type.
   * \param a The marker attributes to set.
   * \see MarkerAttributes
   */
  void setMarkerAttributes( const MarkerAttributes &a );

  /**
   * \return The marker attributes used for decorating the data
   * values.
   * \see MarkerAttributes
   */
  MarkerAttributes markerAttributes() const;

  /**
   * Specify whether to use percentages instead of actual data point values when no
   * specific label is set. In a bar or cartesian diagram, this means that the value
   * will be shown in % in relation to the sum of all values in the same category, in
   * a polar diagram in relation to the sum of all values in a data set.
   *
   * When this is turned on, the value will \b not automatically have the '%' postfix.
   * \param enable Whether to enable percentage values
   */
  void setUsePercentage( bool enable );

  /**
   * \return Whether to use percentage values
   * \see setUsePercentage
   */
  bool usePercentage() const;

  /**
   * Set how  many decimal digits to display when rendering the data value
   * labels. If there are no decimal digits it will not be displayed.
   * \param digits The number of decimal digits to use.
   */
  void setDecimalDigits( int digits );

  /**
   * \return The number of decimal digits displayed.
   */
  int decimalDigits() const;

  /**
   * \brief Prepend a prefix string to the data value label
   * \sa prefix
   */
  void setPrefix( const QString &prefix );

 /**
   * \brief Returns the string used as a prefix to the data value text.
   * \sa setPrefix
   */
  QString prefix() const;

  /**
   * \brief Append a suffix string to the data value label
   * \sa suffix
   */
  void setSuffix( const QString &suffix );

 /**
   * \brief Returns the string used as a suffix to the data value text.
   * \sa setSuffix
   */
  QString suffix() const;

 /**
   * \brief display a string label instead of the original data value label
   * Supports HTML code.
   * \sa dataLabel
   */
  void setDataLabel( const QString &label );

 /**
   * \brief Returns the string displayed instead of the data value label
   * \sa setDataLabel
   */
  QString dataLabel() const;

 /**
   * \return Whether data values not different from their predecessors are drawn.
  */
  bool showRepetitiveDataLabels() const;

 /**
   *
   * Set whether data value labels not different from their predecessors should be drawn.
   * \param showRepetitiveDataLabels Whether data value not different from their predecessors are drawn.
  */
  void setShowRepetitiveDataLabels( bool showRepetitiveDataLabels );

 /**
   * \return Whether data value texts overlapping other data value texts of the same diagram are drawn.
  */
  bool showOverlappingDataLabels() const;

 /**
   *
   * Set whether data value texts overlapping other data value texts of the same diagram should be drawn.
   * \param showOverlappingDataLabels Whether data texts overlapping other data value texts of the same diagram are drawn.
  */
  void setShowOverlappingDataLabels( bool showOverlappingDataLabels );

  /**
    * \cond PLANNED_FOR_FUTURE
    *
    * These method are planned for future versions of KD Chart,
    * so they are not part of the documented API yet.
    *
    */
  void setPowerOfTenDivisor( int powerOfTenDivisor );
  int powerOfTenDivisor() const;
   /**
   * \endcond
   */


  /**
   * \cond PLANNED_FOR_FUTURE
   *
   * These method are planned for future versions of KD Chart,
   * so they are not part of the documented API yet.
   */
  void setShowInfinite( bool infinite );
  bool showInfinite() const;
  /**
   * \endcond
   */

   /**
   * \brief Defines the relative positioning of the data value labels for negative values.
    *
    * The position is specified in relation to the respective data value point, or in
    * releation to the respective data representation area, that's one area segment in
    * a LineDiagram showing areas, or one bar in a BarDiagram, one pie slice ...
    *
   * \sa negativePosition
   */
  void setNegativePosition( const RelativePosition& relPosition );

   /**
   * \brief Return the relative positioning of the data value labels
   * \sa setNegativePosition
   */
  const RelativePosition negativePosition() const;

  /**
   * \brief Defines the relative position of the data value labels for positive values.
   *
   * The position is specified in relation to the respective data value point, or in
   * releation to the respective data representation area, that's one area segment in
   * a LineDiagram showing areas, or one bar in a BarDiagram, one pie slice ...
   *
   * \sa positivePosition
   */
  void setPositivePosition( const RelativePosition& relPosition );

   /**
   * \brief Return the relative positioning of the data value labels
   * \sa setPositivePosition
   */
  const RelativePosition positivePosition() const;

  const RelativePosition position( bool positive ) const
  {
    return positive ? positivePosition() : negativePosition();
  }

private:
  KDCHART_DECLARE_PRIVATE_BASE_VALUE( DataValueAttributes )

}; // End of class DataValueAttributes

}

#if !defined(QT_NO_DEBUG_STREAM)
KDCHART_EXPORT QDebug operator<<(QDebug, const KDChart::DataValueAttributes& );
#endif /* QT_NO_DEBUG_STREAM */

Q_DECLARE_METATYPE( KDChart::DataValueAttributes )
Q_DECLARE_TYPEINFO( KDChart::DataValueAttributes, Q_MOVABLE_TYPE );
KDCHART_DECLARE_SWAP_SPECIALISATION( KDChart::DataValueAttributes )

#endif // KDCHARTDATAVALUEATTRIBUTES_H
