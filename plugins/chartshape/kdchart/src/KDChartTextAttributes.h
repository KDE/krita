/* -*- Mode: C++ -*-
   KDChart - a multi-platform charting engine
   */

/****************************************************************************
 ** Copyright (C) 2005-2007 Klarävdalens Datakonsult AB.  All rights reserved.
 **
 ** This file is part of the KD Chart library.
 **
 ** This file may be used under the terms of the GNU General Public
 ** License versions 2.0 or 3.0 as published by the Free Software
 ** Foundation and appearing in the files LICENSE.GPL2 and LICENSE.GPL3
 ** included in the packaging of this file.  Alternatively you may (at
 ** your option) use any later version of the GNU General Public
 ** License if such license has been publicly approved by
 ** Klarälvdalens Datakonsult AB (or its successors, if any).
 ** 
 ** This file is provided "AS IS" with NO WARRANTY OF ANY KIND,
 ** INCLUDING THE WARRANTIES OF DESIGN, MERCHANTABILITY AND FITNESS FOR
 ** A PARTICULAR PURPOSE. Klarälvdalens Datakonsult AB reserves all rights
 ** not expressly granted herein.
 ** 
 ** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 ** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 **
 **********************************************************************/

#ifndef KDCHARTTEXTATTRIBUTES_H
#define KDCHARTTEXTATTRIBUTES_H

#include <QDebug>
#include <QMetaType>
#include "KDChartGlobal.h"
#include "KDChartMeasure.h"

class QPen;
class QFont;

namespace KDChart {

    /**
     * \brief A set of text attributes.
     *
     * TextAttributes encapsulates settings that have to do with
     * text. This includes font, fontsize, color, whether the text
     * is rotated, etc
     */
class KDCHART_EXPORT TextAttributes
{
public:
  TextAttributes();
  TextAttributes( const TextAttributes& );
  TextAttributes &operator= ( const TextAttributes& );
  bool operator==( const TextAttributes& ) const;
  inline bool operator!=( const TextAttributes& other ) const
  { return !operator==(other); }

  ~TextAttributes();

  /**
   * Set whether the text is to be rendered at all.
   * \param visible Whether the text is visible.
   */
  void setVisible( bool visible );

  /**
   * \return Whether the text is visible.
   */
  bool isVisible() const;

  /**
   * Set the font to be used for rendering the text.
   *
   * \note All of the font's attributes will be used - except of its size!
   * To specify the size please use setFontSize (or setMinimalFontSize, resp.)
   *
   * \param font The font to use.
   *
   * \sa setFontSize, setMinimalFontSize
   */
  void setFont( const QFont& font );

  /**
   * \return The font that is used for rendering text.
   */
  QFont font() const;

  /**
   * Set the size of the font used for rendering text.
   * \param measure The measure to use.
   * \see Measure
   */
  void setFontSize( const Measure & measure );

  /**
   * \return The measure used for the font size.
   */
  Measure fontSize() const;

  /**
   * Set the minimal size of the font used for rendering text.
   * \param measure The measure to use.
   * \see Measure
   */
  void setMinimalFontSize( const Measure & measure );

  /**
   * \return The measure used for the minimal font size.
   */
  Measure minimalFontSize() const;

  /**
   * \brief Returns the font size that is used at drawing time.
   *
   * This method is called at drawing time. It returns the
   * font size as it is used for rendering text, taking into account
   * any measures that were set via setFontSize and/or setMinimalFontSize.
   */
#if QT_VERSION < 0x040400 || defined(Q_COMPILER_MANGLES_RETURN_TYPE)
  const qreal calculatedFontSize(
#else
  qreal calculatedFontSize(
#endif
        const QObject*                   autoReferenceArea,
        KDChartEnums::MeasureOrientation autoReferenceOrientation ) const;

  /**
   * \brief Returns the font in the size that is used at drawing time.
   *
   * This method is called at drawing time. It returns the
   * font as it is used for rendering text, taking into account
   * any measures that were set via setFontSize and/or setMinimalFontSize.
   */
  const QFont calculatedFont(
        const QObject*                   autoReferenceArea,
        KDChartEnums::MeasureOrientation autoReferenceOrientation ) const;

  /**
   * \return Whether the text has an absolute font size set.
   */
  bool hasAbsoluteFontSize() const;

  /**
   * Set whether the text should be automatically rotated as
   * needed when space is constraint.
   * \param autoRotate Whether text should be automatically rotated.
   */
  void setAutoRotate( bool autoRotate );

  /**
   * \return Whether text is automatically rotated when space is
   * constrained.
   */
  bool autoRotate() const;

  /**
   * Set whether the text should automatically be shrunk, if
   * space is constraint.
   * \param autoShrink Whether text should be auto-shrunk.
   */
  void setAutoShrink( bool autoShrink );

  /**
   * \return Whether text is automatically shrunk if space is
   * constraint.
   */
  bool autoShrink() const;

  /**
   * Set the rotation angle to use for the text.
   *
   * \note For axis titles the rotation angle can be set to one of the
   * following angles: 0, 90, 180, 270
   * Any other values specified will be replaced by the next smaller
   * one of the allowed values, so no matter what you set the rotation
   * will always be one of these four values.
   *
   * \param rotation The rotation angle.
   */
  void setRotation( int rotation );

  /**
   * \return The rotation angle used for rendering the text.
   */
  int rotation() const;

  /**
   * Set the pen to use for rendering the text.
   * \param pen The pen to use.
   */
  void setPen( const QPen& pen );

  /**
   * \return The pen used for rendering the text.
   */
  QPen pen() const;

  // FIXME KDChartEnums::TextLayoutPolicy?

private:
  KDCHART_DECLARE_PRIVATE_BASE_VALUE( TextAttributes )

}; // End of class TextAttributes

}

#if !defined(QT_NO_DEBUG_STREAM)
KDCHART_EXPORT QDebug operator<<(QDebug, const KDChart::TextAttributes& );
#endif /* QT_NO_DEBUG_STREAM */

KDCHART_DECLARE_SWAP_SPECIALISATION( KDChart::TextAttributes )
Q_DECLARE_METATYPE( KDChart::TextAttributes )
Q_DECLARE_TYPEINFO( KDChart::TextAttributes, Q_MOVABLE_TYPE );

#endif // KDCHARTTEXTATTRIBUTES_H
