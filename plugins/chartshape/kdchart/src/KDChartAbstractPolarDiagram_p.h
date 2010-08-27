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

#ifndef KDCHARTABSTRACTPOLARDIAGRAM_P_H
#define KDCHARTABSTRACTPOLARDIAGRAM_P_H

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

#include "KDChartAbstractDiagram_p.h"
#include <KDChartGridAttributes.h>

#include <KDABLibFakes>


namespace KDChart {

  class PolarCoordinatePlane;

/**
 * \internal
 */
class AbstractPolarDiagram::Private : public AbstractDiagram::Private
{
    friend class AbstractPolarDiagram;
public:
    Private();
    ~Private();

    Private( const Private& rhs ) :
        AbstractDiagram::Private( rhs )
        {
            // just for consistency
        }

    /** \reimpl */
    // FIXME: Optimize when needed
    virtual qreal calcPercentValue( const QModelIndex & index )
    {
        qreal sum = 0.0;
        for ( int row = 0; row < attributesModel->rowCount( QModelIndex() ); row++ )
            sum += attributesModel->data( attributesModel->index( row, index.column(), QModelIndex() ) ).toDouble();
        if ( sum == 0.0 )
            return 0.0;
        return attributesModel->data( attributesModel->mapFromSource( index ) ).toDouble() / sum * 100.0;
    }

private:
    double granularity;
};

KDCHART_IMPL_DERIVED_DIAGRAM( AbstractPolarDiagram, AbstractDiagram, PolarCoordinatePlane )
/*
inline AbstractPolarDiagram::AbstractPolarDiagram( Private * p )
    : AbstractDiagram( p ) { init(); }
inline AbstractPolarDiagram::AbstractPolarDiagram(
    Private *p, QWidget* parent, PolarCoordinatePlane* plane )
    : AbstractDiagram( p, parent, plane ) { init(); }
inline AbstractPolarDiagram::Private * AbstractPolarDiagram::d_func()
{ return static_cast<Private*>( AbstractDiagram::d_func() ); }
inline const AbstractPolarDiagram::Private * AbstractPolarDiagram::d_func() const
{ return static_cast<const Private*>( AbstractDiagram::d_func() ); }
*/

}
#endif /* KDCHARTABSTRACTCARTESIANDIAGRAM_P_H */

