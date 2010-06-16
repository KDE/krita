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

#ifndef KDCHARTABSTRACTTERNARYDIAGRAM_P_H
#define KDCHARTABSTRACTTERNARYDIAGRAM_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the KD Chart API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "KDChartAbstractTernaryDiagram.h"

#include "KDChartTernaryCoordinatePlane.h"
#include <KDChartAbstractDiagram_p.h>
#include <KDChartAbstractThreeDAttributes.h>
#include <KDChartGridAttributes.h>

#include "../Scenery/ReverseMapper.h"
#include "../Scenery/ChartGraphicsItem.h"
#include <KDABLibFakes>


namespace KDChart {

    class CartesianCoordinatePlane;
    class AbstractTernaryDiagram;

/**
 * \internal
 */
    class AbstractTernaryDiagram::Private : public AbstractDiagram::Private
    {
        friend class AbstractTernaryDiagram;
    public:
        Private();
        ~Private() {}

        Private( const Private& rhs ) :
            AbstractDiagram::Private( rhs ),
        // Do not copy axes and reference diagrams.
        axesList(),
        referenceDiagram( 0 )
        {
        }

        TernaryAxisList axesList;

        AbstractTernaryDiagram* referenceDiagram;
        QPointF referenceDiagramOffset;

        void drawPoint( QPainter* p, int row, int column,
                        const QPointF& widgetLocation )
        {
            // Q_ASSERT( false ); // unused, to be removed
            static const double Diameter = 5.0;
            static const double Radius = Diameter / 2.0;
            QRectF ellipseRect( widgetLocation - QPointF( Radius, Radius ),
                                QSizeF( Diameter, Diameter ) );
            p->drawEllipse( ellipseRect );

            reverseMapper.addRect( row, column, ellipseRect );
        }

        virtual void paint( PaintContext* paintContext )
        {
            paintContext->painter()->setRenderHint( QPainter::Antialiasing,
                                                    antiAliasing );
            if ( !axesList.isEmpty() ) {

                Q_FOREACH( TernaryAxis* axis, axesList ) {
                    PainterSaver s( paintContext->painter() );
                    axis->paintCtx( paintContext );
                }
            }
        }
    };

    KDCHART_IMPL_DERIVED_DIAGRAM( AbstractTernaryDiagram, AbstractDiagram, TernaryCoordinatePlane )
/*
inline AbstractTernaryDiagram::AbstractTernaryDiagram( Private * p )
    : AbstractDiagram( p ) { init(); }
inline AbstractTernaryDiagram::AbstractTernaryDiagram(
    Private * p, QWidget* parent, CartesianCoordinatePlane* plane )
    : AbstractDiagram( p, parent, plane ) { init(); }
inline AbstractTernaryDiagram::Private * AbstractTernaryDiagram::d_func()
{ return static_cast<Private*>( AbstractDiagram::d_func() ); }
inline const AbstractTernaryDiagram::Private * AbstractTernaryDiagram::d_func() const
{ return static_cast<const Private*>( AbstractDiagram::d_func() ); }
*/

}

#endif /* KDCHARTABSTRACTTERNARYDIAGRAM_P_H */

