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

#ifndef KDCHART_RADAR_COORDINATEPLANE_H
#define KDCHART_RADAR_COORDINATEPLANE_H

#include "KDChartPolarCoordinatePlane.h"

namespace KDChart {

    class Chart;
    
    /**
      * @brief Radar coordinate plane
      */
    class KDCHART_EXPORT RadarCoordinatePlane : public PolarCoordinatePlane
    {
        Q_OBJECT

        Q_DISABLE_COPY( RadarCoordinatePlane )
        KDCHART_DECLARE_PRIVATE_DERIVED_PARENT( RadarCoordinatePlane, Chart* )

    public:

        explicit RadarCoordinatePlane ( Chart* parent = 0 );
        ~RadarCoordinatePlane();
        
                
        /**
         * Set the attributes to be used for axis captions.
         *
         * To disable axis captions, for example, your code should like this:
         * \code
         * TextAttributes ta = plane->textAttributes();
         * ta.setVisible( false );
         * plane-setTextAttributes( ta );
         * \endcode
         */
        void setTextAttributes( const TextAttributes & attr );
 
        /**
         * \return The attributes used for axis.
         *
         * \note This function always returns a valid set of text attributes:
         * If no special text attributes was set a default one is 
         * returned.
         *
         * \sa setTextAttributes
         */
        const TextAttributes textAttributes() const;

    };
    
}

#endif
