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
#ifndef KDCHARTLEVEYJENNINGSAXIS_H
#define KDCHARTLEVEYJENNINGSAXIS_H

#include <QList>

#include "../KDChartCartesianAxis.h"

#include "KDChartLeveyJenningsGridAttributes.h"

namespace KDChart {

    class LeveyJenningsDiagram;

    /**
      * The class for levey jennings axes.
      *
      * For being useful, axes need to be assigned to a diagram, see
      * LeveyJenningsDiagram::addAxis and LeveyJenningsDiagram::takeAxis.
      *
      * \sa PolarAxis, AbstractCartesianDiagram
      */
    class KDCHART_EXPORT LeveyJenningsAxis : public CartesianAxis
    {
        Q_OBJECT

        Q_DISABLE_COPY( LeveyJenningsAxis )
        KDCHART_DECLARE_PRIVATE_DERIVED_PARENT( LeveyJenningsAxis, AbstractDiagram* )

    public:
        /**
          * C'tor of the class for levey jennings axes.
          *
          * \note If using a zero parent for the constructor, you need to call
          * your diagram's addAxis function to add your axis to the diagram.
          * Otherwise, there is no need to call addAxis, since the constructor
          * does that automatically for you, if you pass a diagram as parameter.
          *
          * \sa AbstractCartesianDiagram::addAxis
          */
        explicit LeveyJenningsAxis ( LeveyJenningsDiagram* diagram = 0 );
        ~LeveyJenningsAxis();

        LeveyJenningsGridAttributes::GridType type() const;
        void setType( LeveyJenningsGridAttributes::GridType type );

        Qt::DateFormat dateFormat() const;
        void setDateFormat( Qt::DateFormat format );

        /**
         * Returns true if both axes have the same settings.
         */
        bool compare( const LeveyJenningsAxis* other ) const;

        /** reimpl */
        void paintCtx( PaintContext* );

    protected:
        virtual void paintAsOrdinate( PaintContext* );

        virtual void paintAsAbscissa( PaintContext* );
    };

    typedef QList<LeveyJenningsAxis*> LeveyJenningsAxisList;
}

#endif
