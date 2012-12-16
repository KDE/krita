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

#ifndef KDCHARTTERNARYCOORDINATEPLANE_H
#define KDCHARTTERNARYCOORDINATEPLANE_H

#include "../KDChartAbstractCoordinatePlane.h"

namespace KDChart {

    class TernaryGrid;

    /**
      * @brief Ternary coordinate plane
      */
    class KDCHART_EXPORT TernaryCoordinatePlane
        : public AbstractCoordinatePlane
    {
        Q_OBJECT
        Q_DISABLE_COPY( TernaryCoordinatePlane )
        KDCHART_DECLARE_PRIVATE_DERIVED_PARENT( TernaryCoordinatePlane, Chart* )

    public:
        explicit TernaryCoordinatePlane( Chart* parent = 0 );
        ~TernaryCoordinatePlane();

        void addDiagram( AbstractDiagram* diagram );

        void layoutDiagrams();

        const QPointF translate ( const QPointF& diagramPoint ) const;

        void paint( QPainter* );
        DataDimensionsList getDataDimensionsList() const;

        /** \reimpl */
        QSize minimumSizeHint() const;
        /** \reimpl */
        QSizePolicy sizePolicy() const;

    private:
        TernaryGrid* grid() const;
    };

}

#endif
