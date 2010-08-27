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
