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

#include "KDChartMeasure.h"

#include <QWidget>

#include <QtXml/QDomDocumentFragment>
#include <KDChartAbstractArea.h>
#include <KDChartCartesianCoordinatePlane.h>
#include <KDChartTextAttributes.h>
#include <KDChartFrameAttributes.h>
#include <KDChartBackgroundAttributes.h>

#include <KDABLibFakes>


namespace KDChart {


Measure::Measure()
  : mValue( 0.0 ),
    mMode(  KDChartEnums::MeasureCalculationModeAuto ),
    mArea(  0 ),
    mOrientation( KDChartEnums::MeasureOrientationAuto )
{
    // this bloc left empty intentionally
}

Measure::Measure( qreal value,
    KDChartEnums::MeasureCalculationMode mode,
    KDChartEnums::MeasureOrientation orientation )
  : mValue( value ),
    mMode(  mode ),
    mArea(  0 ),
    mOrientation( orientation )
{
    // this bloc left empty intentionally
}

Measure::Measure( const Measure& r )
  : mValue( r.value() ),
    mMode(  r.calculationMode() ),
    mArea(  r.referenceArea() ),
    mOrientation( r.referenceOrientation() )
{
    // this bloc left empty intentionally
}

Measure & Measure::operator=( const Measure& r )
{
    if( this != &r ){
        mValue = r.value();
        mMode  = r.calculationMode();
        mArea  = r.referenceArea();
        mOrientation = r.referenceOrientation();
    }

    return *this;
}


qreal Measure::calculatedValue( const QSizeF& autoSize,
                                KDChartEnums::MeasureOrientation autoOrientation) const
{
    if( mMode == KDChartEnums::MeasureCalculationModeAbsolute ){
        return mValue;
    }else{
        qreal value = 0.0;
        const QObject theAutoArea;
        const QObject* autoArea = &theAutoArea;
        const QObject* area = mArea ? mArea : autoArea;
        KDChartEnums::MeasureOrientation orientation = mOrientation;
        switch( mMode ){
            case KDChartEnums::MeasureCalculationModeAuto:
                area = autoArea;
                orientation = autoOrientation;
                break;
            case KDChartEnums::MeasureCalculationModeAutoArea:
                area = autoArea;
                break;
            case KDChartEnums::MeasureCalculationModeAutoOrientation:
                orientation = autoOrientation;
                break;
            case KDChartEnums::MeasureCalculationModeAbsolute: // fall through intended
            case KDChartEnums::MeasureCalculationModeRelative:
                break;
        }
        if( area ){
            QSizeF size;
            if( area == autoArea )
                size = autoSize;
            else
                size = sizeOfArea( area );
            //qDebug() << ( area == autoArea ) << "size" << size;
            qreal referenceValue;
            switch( orientation ){
                case KDChartEnums::MeasureOrientationAuto: // fall through intended
                case KDChartEnums::MeasureOrientationMinimum:
                    referenceValue = qMin( size.width(), size.height() );
                    break;
                case KDChartEnums::MeasureOrientationMaximum:
                    referenceValue = qMax( size.width(), size.height() );
                    break;
                case KDChartEnums::MeasureOrientationHorizontal:
                    referenceValue = size.width();
                    break;
                case KDChartEnums::MeasureOrientationVertical:
                    referenceValue = size.height();
                    break;
            }
            value = mValue / 1000.0 * referenceValue;
        }
        return value;
    }
}


qreal Measure::calculatedValue( const QObject* autoArea,
                                KDChartEnums::MeasureOrientation autoOrientation) const
{
    return calculatedValue( sizeOfArea( autoArea ), autoOrientation);
}


const QSizeF Measure::sizeOfArea( const QObject* area ) const
{
    QSizeF size;
    const CartesianCoordinatePlane* plane = dynamic_cast<const CartesianCoordinatePlane*>( area );
    if ( false ) {
        size = plane->visibleDiagramArea().size();
    } else {
        const AbstractArea* kdcArea = dynamic_cast<const AbstractArea*>(area);
        if( kdcArea ){
            size = kdcArea->geometry().size();
            //qDebug() << "Measure::sizeOfArea() found kdcArea with size" << size;
        }else{
            const QWidget* widget = dynamic_cast<const QWidget*>(area);
            if( widget ){
                /* ATTENTION: Using the layout does not work: The Legend will never get the right size then!
                const QLayout * layout = widget->layout();
                if( layout ){
                    size = layout->geometry().size();
                    //qDebug() << "Measure::sizeOfArea() found widget with layout size" << size;
                }else*/
                {
                    size = widget->geometry().size();
                    //qDebug() << "Measure::sizeOfArea() found widget with size" << size;
                }
            }else if( mMode != KDChartEnums::MeasureCalculationModeAbsolute ){
                size = QSizeF(1.0, 1.0);
                //qDebug("Measure::sizeOfArea() got no valid area.");
            }
        }
    }
    const QPair< qreal, qreal > factors
            = GlobalMeasureScaling::instance()->currentFactors();
    return QSizeF(size.width() * factors.first, size.height() * factors.second);
}


bool Measure::operator==( const Measure& r ) const
{
    return( mValue == r.value() &&
            mMode  == r.calculationMode() &&
            mArea  == r.referenceArea() &&
            mOrientation == r.referenceOrientation() );
}



GlobalMeasureScaling::GlobalMeasureScaling()
{
    mFactors.push( qMakePair(qreal(1.0), qreal(1.0)) );
}

GlobalMeasureScaling::~GlobalMeasureScaling()
{
    // this space left empty intentionally
}

GlobalMeasureScaling* GlobalMeasureScaling::instance()
{
    static GlobalMeasureScaling instance;
    return &instance;
}

void GlobalMeasureScaling::setFactors(qreal factorX, qreal factorY)
{
    instance()->mFactors.push( qMakePair(factorX, factorY) );
}

void GlobalMeasureScaling::resetFactors()
{
    // never remove the initial (1.0. 1.0) setting
    if( instance()->mFactors.count() > 1 )
        instance()->mFactors.pop();
}

const QPair< qreal, qreal > GlobalMeasureScaling::currentFactors()
{
    return instance()->mFactors.top();
}

void GlobalMeasureScaling::setPaintDevice( QPaintDevice* paintDevice )
{
    instance()->m_paintDevice = paintDevice;
}

QPaintDevice* GlobalMeasureScaling::paintDevice()
{
    return instance()->m_paintDevice;
}

}

#if !defined(QT_NO_DEBUG_STREAM)
QDebug operator<<(QDebug dbg, const KDChart::Measure& m)
{
    dbg << "KDChart::Measure("
	<< "value="<<m.value()
	<< "calculationmode="<<m.calculationMode()
	<< "referencearea="<<m.referenceArea()
	<< "referenceorientation="<<m.referenceOrientation()
	<< ")";
    return dbg;
}
#endif /* QT_NO_DEBUG_STREAM */
