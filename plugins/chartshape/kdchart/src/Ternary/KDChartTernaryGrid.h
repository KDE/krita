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

#ifndef KDCHARTTERNARYGRID_H
#define KDCHARTTERNARYGRID_H

#include <QList>

#include "KDChartAbstractGrid.h"
#include "PrerenderedElements/KDChartTextLabelCache.h"

namespace KDChart {

    struct TickInfo {
        TickInfo( double percentage = 0, int depth = 0 );
        double percentage;
        int depth;
    };

    bool operator==(const TickInfo&, const TickInfo& );

    class PaintContext;

    // VERIFY: Grids are not public API, are they?
    class TernaryGrid : public AbstractGrid
    {
    public:
        TernaryGrid();

        virtual ~TernaryGrid();

        void drawGrid( PaintContext* context );
        DataDimensionsList calculateGrid( const DataDimensionsList& rawDataDimensions ) const;

        /** Returns two QSizeF objects specifying the dimension of the
            margins needed between each corner of the diagram and the
            border of the drawing area. Margins are required because
            the tick marks are placed outside of the trianges
            containing rectangle.
            The margins are returned in <em>diagram coordinates</em>,
            since the grid does not know about widget coordinates.
        */
        QPair<QSizeF, QSizeF> requiredMargins() const;
        /** Return the locations of the grid lines, so that axes can
            draw axis rulers at the correct positions.
            This information is valid after the grid has been
            painted (that is, the axes need to be painted after the
            grid. */
        const QVector<TickInfo>& tickInfo() const;
    private:
        QVector<TickInfo> m_tickInfo;
        // QList<PrerenderedLabel> m_labels;
    };

}

#endif
