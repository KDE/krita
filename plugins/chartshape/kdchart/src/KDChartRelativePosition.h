/* -*- Mode: C++ -*-
  KDChart - a multi-platform charting engine
  */

/****************************************************************************
** Copyright (C) 2005-2007 Klarälvdalens Datakonsult AB.  All rights reserved.
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

#ifndef KDCHARTRELATIVEPOSITION_H
#define KDCHARTRELATIVEPOSITION_H

#include <QDebug>
#include <QMetaType>
#include <Qt>
#include <QPointF>
#include <QSizeF>
#include "KDChartGlobal.h"

namespace KDChart {

    class Position;
    class PositionPoints;
    class Measure;

/**
  \class RelativePosition KDChartRelativePosition.h
  \brief Defines relative position information: reference area, position
  in this area, horizontal / vertical padding, and rotating.

  Using RelativePosition you can specify the relative parts
  of some position information, and you can specify the absolute parts:
  the reference area, and the position in this area.

  \Note To get an absolute position, you have three options:
  \li either you declare both, the relative and the absolute parts,
  using setReferenceArea for the later,
  \li or you specify a set of points, using setReferencePoints,
  \li or you refrein from using either, but leave it to KD Chart to find
  a matching reference area for you.
  */
class KDCHART_EXPORT RelativePosition
{
public:
    RelativePosition();
    RelativePosition( const RelativePosition& );

    RelativePosition & operator=( const RelativePosition & other );

    ~RelativePosition();

    /**
     * \brief Specifies the reference area to be used to find the anchor point.
     *
     * The reference area's type can be either QWidget, or be derived from KDChart::AbstractArea.
     *
     * \note Usage of reference area and reference points works mutually exclusively:
     * Only one setting can be valid, so any former specification of reference points is reset
     * when you call setReferenceArea.
     *
     * Also note: In a few cases KD Chart will ignore your area (or points, resp.) settings!
     * Relative positioning of data value texts is an example: For these
     * the reference area is the respective data area taking precendence over your settings.
     *
     * \sa setReferencePosition, setAlignment, setHorizontalPadding, setVerticalPadding
     */
    void setReferenceArea( QObject* area );
    QObject* referenceArea() const;

    /**
     * \brief Specifies a set of points from which the anchor point will be selected.
     *
     * \note Usage of reference area and reference points works mutually exclusively:
     * Only one setting can be valid, so any former specification of reference area is reset
     * when you call setReferencePoints.
     *
     * Also note: In a few cases KD Chart will ignore your points (or area, resp.) settings!
     * Relative positioning of data value texts is an example: For these
     * the reference area is the respective data area taking precendence over your settings.
     *
     * \sa setReferenceArea, setReferencePosition, setAlignment, setHorizontalPadding, setVerticalPadding
     */
    void setReferencePoints( const PositionPoints& points );
    const PositionPoints referencePoints() const;

    /**
     * \brief Specifies the position of the anchor point.
     *
     * The anchor point of a RelativePosition may be one of the pre-defined
     * points of it's reference area - for details see KDChart::Position.
     *
     *  \sa resetReferencePosition, setReferenceArea, setAlignment, setHorizontalPadding, setVerticalPadding, KDChart::Position
     */
    void setReferencePosition( Position position );

    /**
     * \brief Resets the position of the anchor point to the built-in default.
     *
     * If the anchor point of a RelativePosition is reset (or never changed from the
     * default setting, resp.) KD Chart will choose an appropriate Position at run-time.
     *
     * e.g. BarDiagrams will use Position::NorthWest / Position::SouthEast for positive / negative values.
     *
     *  \sa setReferencePosition, setReferenceArea, setAlignment, setHorizontalPadding, setVerticalPadding, KDChart::Position
     */
    void resetReferencePosition();
    Position referencePosition() const;

