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

#ifndef KDCHARTABSTRACTCARTESIANDIAGRAM_P_H
#define KDCHARTABSTRACTCARTESIANDIAGRAM_P_H

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

#include "KDChartAbstractCartesianDiagram.h"

#include <KDChartAbstractDiagram_p.h>
#include <KDChartAbstractThreeDAttributes.h>
#include <KDChartGridAttributes.h>

#include <KDABLibFakes>


namespace KDChart {

  class CartesianCoordinatePlane;
  class AbstractCartesianDiagram;

/**
 * \internal
 */
class AbstractCartesianDiagram::Private : public AbstractDiagram::Private
{
    friend class AbstractCartesianDiagram;
public:
    Private();
   virtual ~Private();

    Private( const Private& rhs ) :
        AbstractDiagram::Private( rhs ),
        // Do not copy axes and reference diagrams.
        axesList(),
        referenceDiagram( 0 )
        {
        }

    /** \reimpl */
    virtual CartesianDiagramDataCompressor::DataValueAttributesList aggregatedAttrs(
            AbstractDiagram * diagram,
            const QModelIndex & index,
            const CartesianDiagramDataCompressor::CachePosition * position ) const
    {
        if( position )
            return compressor.aggregatedAttrs( diagram, index, *position );
        CartesianDiagramDataCompressor::DataValueAttributesList allAttrs;
        allAttrs[index] = diagram->dataValueAttributes( index );
        return allAttrs;
    }

   CartesianAxisList axesList;

   AbstractCartesianDiagram* referenceDiagram;
   QPointF referenceDiagramOffset;

   mutable CartesianDiagramDataCompressor compressor;
};

KDCHART_IMPL_DERIVED_DIAGRAM( AbstractCartesianDiagram, AbstractDiagram, CartesianCoordinatePlane )
/*
inline AbstractCartesianDiagram::AbstractCartesianDiagram( Private * p )
    : AbstractDiagram( p ) { init(); }
inline AbstractCartesianDiagram::AbstractCartesianDiagram(
    Private * p, QWidget* parent, CartesianCoordinatePlane* plane )
    : AbstractDiagram( p, parent, plane ) { init(); }
inline AbstractCartesianDiagram::Private * AbstractCartesianDiagram::d_func()
{ return static_cast<Private*>( AbstractDiagram::d_func() ); }
inline const AbstractCartesianDiagram::Private * AbstractCartesianDiagram::d_func() const
{ return static_cast<const Private*>( AbstractDiagram::d_func() ); }
*/

}
#endif /* KDCHARTABSTRACTCARTESIANDIAGRAM_P_H */

