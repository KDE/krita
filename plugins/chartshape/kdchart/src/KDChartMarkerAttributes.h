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

#ifndef KDCHARTMARKERATTRIBUTES_H
#define KDCHARTMARKERATTRIBUTES_H

#include <QMetaType>
#include "KDChartGlobal.h"

class QColor;
class QSizeF;
class QPen;
class QDebug;
template <typename T, typename K> class QMap;

namespace KDChart {

    /**
      * @brief A set of attributes controlling the appearance of data set markers
      */
    class KDCHART_EXPORT MarkerAttributes
    {
    public:
        MarkerAttributes();
        MarkerAttributes( const MarkerAttributes& );
        MarkerAttributes &operator= ( const MarkerAttributes& );

        ~MarkerAttributes();

        enum MarkerStyle { MarkerCircle  = 0,
                           MarkerSquare  = 1,
                           MarkerDiamond = 2,
                           Marker1Pixel  = 3,
                           Marker4Pixels = 4,
                           MarkerRing    = 5,
                           MarkerCross   = 6,
                           MarkerFastCross = 7,
                           NoMarker = 8,
                           // Above is kept for backwards binary compatibility.
                           // Below are added for compatibility with ODF
                           MarkerArrowDown     =  9,
                           MarkerArrowUp       = 10,
                           MarkerArrowRight    = 11,
                           MarkerArrowLeft     = 12,
                           MarkerBowTie        = 13,
                           MarkerHourGlass     = 14,
                           MarkerStar          = 15,
                           MarkerX             = 16,
                           MarkerAsterisk      = 17,
                           MarkerHorizontalBar = 18,
                           MarkerVerticalBar   = 19 };

        enum MarkerSizeMode {
            /// the marker size is directly specified in pixels
            AbsoluteSize = 0,
            /// the marker size is specified in pixels, but scaled by the
            /// painter's zoom level
            AbsoluteSizeScaled = 1,
            /// the marker size is relative to the diagram's width
            RelativeToDiagramWidth  = 2,
            /// the marker size is relative to the diagram's height
            RelativeToDiagramHeight = 3,
            /// the marker size is relative to the diagram's min(width, height)
            RelativeToDiagramWidthHeightMin = 4 };

        void setVisible( bool visible );
        bool isVisible() const;

        typedef QMap<uint, MarkerStyle> MarkerStylesMap;
        void setMarkerStylesMap( const MarkerStylesMap & map );
        MarkerStylesMap markerStylesMap() const;
        
        void setThreeD( bool value );
        bool threeD() const;

        void setMarkerStyle( MarkerStyle style );
        MarkerStyle markerStyle() const;

        /**
         * Normally you need to specify a valid QSizeF here, but for Legends you can
         * use the invalid size QSizeF(), to enable automatic marker size calculation:
         *
         * For Markers shown in a Legend this means the marker size will be equal to
         * the font height used for the labels that are shown next to the markers.
         */
        void setMarkerSize( const QSizeF& size );
        QSizeF markerSize() const;

        /**
         * With this method you can change the way the actual marker size is
         * calculated.
         *
         * By default, the marker size is absolute (equiv. to @a mode = AbsoluteSize)
         * and specifies the size in pixels.
         *
         * In any other case, the size specified will be relative to what is
         * specified in @a mode, e.g. the diagram's width. A marker width or
         * height of 1.0 is then 100% of the diagram's width.
         */
        void setMarkerSizeMode( MarkerSizeMode mode );
        MarkerSizeMode markerSizeMode() const;

        void setMarkerColor( const QColor& color );
        QColor markerColor() const;

        void setPen( const QPen& pen );
        QPen pen() const;

        bool operator==( const MarkerAttributes& ) const;
        bool operator!=( const MarkerAttributes& ) const;

    private:
        KDCHART_DECLARE_PRIVATE_BASE_VALUE( MarkerAttributes )
    }; // End of class MarkerAttributes

    inline bool MarkerAttributes::operator!=( const MarkerAttributes & other ) const { return !operator==( other ); }
}

KDCHART_DECLARE_SWAP_SPECIALISATION( KDChart::MarkerAttributes )
Q_DECLARE_TYPEINFO( KDChart::MarkerAttributes, Q_MOVABLE_TYPE );
Q_DECLARE_METATYPE( KDChart::MarkerAttributes )

#ifndef QT_NO_DEBUG_STREAM
KDCHART_EXPORT QDebug operator<<( QDebug, const KDChart::MarkerAttributes & );
#endif

#endif // KDCHARTMARKERATTRIBUTES_H
