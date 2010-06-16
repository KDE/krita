#ifndef KDCHARTABSTRACTPROXYMODEL_H
#define KDCHARTABSTRACTPROXYMODEL_H

/* -*- Mode: C++ -*-
   KDChart - a multi-platform charting engine
   */

/****************************************************************************
 ** Copyright (C) 2007 Klaraelvdalens Datakonsult AB.  All rights reserved.
 **
 ** This file is part of the KDChart library.
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

#include <QAbstractProxyModel>

#include "KDChartGlobal.h"

namespace KDChart
{
    /**
      * @brief Base class for all proxy models used inside KD Chart
      * \internal
      */
    class KDCHART_EXPORT AbstractProxyModel : public QAbstractProxyModel
    {
        Q_OBJECT
    public:
        explicit AbstractProxyModel( QObject* parent = 0 );

        /*! \reimpl */ 
        QModelIndex mapFromSource( const QModelIndex & sourceIndex ) const;
        /*! \reimpl */ 
        QModelIndex mapToSource( const QModelIndex &proxyIndex ) const;

        /*! \reimpl */
        QModelIndex index( int row, int col, const QModelIndex& index ) const;
        /*! \reimpl */ 
        QModelIndex parent( const QModelIndex& index ) const;
    };
}

#endif /* KDCHARTABSTRACTPROXYMODEL_H */
