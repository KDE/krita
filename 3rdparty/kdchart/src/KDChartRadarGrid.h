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

#ifndef KDCHARTRADARGrid_H
#define KDCHARTRADARGrid_H

//#include "KDChartRadarCoordinatePlane.h"
#include "KDChartAbstractGrid.h"

namespace KDChart {

    class PaintContext;
    class RadarCoordinatePlane;

    /**
     * \internal
     *
     * \brief Class for the grid in a radar plane.
     *
     * The RadarGrid interface is used
     * for calculating and for drawing
     * the grid lines of a radar charts including "axis" 
     * labels.
     */
    class RadarGrid : public AbstractGrid
    {
    public:
        RadarGrid() : AbstractGrid(){}
        virtual ~RadarGrid(){}

        virtual void drawGrid( PaintContext* context );

    private:
        virtual DataDimensionsList calculateGrid(
            const DataDimensionsList& rawDataDimensions ) const;
    };

}

#endif