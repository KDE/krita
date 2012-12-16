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

#ifndef KDCHARTABSTRACTTERNARYDIAGRAM_H
#define KDCHARTABSTRACTTERNARYDIAGRAM_H

#include "../KDChartAbstractDiagram.h"
#include "KDChartTernaryAxis.h"

namespace KDChart {

    class TernaryCoordinatePlane;
    class TernaryAxis;

    /**
      * @brief Base class for diagrams based on a ternary coordinate plane.
      */
    class KDCHART_EXPORT AbstractTernaryDiagram : public AbstractDiagram
    {
        Q_OBJECT
        Q_DISABLE_COPY( AbstractTernaryDiagram )
        KDCHART_DECLARE_DERIVED_DIAGRAM( AbstractTernaryDiagram,
                                         TernaryCoordinatePlane )

    public:
        explicit AbstractTernaryDiagram ( QWidget* parent = 0,
                                          TernaryCoordinatePlane* plane = 0 );
        virtual ~AbstractTernaryDiagram();

        virtual void resize (const QSizeF &area) = 0;
        virtual void paint (PaintContext *paintContext);

        virtual void addAxis( TernaryAxis* axis );
        virtual void takeAxis( TernaryAxis* axis );
        virtual TernaryAxisList axes () const;

    protected:
        virtual const QPair< QPointF, QPointF >  calculateDataBoundaries () const = 0;

    };

}

#endif
