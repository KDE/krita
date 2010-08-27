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

#ifndef KDCHARTTERNARYPOINTDIAGRAM_H
#define KDCHARTTERNARYPOINTDIAGRAM_H

#include "KDChartTernaryCoordinatePlane.h"
#include "KDChartAbstractTernaryDiagram.h"

namespace KDChart {

    /**
      * @brief A TernaryPointDiagram is a point diagram within a ternary coordinate plane
      */
    class KDCHART_EXPORT TernaryPointDiagram : public AbstractTernaryDiagram
    {
        Q_OBJECT
        Q_DISABLE_COPY( TernaryPointDiagram )
        KDCHART_DECLARE_DERIVED_DIAGRAM( TernaryPointDiagram, TernaryCoordinatePlane )

    public:
        explicit TernaryPointDiagram ( QWidget* parent = 0, TernaryCoordinatePlane* plane = 0 );
        virtual ~TernaryPointDiagram();

        virtual void resize (const QSizeF &area);
        virtual void paint (PaintContext *paintContext);

    protected:
        virtual const QPair< QPointF, QPointF >  calculateDataBoundaries () const;
    };

}

#endif
