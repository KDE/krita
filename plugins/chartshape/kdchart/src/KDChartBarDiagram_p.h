/****************************************************************************
 ** Copyright (C) 2007 Klaralvdalens Datakonsult AB.  All rights reserved.
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

#ifndef KDCHARTBARDIAGRAM_P_H
#define KDCHARTBARDIAGRAM_P_H

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

#include "KDChartBarDiagram.h"

#include <QPainterPath>

#include "KDChartAbstractCartesianDiagram_p.h"
#include "KDChartThreeDBarAttributes.h"

#include <KDABLibFakes>


namespace KDChart {

class PaintContext;

/**
 * \internal
 */
class BarDiagram::Private : public AbstractCartesianDiagram::Private
{
    friend class BarDiagram;
    friend class BarDiagramType;

public:
    Private();
    Private( const Private& rhs );
    ~Private();

/* refactoring */
/*
    Private( const Private& rhs ) :
        AbstractCartesianDiagram::Private( rhs ),
        barType( rhs.barType ),
        maxDepth( rhs.maxDepth )
        {
        }

    void calculateValueAndGapWidths( int rowCount, int colCount,
                                     double groupWidth,
                                     double& barWidth,
                                     double& spaceBetweenBars,
                                     double& spaceBetweenGroups );
*/

    Qt::Orientation orientation;

    BarDiagram* diagram;
    BarDiagramType* implementor; // the current type
    BarDiagramType* normalDiagram;
    BarDiagramType* stackedDiagram;
    BarDiagramType* percentDiagram;
    BarDiagramType* normalLyingDiagram;
    BarDiagramType* stackedLyingDiagram;
    BarDiagramType* percentLyingDiagram;

/* refactoring */
/*
    BarType barType;
private:
    double maxDepth;
*/
};

KDCHART_IMPL_DERIVED_DIAGRAM( BarDiagram, AbstractCartesianDiagram, CartesianCoordinatePlane )

   // we inherit privately, so that derived classes cannot call the
    // base class functions - those reference the wrong (unattached to
    // a diagram) d
    class BarDiagram::BarDiagramType : private BarDiagram::Private
    {
    public:
        explicit BarDiagramType( BarDiagram* d )
            : BarDiagram::Private()
            , m_private( d->d_func() )
        {
        }
        virtual ~BarDiagramType() {}
        virtual BarDiagram::BarType type() const = 0;
        virtual const QPair<QPointF,  QPointF> calculateDataBoundaries() const = 0;
        virtual void paint(  PaintContext* ctx ) = 0;
        BarDiagram* diagram() const;

    protected:
        // method that make elements of m_private available to derived
        // classes:
        AttributesModel* attributesModel() const;
        QModelIndex attributesModelRootIndex() const;
        ReverseMapper& reverseMapper();
        CartesianDiagramDataCompressor& compressor() const;

        void appendDataValueTextInfoToList(
            AbstractDiagram * diagram,
            DataValueTextInfoList & list,
            const QModelIndex & index,
            const PositionPoints& points,
            const Position& autoPositionPositive,
            const Position& autoPositionNegative,
            const qreal value );
        void paintDataValueTextsAndMarkers(
            AbstractDiagram* diag,
            PaintContext* ctx,
            const DataValueTextInfoList & list,
            bool paintMarkers );

        void paintBars( PaintContext* ctx, const QModelIndex& index,
            const QRectF& bar, double& maxDepth );
        void calculateValueAndGapWidths( int rowCount, int colCount,
            double groupWidth,
            double& barWidth,
            double& spaceBetweenBars,
            double& spaceBetweenGroups );

        BarDiagram::Private* m_private;
    };
}

#endif /* KDCHARTBARDIAGRAM_P_H */
