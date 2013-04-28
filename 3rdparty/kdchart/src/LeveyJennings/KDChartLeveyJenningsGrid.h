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

#ifndef KDCHARTLEVEYJENNINGSGRID_H
#define KDCHARTLEVEYJENNINGSGRID_H

#include "KDChartCartesianGrid.h"

namespace KDChart {

    class PaintContext;

    /**
     * \internal
     *
     * \brief Class for the grid in a Levey Jennings plane.
     *
     * The LeveyJenningsGrid interface is used
     * for calculating and for drawing
     * the horizontal grid lines, and the vertical grid lines
     * of a Levey Jennings coordinate plane.
     */
    class LeveyJenningsGrid : public CartesianGrid
    {
    public:
        LeveyJenningsGrid() : CartesianGrid(){}
        virtual ~LeveyJenningsGrid(){}

        void drawGrid( PaintContext* context );

    private:
        DataDimensionsList calculateGrid( const DataDimensionsList& rawDataDimensions ) const;
        DataDimension calculateGridXY( const DataDimension& rawDataDimension, 
                          Qt::Orientation orientation, bool adjustLower, bool adjustUpper ) const;
        void calculateStepWidth( qreal start_, qreal end_, const QList<qreal>& granularities, Qt::Orientation orientation,
                                 qreal& stepWidth, qreal& subStepWidth, bool adjustLower, bool adjustUpper ) const;
    };

}

#endif
