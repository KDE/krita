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

#ifndef KDCHARTBARDIAGRAM_H
#define KDCHARTBARDIAGRAM_H

#include "KDChartAbstractCartesianDiagram.h"
#include "KDChartBarAttributes.h"
//#include "KDChartThreeDBarAttributes.h"

class QPainter;

namespace KDChart {

    class ThreeDBarAttributes;

/**
 * @brief BarDiagram defines a common bar diagram.
 *
 * It provides different subtypes which are set using \a setType.
 */
class KDCHART_EXPORT BarDiagram : public AbstractCartesianDiagram
{
    Q_OBJECT

    Q_DISABLE_COPY( BarDiagram )

    KDCHART_DECLARE_DERIVED_DIAGRAM( BarDiagram, CartesianCoordinatePlane )

public:
    class BarDiagramType;
    friend class BarDiagramType;

    explicit BarDiagram(
        QWidget* parent = 0, CartesianCoordinatePlane* plane = 0 );
    virtual ~BarDiagram();

    virtual BarDiagram * clone() const;
    /**
    * Returns true if both diagrams have the same settings.
    */
    bool compare( const BarDiagram* other )const;

    enum BarType { Normal,
                   Stacked,
                   Percent,
                   Rows ///< @deprecated Use BarDiagram::setOrientation() instead
                 };

    void setType( const BarType type );
    BarType type() const;
    
    void setOrientation( Qt::Orientation orientation );
    Qt::Orientation orientation() const;

    void setBarAttributes( const BarAttributes & a );
    void setBarAttributes( int column, const BarAttributes & a );
    void setBarAttributes( const QModelIndex & index, const BarAttributes & a );

    BarAttributes barAttributes() const;
    BarAttributes barAttributes( int column ) const;
    BarAttributes barAttributes( const QModelIndex & index ) const;

    void setThreeDBarAttributes( const ThreeDBarAttributes & a );
    void setThreeDBarAttributes( int column, const ThreeDBarAttributes & a );
    void setThreeDBarAttributes( const QModelIndex & index,
                                  const ThreeDBarAttributes & a );
    ThreeDBarAttributes threeDBarAttributes() const;
    ThreeDBarAttributes threeDBarAttributes( int column ) const;
    ThreeDBarAttributes threeDBarAttributes( const QModelIndex & index ) const;

#if QT_VERSION < 0x040400 || defined(Q_COMPILER_MANGLES_RETURN_TYPE)
    // implement AbstractCartesianDiagram
    /** \reimpl */
    const int numberOfAbscissaSegments () const;
    /** \reimpl */
    const int numberOfOrdinateSegments () const;
#else
    // implement AbstractCartesianDiagram
    /** \reimpl */
    int numberOfAbscissaSegments () const;
    /** \reimpl */
    int numberOfOrdinateSegments () const;
#endif

protected:
    void paint ( PaintContext* paintContext );

public:
    void resize ( const QSizeF& area );

#if 0
    // FIXME merge with 3DAttributes?
    void setThreeDimensionalBars( bool threeDBars );
    bool threeDimensionalBars() const;

    void setThreeDimensionalBarsShadowColors( bool shadow );
    bool threeDimensionalBarsShadowColors() const;

    void setThreeDimensionalBarAngle( uint angle );
    uint threeDimensionalBarAngle() const;

    void setThreeDimensionalBarDepth( double depth );
    double threeDimensionalBarDepth() const;

#endif


protected:
    virtual double threeDItemDepth( const QModelIndex & index ) const;
    virtual double threeDItemDepth( int column ) const;
    /** \reimpl */
    const QPair<QPointF, QPointF> calculateDataBoundaries() const;
    void paintEvent ( QPaintEvent* );
    void resizeEvent ( QResizeEvent* );
private:

    /*
    void paintBars( PaintContext* ctx, const QModelIndex& index, const QRectF& bar, double& maxDepth );
    */
    void calculateValueAndGapWidths( int rowCount, int colCount,
                                     double groupWidth,
                                     double& barWidth,
                                     double& spaceBetweenBars,
                                     double& spaceBetweenGroups );
}; // End of class BarDiagram

}

#endif // KDCHARTBARDIAGRAM_H
