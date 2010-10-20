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

#ifndef KDCHARTRULERATTRIBUTES_H
#define KDCHARTRULERATTRIBUTES_H

#include <QMetaType>
#include "KDChartGlobal.h"
#include "KDChartEnums.h"

class QPen;

namespace KDChart {

/**
  * @brief A set of attributes controlling the appearance of axis rulers
  *
  * As the naming of this class suggests, it can not be used to modify the
  * grid of an axis, see e.g. @a CartesianCoordinatePlane::gridAttributes()
  * instead.
  */
class KDCHART_EXPORT RulerAttributes
{
public:
	RulerAttributes();
	RulerAttributes( const RulerAttributes& );
	RulerAttributes &operator= ( const RulerAttributes& );

    ~RulerAttributes();

    /**
      * Sets the pen used to draw all axis elements. This can be overwritten
      * for individual elements using e.g. @a setTickMarkPen().
      *
      * The default is a cosmetic black pen.
      */
    void setPen( const QPen& pen );
    QPen pen() const;

    /**
      * Use this to specify if the axis line (the one perpendicular to the
      * tick marks, reaching from one end of the axis to the other) is to be
      * drawn or not.
      *
      * By default, the ruler line is not drawn.
      *
      * This line is drawn exactly where the first or last grid line is drawn,
      * for the grid that has the same line orientation as the axis that these
      * attributes belong to.
      *
      * Thus, by default you will see this grid line instead if it is drawn.
      */
    void setShowRulerLine( bool show );
    bool showRulerLine() const;
    
    /**
      * Sets the pen used to draw the tick marks
      */
    void setTickMarkPen( const QPen& pen );
    QPen tickMarkPen() const;

    /**
      * Sets the pen used to draw major tick marks
      */
    void setMajorTickMarkPen( const QPen& pen );
    bool majorTickMarkPenIsSet() const;
    QPen majorTickMarkPen() const;

    /**
      * Sets the pen used to draw minor tick marks
      */
    void setMinorTickMarkPen( const QPen& pen );
    bool minorTickMarkPenIsSet() const;
    QPen minorTickMarkPen() const;

    /**
      * Sets the pen used to draw the tick mark at a specific value
      * 
      * Note: This will not paint a tick mark at the specified value
      * if it wasn't already drawn before. 
      */
    void setTickMarkPen( qreal value, const QPen& pen );
    QPen tickMarkPen( qreal value ) const;
    typedef QMap<qreal, QPen> TickMarkerPensMap;
    TickMarkerPensMap tickMarkPens() const;
    
    bool hasTickMarkPenAt( qreal value) const;

    /**
      * Color setter method provided for convenience
      */
    void setTickMarkColor( const QColor& color );
    QColor tickMarkColor() const;

    /**
      * Shows or hides minor tick marks
      */
    void setShowMinorTickMarks( bool show );
    bool showMinorTickMarks() const;

    /**
      * Shows or hides major tick marks
      */
    void setShowMajorTickMarks( bool show );
    bool showMajorTickMarks() const;
    
    /**
     * Set margin that should be used between the labals and the ticks. Per
     * default the value is -1 what means, that half of the label's font
     * height/width should be used as margin.
     */
    void setLabelMargin(int margin);
    int labelMargin() const;

    bool operator==( const RulerAttributes& ) const;
    inline bool operator!=( const RulerAttributes& other ) const { return !operator==(other); }

private:
    KDCHART_DECLARE_PRIVATE_BASE_VALUE( RulerAttributes )
}; // End of class RulerAttributes

}

#if !defined(QT_NO_DEBUG_STREAM)
KDCHART_EXPORT QDebug operator<<(QDebug, const KDChart::RulerAttributes& );
#endif /* QT_NO_DEBUG_STREAM */

KDCHART_DECLARE_SWAP_SPECIALISATION( KDChart::RulerAttributes )
Q_DECLARE_METATYPE( KDChart::RulerAttributes )
Q_DECLARE_TYPEINFO( KDChart::RulerAttributes, Q_MOVABLE_TYPE );


#endif // KDCHARTRULERATTRIBUTES_H