    /**
     * Specifies the location of the content, that is to be positioned by this RelativePosition.
     *
     * Aligning is applied, after horiz./vert. padding was retrieved to calculate the real
     * reference point, so aligning is seen as relative to that point.
     *
     * \note When printing data value texts at a centered point you might want to call
     * setAlignment( Qt::AlignCenter ) and also set the horizontal/vertical padding to Zero
     * to have your texts centered more precisely.
     * 
     * \sa setReferencePosition, setReferenceArea, setHorizontalPadding, setVerticalPadding
     */
    void setAlignment( Qt::Alignment flags );
    Qt::Alignment alignment() const;

    /**
     * Specifies the horizontal width of the gap between the anchor point and the content,
     * that is to be positioned by this RelativePosition.
     *
     * \note When printing data value texts this Measure is used to find the alignment
     * point to align the text to, then alignment() is looked at to determine the way how
     * the text is to be aligned to that point. The font height is used as reference size
     * for both, horizontal and vertical padding, if the respective padding's Measure is
     * using automatic reference area detection.
     *
     * \sa setVerticalPadding, setReferencePosition, setReferenceArea
     */
    void setHorizontalPadding( const Measure& padding );
    Measure horizontalPadding() const;

    /**
     * Specifies the vertical width of the gap between the anchor point and the content,
     * that is to be positioned by this RelativePosition.
     *
     * \note When printing data value texts this Measure is used to find the alignment
     * point to align the text to, then alignment() is looked at to determine the way how
     * the text is to be aligned to that point. The font height is used as reference size
     * for both, horizontal and vertical padding, if the respective padding's Measure is
     * using automatic reference area detection.
     *
     * \sa setHorizontalPadding, setReferencePosition, setReferenceArea
     */
    void setVerticalPadding( const Measure& padding );
    Measure verticalPadding() const;

    void setRotation( qreal rot );
    qreal rotation() const;

    /**
     * \brief Return the reference point, according to the reference area/position, but ignoring horiz/vert padding.
     *
     * This method is called at drawing time.
     * The returned point is used to test if the label of a data value is to be printed: labels
     * are printed only, if their reference points are either inside or touching the coordinate plane.
     *
     * If polarDegrees is set, the degree information will be returned that was stored for the
     * respective point.  This is used by the PieDiagram class to determin how vertical/horizontal
     * padding settings should affect the position of the data value texts' reference points.
     *
     * \sa calculatedPoint, setReferenceArea, setReferencePosition, setHorizontalPadding, setVerticalPadding
     */
    const QPointF referencePoint(qreal* polarDegrees=0) const;

    /**
     * \brief Calculate a point, according to the reference area/position and horiz/vert padding.
     *
     * This method is called at drawing time: The returned point is used as anchor point.
     * Note that calculatedPoint ignores the alignment setting, it just returns the point,
     * so the calling code needs to take alignment into account explicitly.
     *
     * \sa referencePoint, setReferenceArea, setReferencePosition, setHorizontalPadding, setVerticalPadding
     */
    const QPointF calculatedPoint( const QSizeF& autoSize ) const;

    bool operator==( const RelativePosition& ) const;
    bool operator!=( const RelativePosition & other ) const;

private:
    KDCHART_DECLARE_PRIVATE_BASE_VALUE( RelativePosition )
};

inline bool RelativePosition::operator!=( const RelativePosition & other ) const { return !operator==( other ); }
}

KDCHART_DECLARE_SWAP_SPECIALISATION( KDChart::RelativePosition )

Q_DECLARE_TYPEINFO( KDChart::RelativePosition, Q_MOVABLE_TYPE );
Q_DECLARE_METATYPE( KDChart::RelativePosition )

#if !defined(QT_NO_DEBUG_STREAM)
KDCHART_EXPORT QDebug operator<<(QDebug, const KDChart::RelativePosition& );
#endif /* QT_NO_DEBUG_STREAM */


#endif // KDCHARTRELATIVEPOSITION_H
