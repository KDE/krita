/****************************************************************************
 ** Copyright (C) 2007 Klarälvdalens Datakonsult AB.  All rights reserved.
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

#ifndef CARTESIANCOORDINATETRANSFORMATION_H
#define CARTESIANCOORDINATETRANSFORMATION_H

#include <QList>
#include <QRectF>
#include <QPointF>

#include "KDChartZoomParameters.h"

#include <cmath>
#include <limits>

namespace KDChart {

    // FIXME: if this struct is used more often, we need to make it a class
    // with proper accessor methods:

    /**
      * \internal
      */
    struct CoordinateTransformation {
        QRectF diagramRect;
        // represents the distance of the diagram coordinate origin to the
        // origin of the coordinate plane space:
        QPointF originTranslation;
        // make a vector base for R2:
        double unitVectorX;
        double unitVectorY;
        // implement isometric scaling:
        double isoScaleX;
        double isoScaleY;

        CartesianCoordinatePlane::AxesCalcMode axesCalcModeY;
        CartesianCoordinatePlane::AxesCalcMode axesCalcModeX;

        ZoomParameters zoom;

        typedef QPair< qreal, qreal > qrealPair;

        inline qreal makeLogarithmic( qrealPair reference, qreal value ) const
        {
            qreal result = value;

            qreal relation;
            if( reference.second == -1.0 )
                relation = 1.0;
            else if( reference.second == 1.0 )
                relation = 1.0;
            else if( reference.second > 0.0 )
                relation = reference.second / log10( reference.second );
            else if( result < 0.0 )
                relation = reference.second / log10( -reference.second );
            else
                relation = 10.0;

            if( value == 0.0 )
                result = 0.0;//std::numeric_limits< qreal >::quiet_NaN();
            else if( value > 0.0 )
                result = log10( result ) * relation;
            else if( value < 0.0 )
                result = -log10( -result ) * relation;

            if( value == 0.0 )
                return result;

            result -= log10( qAbs( reference.first ) ) * relation;
            result *= ( reference.second - reference.first ) / relation / (log10(qAbs(reference.second))-log10(qAbs(reference.first)));
            result += reference.first;
           
            if( reference.first < 0.0 )
            {
                result += reference.first;
                result -= reference.second;
                result = reference.first - result + reference.second;

            }
           
            return result;
        }

        inline QPointF translate( const QPointF& diagramPoint ) const
        {
            // ### de-inline me
            QPointF result = originTranslation;
            QPointF tempPoint = diagramPoint;

            const QRectF& diagRect = diagramRect;
            
            if( axesCalcModeY == CartesianCoordinatePlane::Logarithmic )
            {
                tempPoint.setY( makeLogarithmic( qrealPair( diagRect.bottom(), diagRect.y() ), tempPoint.y() ) );
            }
            if( axesCalcModeX == CartesianCoordinatePlane::Logarithmic )
            {
                tempPoint.setX( makeLogarithmic( qrealPair( diagRect.x(), diagRect.right() ), tempPoint.x() ) );
            }

            tempPoint.rx() += diagRect.width() / (2.0 * zoom.xFactor);
            tempPoint.ry() += diagRect.height() / (2.0 * zoom.yFactor);

            tempPoint.rx() -= diagRect.width() * zoom.xCenter;
            tempPoint.ry() -= diagRect.height() * zoom.yCenter;

            // translate:      xNew = (xOld - diaX) * zoomX + diaX
            tempPoint.setX( ( tempPoint.x() - diagRect.x() ) * zoom.xFactor + diagRect.x() );
            tempPoint.setY( ( tempPoint.y() - diagRect.y() ) * zoom.yFactor + diagRect.y() );

            result.rx() += isoScaleX * unitVectorX * tempPoint.x();
            result.ry() += isoScaleY * unitVectorY * tempPoint.y();

            return result;
        }

        // convert screen points to value space points
        inline const QPointF translateBack( const QPointF& screenPoint ) const
        {
            qreal x, y;

            x = screenPoint.x() - originTranslation.x();
            y = screenPoint.y() - originTranslation.y();

            x /= isoScaleX * unitVectorX;
            y /= isoScaleY * unitVectorY;

            // translate back: xOld = DiaX + (xNew - DiaX) / zoomX
            x = diagramRect.x() + (x - diagramRect.x()) / zoom.xFactor;
            y = diagramRect.y() + (y - diagramRect.y()) / zoom.yFactor;

            x += diagramRect.width()  * zoom.xCenter;
            y += diagramRect.height() * zoom.yCenter;

            x -= diagramRect.width()  / (2.0 * zoom.xFactor);
            y -= diagramRect.height() / (2.0 * zoom.yFactor);

            /*
            if ( axesCalcModeY == CartesianCoordinatePlane::Logarithmic ){
                tempPoint.setY( makeLogarithmic( diagramRect.y(), tempPoint.y() ) );
                //qDebug() << "Y: " << tempPoint.y();
            }
            if ( axesCalcModeX == CartesianCoordinatePlane::Logarithmic ){
                //qDebug() << "X diagramRect.x(): " << diagramRect.x();
                //qDebug() << "X tempPoint old: " << tempPoint;
                tempPoint.setX( makeLogarithmic( diagramRect.width(), tempPoint.x() ) );
                //qDebug() << "X tempPoint new: " << tempPoint;
            }
//            qDebug() << "CoordinateTransformation::translate() using diagramRect: "
//                     << diagramRect.x() << diagramRect.y() << diagramRect.width() << diagramRect.height();
            */

            return QPointF(x, y);
        }

    };

    typedef QList<CoordinateTransformation> CoordinateTransformationList;

}

#endif

