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

#ifndef KDCHARTLINEDIAGRAM_H
#define KDCHARTLINEDIAGRAM_H

#include "KDChartAbstractCartesianDiagram.h"
#include "KDChartLineAttributes.h"
#include "KDChartValueTrackerAttributes.h"

class QPainter;
class QPolygonF;

namespace KDChart {

    class ThreeDLineAttributes;

/**
 * @brief LineDiagram defines a common line diagram.
 *
 * It provides different subtypes which are set using \a setType.
 */
class KDCHART_EXPORT LineDiagram : public AbstractCartesianDiagram
{
    Q_OBJECT

    Q_DISABLE_COPY( LineDiagram )
//    KDCHART_DECLARE_PRIVATE_DERIVED_PARENT( LineDiagram, CartesianCoordinatePlane * )

    KDCHART_DECLARE_DERIVED_DIAGRAM( LineDiagram, CartesianCoordinatePlane )


public:
    class LineDiagramType;
    friend class LineDiagramType;

    explicit LineDiagram( QWidget* parent = 0, CartesianCoordinatePlane* plane = 0 );
    virtual ~LineDiagram();

    virtual LineDiagram * clone() const;

    /**
     * Returns true if both diagrams have the same settings.
     */
    bool compare( const LineDiagram* other ) const;

    enum LineType {
        Normal =  0,
        Stacked = 1,
        Percent = 2
    };


    void setType( const LineType type );
    LineType type() const;

    /** If centerDataPoints() is true, all data points are moved by an
     * offset of 0.5 to the right. This is useful in conjunction with
     * bar diagrams, since data points are then centered just like bars.
     *
     * \sa centerDataPoints()
     */
    void setCenterDataPoints( bool center );
    /** @return option set by setCenterDataPoints() */
    bool centerDataPoints() const;

    /** With this property set to true, data sets in a normal line diagram
     * are drawn in reversed order. More clearly, the first (top-most) data set
     * in the source model will then appear in front. This is mostly due to
     * historical reasons.
     */
    void setReverseDatasetOrder( bool reverse );
    /** \see setReverseDatasetOrder */
    bool reverseDatasetOrder() const;

    void setLineAttributes( const LineAttributes & a );
    void setLineAttributes( int column, const LineAttributes & a );
    void setLineAttributes( const QModelIndex & index, const LineAttributes & a );
    void resetLineAttributes( int column );
    void resetLineAttributes( const QModelIndex & index );
    LineAttributes lineAttributes() const;
    LineAttributes lineAttributes( int column ) const;
    LineAttributes lineAttributes( const QModelIndex & index ) const;

    void setThreeDLineAttributes( const ThreeDLineAttributes & a );
    void setThreeDLineAttributes( int column, const ThreeDLineAttributes & a );
    void setThreeDLineAttributes( const QModelIndex & index,
                                  const ThreeDLineAttributes & a );

    //FIXME(khz): big TODO(khz): add a lot of reset...Attributes() methods to all
    // appropriate places, for 2.1 (that is: after we have release 2.0.2)  :-)

    ThreeDLineAttributes threeDLineAttributes() const;
    ThreeDLineAttributes threeDLineAttributes( int column ) const;
    ThreeDLineAttributes threeDLineAttributes( const QModelIndex & index ) const;

    void setValueTrackerAttributes( const QModelIndex & index,
                                    const ValueTrackerAttributes & a );
    ValueTrackerAttributes valueTrackerAttributes( const QModelIndex & index ) const;

#if QT_VERSION < 0x040400 || defined(Q_COMPILER_MANGLES_RETURN_TYPE)
    // implement AbstractCartesianDiagram
    /* reimpl */
    const int numberOfAbscissaSegments () const;
    /* reimpl */
    const int numberOfOrdinateSegments () const;
#else
    // implement AbstractCartesianDiagram
    /* reimpl */
    int numberOfAbscissaSegments () const;
    /* reimpl */
    int numberOfOrdinateSegments () const;
#endif

protected:
    void paint ( PaintContext* paintContext );

public:
    void resize ( const QSizeF& area );

protected:
    // FIXME what does that mean?
    double valueForCellTesting( int row, int column,
                                bool& bOK,
                                bool showHiddenCellsAsInvalid = false ) const;
    LineAttributes::MissingValuesPolicy getCellValues(
        int row, int column,
        bool shiftCountedXValuesByHalfSection,
        double& valueX, double& valueY ) const;

    virtual double threeDItemDepth( const QModelIndex & index ) const;
    virtual double threeDItemDepth( int column ) const;
    /** \reimpl */
    virtual const QPair<QPointF, QPointF> calculateDataBoundaries() const;
    void paintEvent ( QPaintEvent* );
    void resizeEvent ( QResizeEvent* );
}; // End of class KDChartLineDiagram

}

#endif // KDCHARTLINEDIAGRAM_H
