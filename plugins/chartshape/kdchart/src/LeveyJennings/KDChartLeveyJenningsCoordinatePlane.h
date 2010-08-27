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

#ifndef KDCHARTLEVEYJENNINGSCOORDINATEPLANE_H
#define KDCHARTLEVEYJENNINGSCOORDINATEPLANE_H

#include "../KDChartCartesianCoordinatePlane.h"

#include "KDChartLeveyJenningsGridAttributes.h"

namespace KDChart {

    class LeveyJenningsGrid;

    /**
      * @brief Levey Jennings coordinate plane
      * This is actually nothing real more than a plain cartesian
      * coordinate plane. The difference is, that only Levey Jennings
      * Diagrams can be added to it.
      */
    class KDCHART_EXPORT LeveyJenningsCoordinatePlane
        : public CartesianCoordinatePlane
    {
        Q_OBJECT
        Q_DISABLE_COPY( LeveyJenningsCoordinatePlane )
        KDCHART_DECLARE_PRIVATE_DERIVED_PARENT( LeveyJenningsCoordinatePlane, Chart* )

        friend class LeveyJenningsGrid;

    public:
        explicit LeveyJenningsCoordinatePlane( Chart* parent = 0 );
        ~LeveyJenningsCoordinatePlane();

        void addDiagram( AbstractDiagram* diagram );

        LeveyJenningsGridAttributes gridAttributes() const;
        void setGridAttributes( const LeveyJenningsGridAttributes& attr );
   
    protected: 
        const QPointF translateBack( const QPointF& screenPoint ) const;

    private:
        LeveyJenningsGrid* grid() const;
    };

}

#endif
